#pragma once

#include "BinaryTree.h"
#include "Comparator.h"
#include "Equal.h"
#include "SCBF.h"

class BSTree :public BinaryTree{
private:
	Comparator * comparator;
	Equal * equal;
public:
	BSTree(Comparator* comparator, Equal* equal) :BinaryTree(), comparator(comparator), equal(equal) {}

	BSTree() :BSTree(NULL, NULL) {}

	virtual void add(Flow * flow) {
		if (flow == NULL) {
			return;
		}
		if (root == NULL){
			root = createNode(flow, NULL);
			size++;
			afterAdd(root);
			return;
		}
		Node * parent = root;
		Node * node = root;
		int cmp = 0;
		while (node != NULL) {
			parent = node;
			cmp = comparator->compare(flow, node->flowList.front());
			if (cmp < 0) {
				node = node->left;
			}
			else if (cmp > 0) {
				node = node->right;
			}
			else {
				for (typename list<Flow*>::iterator itor = node->flowList.begin(); itor != node->flowList.end(); itor++) {
					if (equal->equals(*itor,flow)) {
						return;
					}
				}
				node->flowList.push_back(flow);
				size++;
				return;
			}
		}
		Node* newNode = createNode(flow, parent);
		if (cmp > 0) {
			parent->right = newNode;
		}
		else {
			parent->left = newNode;
		}
		size++;
		afterAdd(newNode);
	}

	virtual void remove(Flow* flow) {
		if (flow == NULL) {
			return;
		}
		Node* node = root;
		while (node != NULL) {
			int cmp = compare(flow, node->flowList.front());
			if (cmp == 0) {
				for (typename list<Flow*>::iterator itor = node->flowList.begin(); itor != node->flowList.end();) {
					if (equal->equals(*itor, flow)) {
						itor = node->flowList.erase(itor);
						this->size--;
						if (node->flowList.size() == 0) {
							removeNode(node);
						}
						return;
					}
					else {
						itor++;
					}
				}
			}
			else if (cmp > 0) {
				node = node->right;
			}
			else {
				node = node->left;
			}
		}
	}

	virtual bool contains(Flow* flow) {
		Node * p = node(flow);
		for (typename list<Flow*>::iterator itor = p->flowList.begin(); itor != p->flowList.end(); itor++) {
			if (equal->equals(*itor, flow)) {
				return true;
			}
		}
		return false;
	}

	virtual ULONG find(Flow * flow, PFlow & result) {
		ULONG len = 0;
		if (root == NULL) {
			return 0;
		}
		Node* node = root;
		while (node != NULL) {
			int cmp = comparator->compare(flow, node->flowList.front());
			if (cmp == 0) {
				for (typename list<Flow*>::iterator itor = node->flowList.begin(); itor != node->flowList.end(); itor++) {
					len++;
					if (equal->equals(*itor, flow)) {
						result = *itor;
						return len;
					}
				}
				return len;
			}
			else if (cmp > 0) {
				node = node->right;
				len++;
			}
			else {
				node = node->left;
				len++;
			}
		}
		return len;
	}

	virtual ULONG timeoutScan(Time tCur,SCBF * scbf) {
		list<Flow*> flowList;
		timeoutScan(root, flowList, tCur, scbf);
		for (list<Flow*>::iterator itor = flowList.begin(); itor != flowList.end(); itor++ ) {
			remove(*itor);
		}
		return 0;
	}

private:
	void timeoutScan(Node* node, list<Flow*> & timeoutFlowList,Time tCur,SCBF * scbf) {
		if (node != NULL) {
			for (list<Flow*>::iterator itor = node->flowList.begin(); itor != node->flowList.end(); itor++ ) {
				Time t = tCur - (*itor)->tLast;
				if (t.sec > FLOW_TIMEOUT) {
					Flow* tmp = (*itor);
					timeoutFlowList.push_back(tmp);
				}
			}
			timeoutScan(node->left, timeoutFlowList, tCur, scbf);
			timeoutScan(node->right, timeoutFlowList, tCur, scbf);
		}
	}

protected:
	virtual Node* predessor(Node* node) {
		if (node == NULL) {
			return NULL;
		}
		Node* p = node->left;
		if (p != NULL) {
			while (p->right != NULL) {
				p = p->right;
			}
			return p;
		}
		while (node->parent != NULL && node->parent->left == node) {
			node = node->parent;
		}
		return node->parent;
	}

	virtual Node* successor(Node* node) {
		if (node == NULL) {
			return NULL;
		}
		Node* p = node->right;
		if (p != NULL) {
			while (p->left != NULL) {
				p = p->left;
			}
			return p;
		}
		while (node->parent != NULL && node == node->parent->right) {
			node = node->parent;
		}
		return node->parent;
	}
	
	virtual void afterAdd(Node* node) {}
	virtual void afterRemove(Node* node, Node* replacement) {
		if (node != NULL) {
			delete node;
			node = NULL;
		}
	}

	virtual void removeNode(Node* node) {
		if (node == NULL) {
			return;
		}
		if (node->hasTwoChildren()) {
			Node* s = successor(node);
			for (typename list<Flow*>::iterator itor = s->flowList.begin(); itor != s->flowList.end(); itor++) {
				node->flowList.push_back(*itor);
			}
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

private:
	int compare(Flow* flow1, Flow* flow2) {
		return comparator->compare(flow1, flow2);
	}

	Node* node(Flow* flow) { 
		Node * node = root;
		while (node != NULL) {
			int cmp = comparator->compare(flow, node->flowList.front());
			if (cmp == 0) {
				return node;
			}
			else if (cmp > 0) {
				node = node->right;
			}
			else {
				node = node->left;
			}
		}
		return NULL;
	}
};