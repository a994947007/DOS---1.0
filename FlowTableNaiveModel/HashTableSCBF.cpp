#include "stdafx.h"
#include "HashTableSCBF.h"

HashTableSCBF::HashTableSCBF(UINT uiLen)
{
	this->HASH_LEN = uiLen;
	for (ULONG i = 0; i<HASH_LEN; i++)
	{
		HashListSCBF *p = new HashListSCBF;
		if (NULL == p)
		{
			break;
		}
		m_pHashTable.push_back(p);
	}
}

HashTableSCBF::HashTableSCBF(UINT uiLen,SCBF * filter)
{
	this->HASH_LEN = uiLen;
	for (ULONG i = 0; i<HASH_LEN; i++)
	{
		HashListSCBF *p = new HashListSCBF;
		if (NULL == p)
		{
			break;
		}
		m_pHashTable.push_back(p);
	}
	this->filter = filter;
}


HashTableSCBF::~HashTableSCBF()
{
}

void HashTableSCBF::SetFilter(SCBF * filter){
	this->filter = filter;
}

ULONG HashTableSCBF::TimeoutScan(Time tCur) {
	ULONG ulTotal = 0;
	for (ULONG i = 0; i<HASH_LEN; i++)
	{
		ulTotal += ((HashListSCBF *)m_pHashTable[i])->TimeoutScan(tCur,this->filter);
	}

	return ulTotal;
}