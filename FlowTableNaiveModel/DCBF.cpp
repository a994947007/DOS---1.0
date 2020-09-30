#include "stdafx.h"
#include "DCBF.h"

DCBF::DCBF(ULONG h3_row, ULONG limit_capacity):SCBF(h3_row,limit_capacity)
{   
}

SCNode* DCBF::newSCNode(ULONG h3_row, ULONG limit_capacity)
{
    return new SCNode(h3_row, limit_capacity);
}
