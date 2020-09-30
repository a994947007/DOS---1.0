#pragma once

#include "uniform.h"
#define H3_HASH_NUM 6

/*SCBF计数器索引结点结构*/
typedef struct SCNode {
	ULONG h3_row;			//对应H3矩阵的行数
	ULONG limit_capacity;	//最佳容量
	ULONG current_capacity;	//当前容量
	UCHAR * m_pCounters;	//计数器数组首地址
	struct SCNode * next;
	SCNode(ULONG h3_row, ULONG limit_capacity) :h3_row(h3_row), limit_capacity(limit_capacity), current_capacity(0), next(NULL) {
		m_pCounters = new UCHAR[(ULONG)pow(2, h3_row)];
		memset(m_pCounters, 0, ((ULONG)pow(2, h3_row)) * sizeof(UCHAR));
	}
	~SCNode() {
		if (m_pCounters) delete m_pCounters;
	}
}SCNode, *PSCNode;

typedef struct SCFlow :public Flow {
	PSCNode pSCNode;
	SCFlow() :pSCNode(NULL) {}
}SCFlow;


class SCBF
{
protected:
	typedef struct H3_Matrix {
		ULONG col[32];
	}H3_Matrix;

protected:
	H3_Matrix h3_matrix[H3_HASH_NUM];
	SCNode * first_node;

public:
	SCBF(ULONG h3_row, ULONG limit_capacity);	//第一个CBF的最佳容量和其哈希矩阵的行数
	virtual bool init_h3_matrix();					//初始化哈希函数矩阵
	virtual bool filter_insert(SCFlow &);		//插入一条流到过滤器
	virtual bool filter_query(const FlowID &,ULONG & lookup_len);		//查找集合中是否存在可匹配的流ID
	virtual bool filter_delete(const SCFlow &);		//在SCBF中删除对应的流ID
	virtual ULONG h3_function(const ULONG h3_row, const ULONG h3_hash, const int matrix_index);//h3哈希函数
	virtual SCNode * lookup_spare_scNode();			//查找流表项数目小于最佳容量的结点
	virtual SCNode* newSCNode(ULONG h3_row, ULONG limit_capacity);
	virtual ~SCBF();
};

