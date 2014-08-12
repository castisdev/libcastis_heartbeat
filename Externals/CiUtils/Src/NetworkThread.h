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
	CNetworkThread(int iListenPortNumber
		, int iConnectedSocketSendBufferSize = CI_SOCKET_SEND_BUFFER_SIZE
		, int iConnectedSocketRecvBufferSize = CI_SOCKET_RECV_BUFFER_SIZE
		, int iListenQueueSize = NETWORK_THREAD_LISTEN_QUEUE_DEFAULTSIZE
		, std::string listenIPAddr = "0.0.0.0"
		, int iMaxFDSize = NETWORK_THREAD_FD_SETSIZE);

	virtual ~CNetworkThread();

protected:
	// TCP listen port number
	int m_iListenPortNumber;
	std::string m_ListenIPAddr;

	// TCP listen socket
	/* The listen socket will be created in InitInstance(), */
	/* and destroyed in Destructor() */
	CCiSocket* m_pListenSocket;

	// The read sockets of this network thread
	int m_iReadSocketCount;
	CCiSocket** m_ReadSockets;

	// The write sockets of this network thread
	int m_iWriteSocketCount;
	CCiSocket** m_WriteSockets;

#ifdef _WIN32
	fd_set m_fdSetAllRead;
	fd_set m_fdSetAllWrite;

	fd_set m_fdSetRead;
	fd_set m_fdSetWrite;

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

	int m_iConnectedSocketSendBufferSize;
	int m_iConnectedSocketRecvBufferSize;

	int m_iListenQueueSize;

private:
	const int m_iMaxFDSize;

public:
	int GetMaxFDSize() const { return m_iMaxFDSize; }

	int GetConnectedSocketSendBufferSize() const { return m_iConnectedSocketSendBufferSize; }
	int GetConnectedSocketRecvBufferSize() const { return m_iConnectedSocketRecvBufferSize; }

	void SetConnectedSocketSendBufferSize(int val) { m_iConnectedSocketSendBufferSize = val; }
	void SetConnectedSocketRecvBufferSize(int val) { m_iConnectedSocketRecvBufferSize = val; }

#ifdef _WIN32
	/* for select */
	/* fd set manipulation */
	bool FDSetAdd(CCiSocket* pSocket, bool bReadCase = true);
	bool FDSetDelete(CCiSocket* pSocket, bool bReadCase = true);
#else
	/* for poll */
	/* poll fd manipulation */
	bool PollFDAdd(CCiSocket *pSocket, bool bReadCase = true);
	bool PollFDDelete(CCiSocket *pSocket);
	bool PollFDEnable(pollfd *pPollFD, bool bReadCase = true);
	bool PollFDDisable(pollfd *pPollFD, bool bReadCase = true);
	pollfd *FindPollFD(CCiSocket *pSocket);
#endif

	/* select/poll timeout */
	void SetTimeoutMillisec(int iTimeoutMillisec);

	/* socket set management */
	CCiSocket* FindSocket(int iSocketFD, bool bReadCase = true);
	bool AddReadSocket(CCiSocket* pReadSocket);
	bool DeleteReadSocket(CCiSocket* pReadSocket, bool bPreserve = false);
	bool AddWriteSocket(CCiSocket* pWriteSocket);
	bool DeleteWriteSocket(CCiSocket* pWriteSocket, bool bPreserve = false);

	/* disable a socket in case of internal errors */
	bool DisableSocket(CCiSocket* pSocket);
	virtual bool OnReadSocketError(CCiSocket* pSocket);

	// implementations
	virtual bool InitInstance();
	virtual bool ExitInstance();
	virtual bool Run();

	// overridables
	virtual bool WaitNetworkEvent(int *piNEvent);
    virtual bool ProcessConnection(CCiSocket **ppConnectedSocket);
	virtual bool ProcessReadEvent();
	virtual bool ProcessWriteEvent();
	virtual bool ProcessTimeout();

	virtual ReceiveIntMessageResult_t ReceiveIntMessage(CCiSocket* pReadSocket, int *piReceivedMessage);
	virtual bool OnMessage(CCiSocket* pReadSocket, int iMessage);
	virtual bool DoWrite(CCiSocket* pWriteSocket);

	virtual bool ProcessWithConnectedSocket(CCiSocket* /*socket*/) { return true; }
};

#endif // !defined(AFX_NETWORKTHREAD_H__E790ED24_8D1D_44DA_AD3D_E3E300322181__INCLUDED_)
