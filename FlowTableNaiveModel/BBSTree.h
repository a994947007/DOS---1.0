#pragma once

#include "BSTree.h"

class BBSTree : public BSTree {
public:
	BBSTree(Comparator* comparator, Equal* equal) :BSTree(comparator, equal) {}

	BBSTree() :BBSTree(NULL, NULL) {}

protected:
	virtual void rotateLeft(Node* grand) {
		Node * parent = grand->right;
		Node * child = parent->left;
		grand->right = child;
		parent->left = grand;
		parent->parent = grand->parent;
		if(grand->isLeftChild()){
			grand->parent->left = parent;
		}
		else if (grand->isRightChild()) {
			grand->parent->right = parent;
		}
		else {
			root = parent;
		}
		if (child != NULL) {
			child->parent = grand;
		}
		grand->parent = parent;
	}

	virtual void rotateRight(Node* grand) {
		Node * parent = grand->left;
		Node * child = parent->right;
		grand->left = child;
		parent->right = grand;
		parent->parent = grand->parent;
		if (grand->isLeftChild()) {
			grand->parent->left = parent;
		}
		else if (grand->isRightChild()) {
			grand->parent->right = parent;
		}
		else {
			this->root = parent;
		}
		if (child != NULL) {
			child->parent = grand;
		}
		grand->parent = parent;
	}
};