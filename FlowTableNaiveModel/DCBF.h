#pragma once
#include "SCBF.h"
class DCBF :
    public SCBF
{
public:
    DCBF(ULONG h3_row, ULONG limit_capacity);	//��һ��CBF��������������ϣ���������
    virtual SCNode* newSCNode(ULONG h3_row, ULONG limit_capacity);
};

