#pragma once

#include "uniform.h"
#include <list>

using namespace std;

// 二叉树节点
class Node {
public:
	list<Flow *> flowList;
	Node * left;
	Node * right;
	Node * parent;
	Node(Flow* flow, Node* parent) :parent(parent), left(NULL), right(NULL) {
		flowList.push_back(flow);
	}

	bool isLeaf() {
		return left == NULL && right == NULL;
	}

	bool hasTwoChildren() {
		if (left != NULL && right != NULL) {
			return true;
		}
		return false;
	}

	bool isLeftChild() {
		return parent != NULL && this == parent->left;
	}

	bool isRightChild() {
		return parent != NULL && this == parent->right;
	}

	Node * sibling() {
		if (parent == NULL) return NULL;
		if (isLeftChild()) {
			return parent->right;
		}
		else {
			return parent->left;
		}
	}
};

// 二叉树
class BinaryTree {
protected:
	int size;
	Node * root;
public:
	BinaryTree():root(NULL),size(0) {}
	virtual int getSize() {
		return size;
	}
	virtual bool isEmpty() {
		return root == NULL;
	}
	virtual void clear() {
		deleteTree(root);
		size = 0;
		root = NULL;
	}
	
	virtual void deleteTree(Node* node) {
		if (node != NULL) {
			deleteTree(node->left);
			deleteTree(node->right);
			delete node;
		}
	}
	virtual ~BinaryTree() {
		clear();
	}

protected:
	virtual Node* createNode(Flow * flow,Node * parent) {
		return new Node(flow, parent);
	}
};