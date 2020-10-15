#pragma once

#include "uniform.h"

class Comparator {
public:
	virtual int compare(Flow* flow1, Flow* flow2) {
		if (flow1->key.src.ip > flow2->key.src.ip) {
			return 1;
		}
		else if (flow1->key.src.ip < flow2->key.src.ip) {
			return -1;
		}
		else {
			return 0;
		}
	}
};