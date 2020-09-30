#pragma once
#include "HashTable.h"
#include "HashListSCBF.h"
#include "SCBF.h"
class HashTableSCBF :
	public HashTable
{
public:
	HashTableSCBF(UINT);
	HashTableSCBF(UINT, SCBF *);
	~HashTableSCBF();

	void SetFilter(SCBF *);
	ULONG TimeoutScan(Time);

protected:
	SCBF * filter;
};

