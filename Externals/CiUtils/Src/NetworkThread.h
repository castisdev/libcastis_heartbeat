// NetworkThread.h: interface for the CNetworkThread class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NETWORKTHREAD_H__E790ED24_8D1D_44DA_AD3D_E3E300322181__INCLUDED_)
#define AFX_NETWORKTHREAD_H__E790ED24_8D1D_44DA_AD3D_E3E300322181__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _WIN32
#include <sys/poll.h>
#endif

#include "CiThread2.h"
#include "CiSocket.h"

#define NETWORK_THREAD_FD_SETSIZE					(1024)
#define NETWORK_THREAD_LISTEN_QUEUE_DEFAULTSIZE		(5)

typedef enum {
	RECEIVE_INT_MESSAGE_FALSE,
	RECEIVE_INT_MESSAGE_TRUE,
	RECEIVE_INT_MESSAGE_INTERIM
} ReceiveIntMessageResult_t;

class CNetworkThread : public CCiThread2
{
public:
	CNetworkThread();
	virtual ~CNetworkThread();

protected:
	// The read sockets of this network thread
	int m_iReadSocketCount;
	CCiSocket** m_ReadSockets;

#ifdef _WIN32
	fd_set m_fdSetAllRead;

	fd_set m_fdSetRead;

	int m_iMaximumSocket;
#else
	/* poll version */
	pollfd* m_PollFds;
	int	m_iPollFdCount;
	CCiSocket** m_PollSockets;
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
	bool FDSetAdd(CCiSocket* pSocket);
	bool FDSetDelete(CCiSocket* pSocket);
#else
	/* for poll */
	/* poll fd manipulation */
	bool PollFDAdd(CCiSocket *pSocket);
	bool PollFDDelete(CCiSocket *pSocket);
	bool PollFDEnable(pollfd *pPollFD);
	bool PollFDDisable(pollfd *pPollFD);
	pollfd *FindPollFD(CCiSocket *pSocket);
#endif

	/* select/poll timeout */
	void SetTimeoutMillisec(int iTimeoutMillisec);

	/* socket set management */
	CCiSocket* FindSocket(int iSocketFD);
	bool AddReadSocket(CCiSocket* pReadSocket);
	bool DeleteReadSocket(CCiSocket* pReadSocket, bool bPreserve = false);

	/* disable a socket in case of internal errors */
	bool DisableSocket(CCiSocket* pSocket);
	virtual bool OnReadSocketError(CCiSocket* pSocket);

	// implementations
	virtual bool InitInstance();
	virtual bool ExitInstance();
	virtual bool Run();

	// overridables
	virtual bool WaitNetworkEvent(int *piNEvent);
	virtual bool ProcessReadEvent();
	virtual bool ProcessTimeout();

	virtual ReceiveIntMessageResult_t ReceiveIntMessage(CCiSocket* pReadSocket, int *piReceivedMessage);
	virtual bool OnMessage(CCiSocket* pReadSocket, int iMessage);
};

#endif // !defined(AFX_NETWORKTHREAD_H__E790ED24_8D1D_44DA_AD3D_E3E300322181__INCLUDED_)
