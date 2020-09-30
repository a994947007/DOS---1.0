#pragma once

#include "uniform.h"
#include "Hashlist.h"
#include <vector>

using namespace std;

class HashTable
{
public:
	HashTable(UINT);
	HashTable();
	~HashTable();

	ULONG Find(const FlowID &, PFlow &);
	bool Insert(Flow *);
	void Free();

	ULONG GetTotalNum();
	virtual ULONG TimeoutScan(Time);

	void SetMask(const FlowID &);

protected:
	USHORT CalcHash(const FlowID &);

protected:
	UINT HASH_LEN;//哈希表长度
	vector<HashList *> m_pHashTable;	//哈希链

public:
	FlowID mask;		//当前元组对应的掩码
	ULONG ulPktNum;		//当前元组访问次数
	Time tLastPkt;		//最后一次访问掩码的时间
};

