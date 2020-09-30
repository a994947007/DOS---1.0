#pragma once
#include "SCBF.h"
class DCBF :
    public SCBF
{
public:
    DCBF(ULONG h3_row, ULONG limit_capacity);	//第一个CBF的最佳容量和其哈希矩阵的行数
    virtual SCNode* newSCNode(ULONG h3_row, ULONG limit_capacity);
};

