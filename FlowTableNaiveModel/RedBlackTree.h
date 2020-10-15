#pragma once

#include "BBSTree.h"
#include "SCBF.h"

static const bool RED = false;
static const bool BLACK = true;

class RBNode :public Node {
public:
	bool color;
public:
	RBNode(Flow* flow, Node* parent) : Node(flow, parent) {
		color = RED;
	}
};

class RedBlackTree : public BBSTree {
public:
	RedBlackTree(Comparator* comparator, Equal* equal,SCBF * scbf) :BBSTree(comparator, equal),scbf(scbf) {}

	ULONG timeoutScan(Time tCur) {
		if (root != NULL) {
			list<Node*> stack1;
			list<Node*> stack2;
			stack1.push_back(root);
			while (stack1.size() != 0) {
				Node* p = stack1.back();
				stack1.pop_back();
				for (list<Flow*>::iterator itor = p->flowList.begin(); itor != p->flowList.end();) {
					Time t = tCur - (*itor)->tLast;
					if (t.sec > FLOW_TIMEOUT) {
						Flow* tmp = (*itor);
						itor = p->flowList.erase(itor);
						scbf->filter_delete(*(SCFlow*)tmp);
						delete tmp;
						if (p->flowList.size() == 0) {
							stack2.push_back(p);
							break;
						}
					}
					else {
						itor++;
					}
				}
				if (p->left != NULL) {
					stack1.push_back(p->left);
				}
				if (p->right != NULL) {
					stack1.push_back(p->right);
				}
			}
			while (stack2.size() != 0) {
				Node* s = stack2.back();
				stack2.pop_back();
				removeNode(s,&stack2);
			}
		}
		return 0;
	}

private:
	SCBF * scbf;

private:

	void removeNode(Node * node,list<Node*> * stack) {
		if (node == NULL) {
			return;
		}
		if (node->hasTwoChildren()) {
			Node* s = successor(node);
			for (typename list<Flow*>::iterator itor = s->flowList.begin(); itor != s->flowList.end(); itor++) {
				node->flowList.push_back(*itor);
			}
			for (typename list<Node*>::iterator itor = stack->begin(); itor != stack->end(); ) {
				if (*itor == s) {
					itor = stack->erase(itor);
				}
				else {
					itor++;
				}
			}
			stack->push_back(node);
			node = s;
		}

		Node* replacement = node->left != NULL ? node->left : node->right;
		if (replacement != NULL) {
			replacement->parent = node->parent;
			if (node->isLeftChild()) {
				node->parent->left = replacement;
			}
			else if (node->isRightChild()) {
				node->parent->right = replacement;
			}
			else if (node == this->root) {
				this->root = replacement;
				this->root->parent = NULL;
			}
		}
		else {
			if (node->parent == NULL) {
				root = NULL;
			}
			else {
				if (node->isLeftChild()) {
					node->parent->left = NULL;
				}
				else {
					node->parent->right = NULL;
				}
			}
		}
		afterRemove(node, replacement);
		BSTree::afterRemove(node, replacement);
	}

	Node * color(Node* node, bool color) {
		if (node == NULL) {
			return node;
		}
		((RBNode*)node)->color = color;
		return node;
	}
	Node * red(Node * node) {
		return color(node, RED);
	}

	Node* black(Node * node) {
		return color(node, BLACK);
	}

	bool colorOf(Node * node) {
		return node == NULL ? BLACK : ((RBNode*)node)->color;	//空节点默认为black
	}

	bool isBlack(Node* node) {
		return colorOf(node) == BLACK;
	}

	bool isRed(Node* node) {
		return colorOf(node) == RED;
	}

protected:
	virtual Node * createNode(Flow * flow, Node* parent) {
		return new RBNode(flow, parent);
	}

	virtual void afterAdd(Node* node) {
		Node* parent = node->parent;
		if (parent == NULL) {
			black(node);
			return;
		}
		if (isBlack(parent)) {
			return;
		}

		Node* uncle = parent->sibling();
		Node* grand = parent->parent;

		//叔父结点是红色，B树上溢
		if (isRed(uncle)) {
			black(uncle);
			black(parent);
			red(grand);
			afterAdd(grand);
			return;
		}

		//叔父结点为黑色，（可以理解为当前节点的sibling == null）
		if (parent->isLeftChild()) {
			if (node->isLeftChild()) {
				black(parent);
				red(grand);
				this->rotateRight(grand);
			}
			else {
				black(node);
				red(parent);
				red(grand);
				this->rotateLeft(parent);
				this->rotateRight(grand);
			}
		}
		else {
			if (node->isLeftChild()) {
				black(node);
				red(parent);
				red(grand);
				this->rotateRight(parent);
				this->rotateLeft(grand);
			}
			else {
				black(parent);
				red(grand);
				this->rotateLeft(grand);
			}
		}
	}
	virtual void afterRemove(Node* node, Node* replacement) {
		if (isRed(node)) {
			return;
		}
		if (isRed(replacement)) {
			black(replacement);
			return;
		}
		Node * parent = node->parent;
		if (parent == NULL) {
			return;
		}

		//来到这，删除的一定是黑色叶子结点，parent.left == NULL的原因是被删后，parent的left属性==NULL
		//或者由于递归操作，B树下溢，node是isLeftChild
		bool left = parent->left == NULL || node->isLeftChild();
		//parent.left属性被清空，无法直接使用sbling()获取兄弟节点
		Node * sibling = left ? parent->right : parent->left;
		if (left) {
			if (isRed(sibling)) {
				black(sibling);
				red(parent);
				this->rotateLeft(parent);
				sibling = parent->right;
			}
			if (isBlack(sibling->left) && isBlack(sibling->right)) {
				bool parentBlack = isBlack(parent);
				red(sibling);
				black(parent);
				if (parentBlack) {
					afterRemove(parent, NULL);
				}
			}
			else {
				if (isBlack(sibling->right)) {
					this->rotateRight(sibling);
					sibling = parent->right;
				}
				color(sibling, colorOf(parent));
				black(parent);
				black(sibling->right);
				this->rotateLeft(parent);
			}
		}
		else {
			if (isRed(sibling)) {
				red(parent);
				black(sibling);
				this->rotateRight(parent);
				sibling = parent->left;
			}
			if (isBlack(sibling->left) && isBlack(sibling->right)) {
				bool parentBlack = isBlack(parent);
				black(parent);
				red(sibling);
				if (parentBlack) {
					afterRemove(parent, NULL);
				}
			}
			else {
				if (isBlack(sibling->left)) {
					this->rotateLeft(sibling);
					sibling = parent->left;
				}
				color(sibling, colorOf(parent));
				black(parent);
				black(sibling->left);
				this->rotateRight(parent);
			}
		}
	}
};