#pragma once

#include "uniform.h"

class Equal {
public:
	virtual bool equals(Flow* flow1, Flow* flow2) {
		return flow1->mask == flow2->mask && flow1->key == flow2->key;
	}
};