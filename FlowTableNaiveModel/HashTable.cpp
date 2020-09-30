#include "stdafx.h"
#include "HashTable.h"
#include "HashFunctions.h"

HashTable::HashTable() {
	this->tLastPkt = Time(0);
	this->ulPktNum = 0;
}

HashTable::HashTable(UINT uiLen):HASH_LEN(uiLen), ulPktNum(0), tLastPkt(0, 0)
{
	for (ULONG i = 0; i<HASH_LEN; i++)
	{
		HashList *p = new HashList;
		if (NULL == p)
		{
			break;
		}
		m_pHashTable.push_back(p);
	}
}


HashTable::~HashTable()
{
	Free();
	for (ULONG i = 0; i<HASH_LEN; i++)
	{
		delete m_pHashTable[i];
		m_pHashTable[i] = NULL;
	}
}

void HashTable::SetMask(const FlowID & mask) {
	this->mask.proto = mask.proto;
	this->mask.src.ip = mask.src.ip;
	this->mask.src.port = mask.src.port;
	this->mask.dst.ip = mask.dst.ip;
	this->mask.dst.port = mask.dst.port;
}

ULONG HashTable::Find(const FlowID & fid, PFlow & pNode) {
	FlowID key = (FlowID)fid&mask;
	USHORT usPos = CalcHash(key);
	return m_pHashTable[usPos]->Find(key, mask, pNode);
}

bool HashTable::Insert(Flow* pNode)
{
	USHORT usPos = CalcHash(pNode->key);
	return m_pHashTable[usPos]->Insert(pNode);
}

void HashTable::Free()
{
	for (UINT i = 0; i<HASH_LEN; i++)
	{
		m_pHashTable[i]->Free();
	}
}

ULONG HashTable::GetTotalNum()
{
	ULONG ulTotal = 0;
	for (ULONG i = 0; i<HASH_LEN; i++)
	{
		ulTotal += m_pHashTable[i]->GetListLen();
	}

	return ulTotal;
}

ULONG HashTable::TimeoutScan(Time tCur)
{
	ULONG ulTotal = 0;
	for (ULONG i = 0; i<HASH_LEN; i++)
	{
		ulTotal += m_pHashTable[i]->TimeoutScan(tCur);
	}

	return ulTotal;
}

USHORT HashTable::CalcHash(const FlowID &key)
{

	UCHAR buf[FID_LEN];
	((FlowID*)&key)->ToData(buf);
	return XOR16(buf, FID_LEN) % HASH_LEN;
}

