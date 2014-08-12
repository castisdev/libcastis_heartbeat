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

	CCiSocketType_t	GetSocketType() const { return m_socketType; }
	void	SetAcceptSocket(int iAcceptSocket) { m_iSocket = iAcceptSocket; }
	int		GetLastError() const { return nu_get_last_error(); }
	bool	IsValidSocket() const { return m_iSocket != CI_INVALID_SOCKET; }

	// Client Method
	void	SetConnectTimeOut(int iMillieSec = CI_SOCKET_DEFAULT_CONNECT_TIME_OUT) { m_iConnectTimeOutMillieSec = iMillieSec; }
	void	SetSendTimeOut(int iMillieSec = CI_SOCKET_DEFAULT_SEND_TIME_OUT) { m_iSendTimeOutMillieSec = iMillieSec; }
	void	SetRecvTimeOut(int iMillieSec = CI_SOCKET_DEFAULT_RECV_TIME_OUT) { m_iRecvTimeOutMillieSec = iMillieSec; }

	int		GetSendTimeOut() const { return m_iSendTimeOutMillieSec; }
	int		GetRecvTimeOut() const { return m_iRecvTimeOutMillieSec; }

	bool	WriteInt16WithTimeOut(short sInt16);
	bool	WriteInt32WithTimeOut(int iInt32);
	bool	WriteInt64WithTimeOut(long long llInt64);
	bool	WriteNWithTimeOut(const void* pBuf, int ntowrite, int* pnwritten);

	bool	ReadInt16WithTimeOut(short* psInt16);
	bool	ReadInt32WithTimeOut(int* piInt32);
	bool	ReadInt64WithTimeOut(long long* pllInt64);
	bool	ReadNWithTimeOut(void* pBuf, int ntoread, int* pnread);
	bool	ReadPatternWithTimeOut(void* pBuf, const char *pattern, int readsize, int* pnread);
				/* by nuri 2005.1.5
				 * 특정 pattern이 나올 때까지 네트웍에서 데이터를 읽는다.
				 * readsize만큼씩 recv시도를 하며, 지정된 시간 안에 읽지 못하면 false를 반환
				 */

	/* 2003.09.22 by nuri */
	bool	GetBufferClon(CCiSocket &rhs);

	////////////////////////////////////////////////////////////////////
	// Server Method
	//
	// Buffer에서 읽고 쓴다.
	bool	WriteInt16(short sInt16);
	bool	WriteInt32(int iInt32);
	bool	WriteInt64(long long llInt64);
	bool	WriteN(const void* pBuf, int ntowrite, int* pnwritten);
		// 2004.04.19 by NURI
		// 기존의 buffer를 이용한 WriteN() 버전을 수정하였음.
		// non-blocking socket의 경우에도 보내고자 하는 데이터를 모두 보내려고 시도한 후에 리턴.
		// false를 리턴하는 경우, errno로 에러값을 체크할 수 있으며
		// true를 리턴하는 경우에도 *pnwritten값이 ntowrite값과 같은지 체크 요망.
	bool	WriteNFlush(const void* pBuf, int ntowrite, int* pnwritten);

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

	int		StrExistInReadBuffer(const char *pStr);	// 2004.11.18 NURI
													// recvBuffer에 특정 str이 존재하는지 찾아서
													// m_iRecvStartPtr로부터의 offset을 리턴
													// 해당 str이 존재하지 않는 경우 -1을 리턴

	//	아래 함수에 버그 존재, 사용하는 곳은 없음
	//bool	AppendReadBuffer(const char *pStr, int *pnappend);	// 2005.1.5 NURI
	//															// pStr을 ReadBuffer에 append
	//															// ReadBuffer의 남은 공간이 부족하면
	//															// 가능한 만큼 append하고 false를 리턴

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
