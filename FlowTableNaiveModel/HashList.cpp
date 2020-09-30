#include "stdafx.h"
#include "HashList.h"


HashList::HashList():m_ulLen(0)
{
	m_list.prior = m_list.next = &m_list;
}

HashList::~HashList()
{
	Free();
}

ULONG HashList::Find(const FlowID & key, const FlowID & mask, PFlow & pNode) {
	ULONG ulSearchLen = 0;

	PFlow pCur;
	PFlow pHead = (PFlow)&m_list;

	LINKEDLIST_FOR_EACH(pCur, pHead)
	{
		ulSearchLen++;
		if (pCur->key == key && pCur->mask == mask)
		{
			break;
		}
	}

	if (pCur != &m_list)
	{
		Remove(pCur);
		pNode = pCur;
	}

	return ulSearchLen;
}

bool HashList::Insert(Flow * pNode) {
	PFlow pCur;
	PFlow pHead = (PFlow)&m_list;

	LINKEDLIST_FOR_EACH(pCur, pHead)
	{
		if (pCur->tLast < pNode->tLast)
			break;
	}

	LINKEDLIST_INSERT_NODE(pCur, pNode);
	m_ulLen++;

	return true;
}

bool HashList::Remove(Flow * pNode) {
	LINKEDLIST_DEL_NODE(pNode);
	m_ulLen--;

	return true;
}

void HashList::Free() {
	while (m_list.next != &m_list)
	{
		PFlow tmp = m_list.next;
		Remove(tmp);
		delete tmp;
		tmp = NULL;
	}
}


ULONG HashList::GetListLen()
{
	return m_ulLen;
}

ULONG HashList::TimeoutScan(Time tCur){
	ULONG ulNum = 0;
	PFlow pCur = m_list.next;
	while (pCur != &m_list)
	{
		PFlow tmp = pCur;
		pCur = pCur->next;
		Time t = tCur - tmp->tLast;
		if (t.sec >= FLOW_TIMEOUT)
		{
			Remove(tmp);	//º¬m_ulLen×Ô¼õ1
			delete tmp;
			tmp = NULL;
			ulNum++;
		}
	}

	return ulNum;
}

