#include "stdafx.h"
#include "HashListSCBF.h"


HashListSCBF::HashListSCBF():HashList()
{
}


HashListSCBF::~HashListSCBF()
{
}

ULONG HashListSCBF::TimeoutScan(Time tCur,SCBF * filter) {
	ULONG ulNum = 0;
	PFlow pCur = m_list.next;
	while (pCur != &m_list)
	{
		PFlow tmp = pCur;
		pCur = pCur->next;
		Time t = tCur - tmp->tLast;
		if (t.sec >= FLOW_TIMEOUT)
		{
			Remove(tmp);	//��m_ulLen�Լ�1
			filter->filter_delete(*(SCFlow *)tmp);
			delete tmp;
			tmp = NULL;
			ulNum++;
		}
	}

	return ulNum;
}
