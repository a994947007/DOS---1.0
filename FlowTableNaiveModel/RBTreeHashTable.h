#pragma once

#include "uniform.h"
#include "RedBlackTree.h"
#include "Equal.h"
#include "Comparator.h"
#include "SCBF.h"
#include <vector>

class RBTreeHashTable
{
private:
	ULONG size;
	Equal flowEqual;
	Comparator flowComparator;
	SCBF* scbf;

public:
	RBTreeHashTable(UINT, SCBF*);
	RBTreeHashTable();
	~RBTreeHashTable();

	ULONG find(const FlowID&, PFlow&);
	bool insert(Flow*);
	void free();

	ULONG getFlowNum();
	ULONG timeoutScan(Time);

	void setMask(const FlowID&);

protected:
	USHORT calcHash(const FlowID&);

protected:
	UINT HASH_LEN;
	vector<RedBlackTree*> m_pHashTable;

public:
	FlowID mask;		//��ǰԪ���Ӧ������
	ULONG ulPktNum;		//��ǰԪ����ʴ���
	Time tLastPkt;		//���һ�η��������ʱ��
};

