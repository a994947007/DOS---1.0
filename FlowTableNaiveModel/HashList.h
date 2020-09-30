#pragma once

#include "uniform.h"

class HashList
{
public:
	HashList();
	virtual ~HashList();

	ULONG Find(const FlowID &, const FlowID &, PFlow &);
	bool Insert(Flow *);
	void Free();

	ULONG GetListLen();
	ULONG TimeoutScan(Time);

protected:
	bool Remove(Flow *);

	Flow m_list;
	ULONG m_ulLen;
};

