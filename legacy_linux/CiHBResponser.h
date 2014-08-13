#pragma once

#include <string>

#include "legacy_support.h"
#include "CiHBDefines.h"

class CCiHBResponser : public cihb::CNetworkThread
{
public:
	CCiHBResponser(const std::string& szRepresentativeIP,
					unsigned short iPortNumber,
					const std::string& szLocalIP,
					int iTimeoutMillisec = 0);
	virtual ~CCiHBResponser();

	bool InitInstance();

	CiHBState_t GetProcessState();

	void SetProcessHWError();
	void SetProcessSWError();
	void SetProcessAlive();

protected:
	bool ReceiveIntMessage(int sockfd, int *piReceivedMessage);
	bool OnMessage(int sockfd, int iMessage);
	bool OnReadSocketError(int sockfd);

	bool OnHeartbeatRequest(int sockfd);
	bool SendHeartbeatResponse(int sockfd, int iSeqNum);
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
