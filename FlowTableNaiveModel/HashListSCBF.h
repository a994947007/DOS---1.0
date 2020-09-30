#pragma once
#include "HashList.h"
#include "SCBF.h"

class HashListSCBF :
	public HashList
{
public:
	HashListSCBF();
	~HashListSCBF();

	ULONG TimeoutScan(Time,SCBF *);
};

