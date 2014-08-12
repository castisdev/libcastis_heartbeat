#pragma once

#include "CiHBDefines.h"
#include <string>

class CCiHBResponser : public CNetworkThread
{
public:
	CCiHBResponser(const std::string& szRepresentativeIP,
					unsigned short iPortNumber,
					const std::string& szLocalIP);
	virtual ~CCiHBResponser();

	bool InitInstance();
	bool ExitInstance();

	CiHBState_t GetProcessState();

	void SetProcessHWError();
	void SetProcessSWError();
	void SetProcessAlive();

protected:
	ReceiveIntMessageResult_t ReceiveIntMessage(CCiSocket *pReadSocket, int *piReceivedMessage);
	bool OnMessage(CCiSocket *pReadSocket, int iMessage);
	bool OnHeartbeatRequest(CCiSocket *pReadSocket);
	bool OnReadSocketError(CCiSocket *pReadSocket);

	bool SendHeartbeatResponse(CCiSocket *pReadSocket, int iSeqNum);

	void SetProcessState(CiHBState_t state);

public:
	std::string m_szRepresentativeIP;
	std::string m_szLocalIP;
	unsigned short m_iPortNumber;

protected:
	CiHBState_t	m_processState;				/* ALIVE or SW_ERROR or HW_ERROR */

	struct sockaddr m_saRecv;				/* address of request machine */
	char m_szRecvBuf[CIHB_MAX_DATA_SIZE];
};
