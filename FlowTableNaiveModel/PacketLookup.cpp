#include "stdafx.h"
#include "PacketLookup.h"
#include "HashTableSCBF.h"
#include "Log.h"
#include <time.h>

#pragma warning(disable:4996)

PacketLookup::PacketLookup(const UserConfig & info) :
	m_ulLookupNum(0),m_ulLookupLenNaive(0),m_ulLookupLenCBF(0),m_ulLookupLenSCBF(0),
	m_ulProbeNumNaive(0),m_ulProbeNumCBF(0),m_ulProbeNumSCBF(0),
	m_ulCBFErrorNum(0),m_ulCBFFindNum(0),
	m_ulSCBFErrorNum(0),m_ulSCBFFindNum(0),
	m_ulLookupLenDCBF(0), m_ulProbeNumDCBF(0), m_ulDCBFFindNum(0), m_ulDCBFErrorNum(0),
	m_ulLookupLenRBTreeSCBF(0),m_ulProbeNumRBTreeSCBF(0),m_ulRBTreeFindNum(0),m_ulRBTreeErrorNum(0),
	m_ulLookupLenRBTree(0)
{
	srand(unsigned(time(NULL)));

	for (ULONG i = 0; i < MASK_NUM; i++) {
		filter2017[i] = new SCBF(info.cbfCountersLen, ULONG_MAX);
		filter2019[i] = new SCBF(info.scbfCountersLen, info.scbfInitCapacity);			//SCBF极限容量设置是一个优化指标
		filterDCBF[i] = new DCBF(info.scbfCountersLen, info.scbfInitCapacity);		
		filterRBTreeSCBF[i] = new SCBF(info.scbfCountersLen, info.scbfInitCapacity);
		filterRBTree[i] = new SCBF(info.scbfCountersLen, info.scbfInitCapacity);

		m_tableNaive[i] = new HashTable(info.uiHashLen);
		m_tableSCBF[i] = new HashTableSCBF(info.uiHashLen,filter2019[i]);
		m_tableCBF[i] = new HashTableSCBF(info.uiHashLen,filter2017[i]);
		m_tableDCBF[i] = new HashTableSCBF(info.uiHashLen, filterDCBF[i]);
		m_tableRBTreeSCBF[i] = new RBTreeHashTable(info.uiHashLen, filterRBTreeSCBF[i]);
		m_tableRBTree[i] = new RBTreeHashTable(info.uiHashLen, filterRBTree[i]);
	}
}

PacketLookup::~PacketLookup()
{
	for (int i = 0; i < MASK_NUM; i++) {
		if (m_tableNaive[i] != NULL) {
			delete m_tableNaive[i];
			m_tableNaive[i] = NULL;
		}
		if (m_tableCBF[i] != NULL) {
			delete m_tableCBF[i];
			m_tableCBF[i] = NULL;
		}
		if (m_tableSCBF[i] != NULL) {
			delete m_tableSCBF[i];
			m_tableSCBF[i] = NULL;
		}
		if (m_tableDCBF[i] != NULL) { 
			delete m_tableDCBF[i];
			m_tableDCBF[i] = NULL;
		}
		if (m_tableRBTreeSCBF[i] != NULL) {
			delete m_tableRBTreeSCBF[i];
			m_tableRBTreeSCBF[i] = NULL;
		}
		if (m_tableRBTree[i] != NULL) {
			delete m_tableRBTree[i];
			m_tableRBTree[i] = NULL;
		}
		if (filter2017[i] != NULL) {
			delete filter2017[i];
			filter2017[i] = NULL;
		}
		if (filter2019[i] != NULL) {
			delete filter2019[i];
			filter2019[i] = NULL;
		}
		if (filterDCBF[i] != NULL) {
			delete filterDCBF[i];
			filterDCBF[i] = NULL;
		}
		if (filterRBTreeSCBF[i] != NULL) { 
			delete filterRBTreeSCBF[i];
			filterRBTreeSCBF[i] = NULL;
		}
		if (filterRBTree[i] != NULL) {
			delete filterRBTree[i];
			filterRBTree[i] = NULL;
		}
	}
}

bool PacketLookup::Initialize(const UserConfig & info) {
	ASSERT(!info.fileList.empty());

	list<CString> strList = info.fileList;
	list<CString>::iterator iter;
	for (iter = strList.begin(); iter != strList.end(); iter++)
	{
		ifstream *pFile = new ifstream;
		pFile->open(*iter, ios_base::in | ios_base::binary);
		if (pFile->fail())
		{
			LOG("open file failure!");
			return false;
		}

		m_fileStream.push_back(pFile);
	}

	//初始化元组掩码
	for (int i = 0; i < MASK_NUM_ROOT; i++) {
		for (int j = 0; j < MASK_NUM_ROOT; j++) {
			FlowID mask;
			mask.proto = MASK_PROTO;
			mask.src.ip = MASK_LIST[i];
			mask.src.port = MASK_PORT;
			mask.dst.ip = MASK_LIST[j];
			mask.dst.port = MASK_PORT;
			m_tableNaive[i * MASK_NUM_ROOT + j]->SetMask(mask);
			m_tableSCBF[i * MASK_NUM_ROOT + j]->SetMask(mask);
			m_tableCBF[i * MASK_NUM_ROOT + j]->SetMask(mask);
			m_tableDCBF[i * MASK_NUM_ROOT + j]->SetMask(mask);
			m_tableRBTreeSCBF[i * MASK_NUM_ROOT + j]->setMask(mask);
			m_tableRBTree[i * MASK_NUM_ROOT + j]->setMask(mask);
		}
	}
	return true;
}

void PacketLookup::LookupTestProc(const UserConfig & info) {
	bool blSuccess = Initialize(info);
	if (!blSuccess)
	{
		LOG("Initiate members failed!");
		return;
	}

	WriteResult(false, info);

	bool bIsFirst = true;
	Time tLastScan,tLastWrite;

	Time firstReadPktTime;
	bool readAttackPktFlag = false;

	 Packet pkt;
	bool pktFlag = true;
	while (pktFlag) {
		if (readAttackPktFlag) {	//获取一个攻击包
			pktFlag = GetPacket(pkt, info.attackPercent,info.attackPercent2);
		}
		else {				//获取一个正常包
			pktFlag = GetPacket(pkt);
		}

		m_ulLookupNum++;

		MaskProbeNaive(pkt);
	//	MaskProbeCBF(pkt);
		MaskProbeSCBF(pkt);
		MaskProbeRBTreeSCBF(pkt);
		MaskProbeRBTree(pkt);
	//	MaskProbeDCBF(pkt);

		if (bIsFirst) {
			tLastWrite = tLastScan = pkt.time;
			bIsFirst = false;
			firstReadPktTime = pkt.time;
		}
		else {
			Time t = pkt.time - tLastWrite;
			Time interval = pkt.time - firstReadPktTime;
			if (interval.sec >= info.attackStartTime && interval.sec <= info.attackEndTime) {
				readAttackPktFlag = true;
			}
			else {
				readAttackPktFlag = false;
			}
			if (t.sec >= 1) {
				WriteResult(true, info);
				tLastWrite = pkt.time;

				m_ulLookupNum = 0;

				m_ulLookupLenNaive = 0;
				m_ulLookupLenCBF = 0;
				m_ulLookupLenSCBF = 0;

				m_ulProbeNumNaive = 0;
				m_ulProbeNumCBF = 0;
				m_ulProbeNumSCBF = 0;
				
				m_ulCBFFindNum = 0;
				m_ulSCBFFindNum = 0;

				m_ulCBFErrorNum = 0;
				m_ulSCBFErrorNum = 0;

				m_ulLookupLenRBTreeSCBF = 0;
				m_ulProbeNumRBTreeSCBF = 0;
				m_ulRBTreeFindNum = 0;
				m_ulRBTreeErrorNum = 0;

				m_ulLookupLenRBTree = 0;

				/*
				m_ulLookupLenDCBF = 0;
				m_ulProbeNumDCBF = 0;
				m_ulDCBFFindNum = 0;
				m_ulDCBFErrorNum = 0;
				*/
			}

			t = pkt.time - tLastScan;

			if (t.usec >= SCAN_INTERVAL) {
				for (int i = 0; i < MASK_NUM; i++) {
					m_tableNaive[i]->TimeoutScan(pkt.time);
					m_tableSCBF[i]->TimeoutScan(pkt.time);
				//	m_tableCBF[i]->TimeoutScan(pkt.time);
				//	m_tableDCBF[i]->TimeoutScan(pkt.time);
					m_tableRBTreeSCBF[i]->timeoutScan(pkt.time);
					m_tableRBTree[i]->timeoutScan(pkt.time);
				}

				tLastScan = pkt.time;
			}
		}
	}
}

bool PacketLookup::MaskProbeNaive(const Packet & pkt) {
	ASSERT(pkt.proto == PROTO_TCP || pkt.proto == PROTO_UDP);

	const FlowID fid = CalcFlowID(pkt.proto, pkt.src, pkt.dst);
	Flow *pNode = NULL;
	ULONG index = INDEX_INVALID;

	for (ULONG i = 0; i < MASK_NUM; i++) {
		m_ulLookupLenNaive += m_tableNaive[i]->Find(fid, pNode);
		if (pNode != NULL) {
			index = i;
			break;
		}
		m_ulProbeNumNaive++;
	}

	if (pNode == NULL) {
		pNode = NewFlow(pkt, fid);
		for (ULONG j = 0; j<MASK_NUM; j++) {
			if (pNode->mask == m_tableNaive[j]->mask) {
				index = j;
				break;
			}
		}
	}

	UpdateFlow(pNode, pkt, fid);
	m_tableNaive[index]->Insert(pNode);

	return true;
}

bool PacketLookup::MaskProbeCBF(const Packet & pkt) {
	ASSERT(pkt.proto == PROTO_UDP || pkt.proto == PROTO_TCP);
	const FlowID fid = CalcFlowID(pkt.proto, pkt.src, pkt.dst);

	Flow * pNode = NULL;
	ULONG index = INDEX_INVALID;
	for (ULONG i = 0; i < MASK_NUM; i++) {
		ULONG bfLen = 0;
		bool flag = filter2017[i]->filter_query((FlowID)fid & m_tableCBF[i]->mask, bfLen);
		m_ulCBFFindNum++;
		m_ulLookupLenCBF += 1;
		if (flag) {
			m_ulLookupLenCBF += m_tableCBF[i]->Find(fid, pNode);
		}
		if (pNode != NULL) {
			index = i;
			break;
		}else if (flag && pNode == NULL) {
			m_ulCBFErrorNum++;
		}
		m_ulProbeNumCBF++;
	}

	if (pNode == NULL) {
		pNode = NewSCFlow(pkt, fid);
		for (ULONG j = 0; j<MASK_NUM; j++) {
			if (pNode->mask == m_tableCBF[j]->mask) {
				index = j;
				break;
			}
		}
		filter2017[index]->filter_insert(*(SCFlow *)pNode);
	}

	UpdateFlow(pNode, pkt, fid);
	m_tableCBF[index]->Insert(pNode);
	return true;
}

bool PacketLookup::MaskProbeSCBF(const Packet & pkt) {
	ASSERT(pkt.proto == PROTO_UDP || pkt.proto == PROTO_TCP);
	const FlowID fid = CalcFlowID(pkt.proto, pkt.src, pkt.dst);

	Flow * pNode = NULL;
	ULONG index = INDEX_INVALID;
	for (ULONG i = 0; i < MASK_NUM; i++) {
		ULONG bfLen = 0;
		bool flag = filter2019[i]->filter_query((FlowID)fid & m_tableSCBF[i]->mask, bfLen);
		m_ulSCBFFindNum++;
		m_ulLookupLenSCBF+= 1;
		if (flag) {
			m_ulLookupLenSCBF += m_tableSCBF[i]->Find(fid, pNode);
		}
		if (NULL != pNode) {
			index = i;
			break;
		}
		else if(flag && pNode == NULL) {
			m_ulSCBFErrorNum++;
		}
		m_ulProbeNumSCBF++;
	}

	if (pNode == NULL) {
		pNode = NewSCFlow(pkt, fid);
		for (ULONG i = 0; i < MASK_NUM; i++) {
			if (pNode->mask == m_tableSCBF[i]->mask) {
				index = i;
				break;
			}
		}
		filter2019[index]->filter_insert(*(SCFlow *)pNode);
	}
	UpdateFlow(pNode, pkt, fid);
	m_tableSCBF[index]->Insert(pNode);
	return true;
}

bool PacketLookup::MaskProbeDCBF(const Packet& pkt) {
	ASSERT(pkt.proto == PROTO_UDP || pkt.proto == PROTO_TCP);
	const FlowID fid = CalcFlowID(pkt.proto, pkt.src, pkt.dst);

	Flow* pNode = NULL;
	ULONG index = INDEX_INVALID;
	for (ULONG i = 0; i < MASK_NUM; i++) {
		ULONG bfLen = 0;
		bool flag = filterDCBF[i]->filter_query((FlowID)fid & m_tableDCBF[i]->mask,bfLen);
		m_ulDCBFFindNum++;
		m_ulLookupLenDCBF += 1;
		if (flag) {
			m_ulLookupLenDCBF += m_tableDCBF[i]->Find(fid, pNode);
		}
		if (NULL != pNode) {
			index = i;
			break;
		}
		else if(flag && pNode == NULL){
			m_ulDCBFErrorNum++;
		}
		m_ulProbeNumDCBF++;
	}

	if (pNode == NULL) {
		pNode = NewSCFlow(pkt, fid);
		for (ULONG i = 0; i < MASK_NUM; i++) {
			if (pNode->mask == m_tableDCBF[i]->mask) {
				index = i;
				break;
			}
		}
		filterDCBF[index]->filter_insert(*(SCFlow*)pNode);
	}
	UpdateFlow(pNode, pkt, fid);
	m_tableDCBF[index]->Insert(pNode);
	return true;
}

bool PacketLookup::MaskProbeRBTreeSCBF(const Packet& pkt)
{
	ASSERT(pkt.proto == PROTO_UDP || pkt.proto == PROTO_TCP);
	const FlowID fid = CalcFlowID(pkt.proto, pkt.src, pkt.dst);

	Flow* pNode = NULL;
	ULONG index = INDEX_INVALID;
	for (ULONG i = 0; i < MASK_NUM; i++) {
		ULONG bfLen = 0;
		bool flag = filterRBTreeSCBF[i]->filter_query((FlowID)fid & m_tableRBTreeSCBF[i]->mask, bfLen);
		m_ulRBTreeFindNum++;
		m_ulLookupLenRBTreeSCBF += 1;
		if (flag) {
			m_ulLookupLenRBTreeSCBF += m_tableRBTreeSCBF[i]->find(fid, pNode);
		}
		if (NULL != pNode) {
			index = i;
			break;
		}
		else if (flag && pNode == NULL) {
			m_ulRBTreeErrorNum++;
		}
		m_ulProbeNumRBTreeSCBF++;
	}

	if (pNode == NULL) {
		pNode = NewSCFlow(pkt, fid);
		for (ULONG i = 0; i < MASK_NUM; i++) {
			if (pNode->mask == m_tableRBTreeSCBF[i]->mask) {
				index = i;
				break;
			}
		}
		filterRBTreeSCBF[index]->filter_insert(*(SCFlow*)pNode);
		m_tableRBTreeSCBF[index]->insert(pNode);
	}
	UpdateFlow(pNode, pkt, fid);
	return true;
}

bool PacketLookup::MaskProbeRBTree(const Packet& pkt) {
	ASSERT(pkt.proto == PROTO_UDP || pkt.proto == PROTO_TCP);
	const FlowID fid = CalcFlowID(pkt.proto, pkt.src, pkt.dst);

	Flow* pNode = NULL;
	ULONG index = INDEX_INVALID;
	for (ULONG i = 0; i < MASK_NUM; i++) {
		ULONG bfLen = 0;
		bool flag = filterRBTree[i]->filter_query((FlowID)fid & m_tableRBTree[i]->mask, bfLen);
		//m_ulRBTreeFindNum++;
		//m_ulLookupLenRBTreeSCBF += 1;
		//if (flag) {
			m_ulLookupLenRBTree += m_tableRBTree[i]->find(fid, pNode);
		//}
		if (NULL != pNode) {
			index = i;
			break;
		}
		//else if (flag && pNode == NULL) {
		//	m_ulRBTreeErrorNum++;
		//}
		//m_ulProbeNumRBTreeSCBF++;
	}

	if (pNode == NULL) {
		pNode = NewSCFlow(pkt, fid);
		for (ULONG i = 0; i < MASK_NUM; i++) {
			if (pNode->mask == m_tableRBTree[i]->mask) {
				index = i;
				break;
			}
		}
		filterRBTree[index]->filter_insert(*(SCFlow*)pNode);
		m_tableRBTree[index]->insert(pNode);
	}
	UpdateFlow(pNode, pkt, fid);
	return true;
}

//计算流标识符
const FlowID PacketLookup::CalcFlowID(UCHAR proto, const EndID & src, const EndID & dst) {
	FlowID fid;
	fid.proto = proto;
	fid.src = src;
	fid.dst = dst;
	
	return fid;
}

//新建流表项
Flow * PacketLookup::NewFlow(const Packet & pkt, const FlowID & fid) { 
	Flow * pNew = new Flow;
	if (NULL == pNew) {
		LOG("new flow error");
		return pNew;
	}

	pNew->mask.proto = MASK_PROTO;
	pNew->mask.src.ip = GetIpMask(fid.src.ip);
	pNew->mask.src.port = MASK_PORT;
	pNew->mask.dst.ip = GetIpMask(fid.dst.ip);
	pNew->mask.dst.port = MASK_PORT;
	pNew->key = (FlowID)fid&pNew->mask;
	
	return pNew;
}

Flow * PacketLookup::NewSCFlow(const Packet & pkt,const FlowID & fid){
	Flow * pNew = new SCFlow;
	if (NULL == pNew) {
		LOG("new SCFlow error");
		return pNew;
	}

	pNew->mask.proto = MASK_PROTO;
	pNew->mask.src.ip = GetIpMask(fid.src.ip);
	pNew->mask.src.port = MASK_PORT;
	pNew->mask.dst.ip = GetIpMask(fid.dst.ip);
	pNew->mask.dst.port = MASK_PORT;
	pNew->key = (FlowID)fid&pNew->mask;

	return pNew;
}

//获取对应IP地址的对应掩码，按前两个字节进行分类
UINT PacketLookup::GetIpMask(UINT ip)
{
	if (0x00000000 == (ip & 0xC0000000))		//00类地址
		return MASK_LIST[0];
	else if (0x40000000 == (ip & 0xC0000000)) {	//01类地址
		return MASK_LIST[1];
	}
	else if (0x80000000 == (ip & 0xC0000000))	//10类地址
		return MASK_LIST[2];
	else									//11类地址
		return MASK_LIST[3];
}

bool PacketLookup::UpdateFlow(Flow * pNode, const Packet & pkt, const FlowID & fid) {
	ASSERT(pNode != NULL && (pkt.proto == PROTO_TCP || pkt.proto == PROTO_UDP));
	pNode->tLast = pkt.time;
	return true;
}

bool PacketLookup::WriteResult(bool bIsOn, const UserConfig & info) {
	CString strFile = info.resultPath + "/" + "Result.txt";
	CStringA _path(strFile);
	char * path = _path.GetBuffer();

	FILE * pfile = NULL;

	if (!bIsOn) {
		if ((pfile = fopen(path, "w")) == NULL) {
			LOG("OpenFile failure");
			return false;
		}
		fprintf(pfile,"哈希表长度：%d，CBF_LEN：%d(2的乘方)，SCBF_INIT_LEN：%d(2的乘方)，SCBF_INIT_CAPACITY：%d，攻击包占比：%d，攻击时间：%ds-%ds\r\n",info.uiHashLen,info.cbfCountersLen,info.scbfCountersLen,info.scbfInitCapacity,info.attackPercent,info.attackStartTime,info.attackEndTime);
	}
	else {
		if ((pfile = fopen(path, "at+")) == NULL) {
			LOG("OpenFile failure");
			return false;
		}
		double dAslNaive = (double)m_ulLookupLenNaive / m_ulLookupNum;
		double dProbeNumNaive = (double)m_ulProbeNumNaive / m_ulLookupNum;

		double dAslCBF = (double)m_ulLookupLenCBF / m_ulLookupNum;
		double dProbeNumCBF = (double)m_ulProbeNumCBF / m_ulLookupNum;
		double dCBFError = (double)m_ulCBFErrorNum / m_ulCBFFindNum;

		double dAslSCBF = (double)m_ulLookupLenSCBF / m_ulLookupNum;
		double dProbeNumSCBF = (double)m_ulProbeNumSCBF / m_ulLookupNum;
		double dSCBFError = (double)m_ulSCBFErrorNum / m_ulSCBFFindNum;

		/*
		double dAslDCBF = (double)m_ulLookupLenDCBF / m_ulLookupNum;
		double dProbeNumDCBF = (double)m_ulProbeNumDCBF / m_ulLookupNum;
		double dDCBFError = (double)m_ulDCBFErrorNum / m_ulDCBFFindNum;
		*/
		

		double dAslRBTreeSCBF = (double)m_ulLookupLenRBTreeSCBF / m_ulLookupNum;
		double dProbeNumRBTreeSCBF = (double)m_ulProbeNumRBTreeSCBF / m_ulLookupNum;
		double dRBTreeSCBFError = (double)m_ulRBTreeErrorNum / m_ulRBTreeFindNum;

		double dAslRBTree = (double)m_ulLookupLenRBTree / m_ulLookupNum;


		//fprintf(pfile, "%d\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\r\n", GetFlowNum(m_tableNaive), dAslNaive, dProbeNumNaive,dAslCBF,dProbeNumCBF, dCBFError,dAslSCBF,dProbeNumSCBF,dSCBFError,dAslDCBF,dProbeNumDCBF,dDCBFError);
		//fprintf(pfile, "%d\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n", GetFlowNum(m_tableNaive), dAslNaive, dProbeNumNaive, dAslCBF, dProbeNumCBF, dCBFError, dAslSCBF, dProbeNumSCBF, dSCBFError);
		//fprintf(pfile, "%d\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\r\n", GetFlowNum(m_tableNaive), dAslNaive, dProbeNumNaive, dAslCBF, dProbeNumCBF, dCBFError, dAslSCBF, dProbeNumSCBF, dSCBFError, dAslRBTreeSCBF, dProbeNumRBTreeSCBF, dRBTreeSCBFError);
		fprintf(pfile, "%d\t%f\t%f\t%f\t%f\n", GetFlowNum(m_tableNaive), dAslNaive, dAslSCBF, dAslRBTree, dAslRBTreeSCBF );
	}

	if (pfile != NULL) {
		fclose(pfile);
		pfile = NULL;
	}

	return true;
}

bool PacketLookup::GetPacket(Packet & pkt) {
	return GetNormalPacket(pkt);
}

//读取一个数据包
bool PacketLookup::GetPacket(Packet &pkt,ULONG attackPercent,ULONG attackPercent2)
{
	if (rand() % 100 < (double)attackPercent/(attackPercent + attackPercent2) * 100) {
		return GetAttackPacket(pkt);
	}
	else {
		return GetNormalPacket(pkt);
	}
	return false;
}

ULONG PacketLookup::GetFlowNum(HashTable ** tuples) {
	int count = 0;
	for (int i = 0; i<MASK_NUM; i++) {
		count += tuples[i]->GetTotalNum();
	}
	return count;
}

/*获取一个正常包*/
bool PacketLookup::GetNormalPacket(Packet & pkt) {
	while (!m_fileStream.empty()) {
		ifstream * f = m_fileStream.front();
		while (!f->eof())
		{
			bool bRet = ReadPacket(*f, pkt);
			if (!bRet)
			{
				LOG("read packet error!");
				continue;
			}
			return true;
		}
		f->close();
		f->clear();
		m_fileStream.pop_front();
		delete f;
		f = NULL;
	}
	return false;
}

/*获取一个攻击包*/
bool PacketLookup::GetAttackPacket(Packet & pkt) {
	bool flag = false;
	flag = GetNormalPacket( pkt);
	//修改源地址为随机
	ULONG ip = (rand() % 256 << 24) | (pkt.src.ip & 0x00ffffff);
	pkt.src.ip = ip;
	return flag;
}


bool PacketLookup::ReadPacket(ifstream & ifFile, Packet & pkt) {
	//读取数据包的时间戳
	ifFile.read((char *)&(pkt.time), sizeof(pkt.time));
	if (!ifFile)
	{
		TRACE("read timestamp failure!\n");
		return false;
	}

	//读取IP首部数据
	IpHeader *pIpHeader = (IpHeader *) new char[sizeof(IpHeader)];
	if (NULL == pIpHeader)
	{
		TRACE("new array error!");
		return false;
	}

	ifFile.read((char *)pIpHeader, sizeof(IpHeader));
	if (!ifFile)
	{
		TRACE("read IP header failure!\n");
		return false;
	}

	//提取IP首部关键字段信息
	USHORT usIpHdrLen = (pIpHeader->ver_ihl & 0x0f) * 4;//提取IP头部长度
	USHORT usIpTotalLen = ntohs(pIpHeader->total_len); 	//提取IP包总长度
	pkt.src.ip = ntohl(pIpHeader->srcIP);
	pkt.dst.ip = ntohl(pIpHeader->dstIP);
	pkt.proto = pIpHeader->protocol;

	delete[] pIpHeader;
	pIpHeader = NULL;

	//若有IP选项，跳过
	if (usIpHdrLen > 20)
	{
		ifFile.seekg(4, ios_base::cur);
		if (!ifFile)
		{
			TRACE("seek file failure!\n");
			return false;
		}
	}

	//根据协议字段区分提取传输层内容
	if (pkt.proto == PROTO_TCP)
	{
		//读取TCP首部数据
		TcpHeader *pTcpHeader = (TcpHeader *) new char[sizeof(TcpHeader)];
		if (NULL == pTcpHeader)
		{
			TRACE("new array error!");
			return false;
		}

		ifFile.read((char *)pTcpHeader, sizeof(TcpHeader));
		if (!ifFile)
		{
			TRACE("read TCP header failure!\n");
			return false;
		}

		//提取端口号
		pkt.src.port = ntohs(pTcpHeader->src_port);
		pkt.dst.port = ntohs(pTcpHeader->dst_port);

		delete[] pTcpHeader;
		pTcpHeader = NULL;
	}
	else if (pkt.proto == PROTO_UDP)
	{
		//读取UDP首部数据
		UdpHeader *pUdpHeader = (UdpHeader *)new char[sizeof(UdpHeader)];
		if (NULL == pUdpHeader)
		{
			TRACE("new array error!");
			return false;
		}

		ifFile.read((char *)pUdpHeader, sizeof(UdpHeader));
		if (!ifFile)
		{
			TRACE("read UDP Header Failure!\n");
			return false;
		}

		//提取端口号
		pkt.src.port = ntohs(pUdpHeader->src_port);
		pkt.dst.port = ntohs(pUdpHeader->dst_port);

		delete[] pUdpHeader;
		pUdpHeader = NULL;
	}
	else	//非TCP和UDP，如GRE(47)
	{
		ifFile.seekg(8, ios_base::cur);
		if (!ifFile)
			TRACE("seek file failure!\n");

		return false;
	}

	return true;
}

