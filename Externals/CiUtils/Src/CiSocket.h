// CCiSocket.h: interface for the CCCiSocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CISOCKET_H__81C762E2_55BB_4440_96F4_4E28594C5809__INCLUDED_)
#define AFX_CISOCKET_H__81C762E2_55BB_4440_96F4_4E28594C5809__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include "CiSemaphore.h"
#include "NetUtil.h"

#define CI_INVALID_SOCKET		(-1)
#define CI_INVALID_PORT_NUMBER	(-1)

#define CI_SOCKET_SEND_BUFFER_SIZE		(100*1024)	// 100K
#define CI_SOCKET_RECV_BUFFER_SIZE		(100*1024)	// 100K

#define CI_SOCKET_DEFAULT_CONNECT_TIME_OUT	(2*1000)		// 2 sec
#define CI_SOCKET_DEFAULT_SEND_TIME_OUT	(2*1000)	// 2 sec
#define CI_SOCKET_DEFAULT_RECV_TIME_OUT	(3*1000)	// 3 sec

typedef enum {
	CI_SOCKET_TCP,
	CI_SOCKET_UDP
} CCiSocketType_t;

class CCiSocket
{
public:
	CCiSocket(CCiSocketType_t socketType=CI_SOCKET_TCP,
				int iSendBufferSize = CI_SOCKET_SEND_BUFFER_SIZE,
				int iRecvBufferSize = CI_SOCKET_RECV_BUFFER_SIZE);	// Client Constructor
	CCiSocket(int iAcceptSocket,
				CCiSocketType_t socketType = CI_SOCKET_TCP,
				int iSendBufferSize = CI_SOCKET_SEND_BUFFER_SIZE,
				int iRecvBufferSize = CI_SOCKET_RECV_BUFFER_SIZE);	// Server Constructor
	virtual ~CCiSocket();

	bool	Initialize(int iAcceptSocket = CI_INVALID_SOCKET,
							CCiSocketType_t socketType = CI_SOCKET_TCP);

	bool	Finalize() { return (m_iSocket == CI_INVALID_SOCKET) || Disconnect(); }

	bool	Connect(const char* pszServerIPAddress,
						unsigned short iPortNumber,
						const char* pszSourceIPAddress = NULL,	// local interface ip
						int iSocketBufferSize = -1);
	bool	Disconnect();

	void	SetAcceptSocket(int iAcceptSocket) { m_iSocket = iAcceptSocket; }
	int		GetLastError() const { return nu_get_last_error(); }

	////////////////////////////////////////////////////////////////////
	// Server Method
	//
	// Buffer에서 읽고 쓴다.
	bool	WriteInt16(short sInt16);
	bool	WriteInt32(int iInt32);
	bool	WriteInt64(long long llInt64);
	bool	WriteN(const void* pBuf, int ntowrite, int* pnwritten);

	bool	ReadInt16(short* psInt16);
	bool	ReadInt32(int* piInt32);
	bool	ReadInt64(long long* pllInt64);
	bool	ReadN(void* pBuf, int ntoread, int* pnread);

	bool	LookAheadInt32(int* piInt32, int iOffset);	// m_iRecvStartPtr에서 offset만큼 떨어져서 4byte를 읽고자 하는경우( Buffer에서 안 지움 )
	bool	LookAheadN(void* pBuf, int ntoReadSize, int iOffset);	// m_iRecvStartPtr에서 offset만큼 떨어져서 ntoReadSize를 읽고자 하는경우( Buffer에서 안 지움 )

	bool	ReadFromNetworkToBuffer();		// recv 하여 buffer에 쓴다..

	// by gun 2004.12.13
	// parameter로 네트웍에서 읽을 MAX값을 넘겨 받는 함수 추가
	bool	ReadFromNetworkToBuffer(int iReadSize);

	int		GetCurrentRecvDataSize();

	bool	WriteFromBufferToNetwork();		// buffer에 있는 것을 send한다.
	int		GetRemainSendDataSize();

	std::string GetPeerIP() const;

	int	m_iSocket;

protected:
	CCiSocketType_t	m_socketType;

	int m_iConnectTimeOutMillieSec;
	int m_iSendTimeOutMillieSec;
	int m_iRecvTimeOutMillieSec;


	char*	m_pSendBuffer;
	char*	m_pRecvBuffer;

	int m_iSendBufferSize;
	int m_iRecvBufferSize;

	int	m_iSendStartPtr;
	int m_iSendEndPtr;
	int	m_iRecvStartPtr;
	int m_iRecvEndPtr;

	bool	WriteNToBuffer(const void* pBuf, int ntowrite, int* pnwritten);

private:
	bool	SetNonBlockSocket();
};

#endif // !defined(AFX_CISOCKET_H__81C762E2_55BB_4440_96F4_4E28594C5809__INCLUDED_)
