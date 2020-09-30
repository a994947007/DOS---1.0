#include "stdafx.h"
#include "SCBF.h"
#include "HashFunctions.h"
#include "Log.h"
#include <time.h>


SCBF::SCBF(ULONG h3_row,ULONG limit_capacity)
{
	this->first_node = new SCNode(h3_row, limit_capacity);
	this->init_h3_matrix();
	srand((unsigned)time(NULL));
}


SCBF::~SCBF()
{
	SCNode * p = this->first_node;
	SCNode * s = NULL;
	while (p != NULL) {
		s = p;
		p = p->next;
		delete s;
	}
}

bool SCBF::init_h3_matrix() {
	for (int i = 0; i<H3_HASH_NUM; i++) {
		for (int j = 0; j<32; j++) {
			ULONG r = 0;
			for (int q = 0; q<4; q++) {
				ULONG randf = rand() % (0xff + 1);
				r += randf;
				if (q == 3) {
					break;
				}
				r <<= 8;
			}
			this->h3_matrix[i].col[j] = r;
		}
	}
	return true;
}

bool SCBF::filter_insert(SCFlow & flow) {
	FlowID fid = flow.key;
	UCHAR buf[FID_LEN];
	((FlowID *)&fid)->ToData(buf);
	ULONG h3_hash = BOB(buf, FID_LEN);
	SCNode * p = lookup_spare_scNode();
	for (int i = 0; i<H3_HASH_NUM; i++) {
		ULONG index = h3_function(p->h3_row, h3_hash, i);
		if (p->m_pCounters[index] == 0xff) {
			LOG("A SCBF counter goes beyond upper limit!");
			return false;
		}
		++(p->m_pCounters[index]);
	}
	++(p->current_capacity);
	flow.pSCNode = p;
	return true;
}

bool SCBF::filter_query(const FlowID & fid,ULONG & lookup_len) {
	UCHAR buf[FID_LEN];
	((FlowID *)&fid)->ToData(buf);
	ULONG h3_hash = BOB(buf, FID_LEN);
	SCNode * p = first_node;
	while (p) {
		int i = 0;
		for (; i<H3_HASH_NUM; i++) {
			ULONG index = h3_function(p->h3_row, h3_hash, i);
			if (p->m_pCounters[index] == 0) {
				break;
			}
		}
		lookup_len++;
		if (i == H3_HASH_NUM) {
			return true;
		}
		p = p->next;
	}
	return false;
}

bool SCBF::filter_delete(const SCFlow & flow) {
	UCHAR buf[FID_LEN];
	FlowID fid = flow.key;
	((FlowID *)&fid)->ToData(buf);
	ULONG h3_hash = BOB(buf, FID_LEN);
	SCNode * scNode = flow.pSCNode;
	for (int i = 0; i<H3_HASH_NUM; i++) {
		ULONG index = h3_function(scNode->h3_row, h3_hash, i);
		--(scNode->m_pCounters[index]);
	}
	--(scNode->current_capacity);
	return true;
}

ULONG SCBF::h3_function(const ULONG h3_row, const ULONG h3_hash, const int matrix_index) {
	ULONG matrix_col = 0xffffffff;
	ULONG col_index = 0x00000001;
	for (int i = 0; i<32; i++, col_index <<= 1) {
		if ((h3_hash & col_index) == col_index) {
			matrix_col ^= h3_matrix[matrix_index].col[i];
		}
	}
	matrix_col >>= (32 - h3_row);
	return matrix_col;
}

SCNode * SCBF::lookup_spare_scNode() {
	SCNode * p = this->first_node;
	while (p) {
		if (p->current_capacity < p->limit_capacity) {
			return p;
		}
		if (p->next == NULL) {
			break;
		}
		p = p->next;
	}
	if (p != NULL) {
		SCNode * s = newSCNode(first_node->h3_row + 1, first_node->limit_capacity * 2); //如果都满了就新增加一个
		s->next = first_node;
		first_node = s;
		return s;
	}
	return NULL;
}

SCNode* SCBF::newSCNode(ULONG h3_row, ULONG limit_capacity) {
	return new SCNode(h3_row + 1, limit_capacity * 2);	//如果都满了就新增加一个
}