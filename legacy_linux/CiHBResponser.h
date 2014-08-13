#pragma once

#include <set>
#include <vector>

#include "CiHBDefines.h"

#define NETWORK_THREAD_FD_SETSIZE					(1024)
#define NETWORK_THREAD_LISTEN_QUEUE_DEFAULTSIZE		(5)

typedef enum {
	RECEIVE_INT_MESSAGE_FALSE,
	RECEIVE_INT_MESSAGE_TRUE,
	RECEIVE_INT_MESSAGE_INTERIM
} ReceiveIntMessageResult_t;

class CCiHBResponser : public CCiThread2
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
	ReceiveIntMessageResult_t ReceiveIntMessage(int sockfd, int *piReceivedMessage);
	bool OnMessage(int sockfd, int iMessage);
	bool OnHeartbeatRequest(int sockfd);
	bool OnReadSocketError(int sockfd);

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


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/// from network thread
///
protected:
	// The read sockets of this network thread
	std::set<int> m_ReadSockets;

#ifdef _WIN32
	fd_set m_fdSetAllRead;

	fd_set m_fdSetRead;

	int m_iMaximumSocket;
#else
	/* poll version */
	pollfd* m_PollFds;
	int	m_iPollFdCount;
#endif

	// Events Count for immediate break to save the time
	int m_iEventsCount;

	// select time
	int	m_iTimeoutMillisec;

private:
	const int m_iMaxFDSize;

public:
	int GetMaxFDSize() const { return m_iMaxFDSize; }

#ifdef _WIN32
	/* for select */
	/* fd set manipulation */
	bool FDSetAdd(int sockfd);
	bool FDSetDelete(int sockfd);
#else
	/* for poll */
	/* poll fd manipulation */
	bool PollFDAdd(int sockfd);
	bool PollFDDelete(int sockfd);
	bool PollFDEnable(pollfd *pPollFD);
	bool PollFDDisable(pollfd *pPollFD);
#endif

	/* socket set management */
	bool AddReadSocket(int sockfd);
	bool DeleteReadSocket(int sockfd);

	// implementations
	virtual bool ExitInstance();
	virtual bool Run();

	// overridables
	virtual bool WaitNetworkEvent(int *piNEvent);
	virtual bool ProcessReadEvent();
};
