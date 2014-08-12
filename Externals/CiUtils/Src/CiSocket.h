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
				 * Ư�� pattern�� ���� ������ ��Ʈ������ �����͸� �д´�.
				 * readsize��ŭ�� recv�õ��� �ϸ�, ������ �ð� �ȿ� ���� ���ϸ� false�� ��ȯ
				 */

	/* 2003.09.22 by nuri */
	bool	GetBufferClon(CCiSocket &rhs);

	////////////////////////////////////////////////////////////////////
	// Server Method
	//
	// Buffer���� �а� ����.
	bool	WriteInt16(short sInt16);
	bool	WriteInt32(int iInt32);
	bool	WriteInt64(long long llInt64);
	bool	WriteN(const void* pBuf, int ntowrite, int* pnwritten);
		// 2004.04.19 by NURI
		// ������ buffer�� �̿��� WriteN() ������ �����Ͽ���.
		// non-blocking socket�� ��쿡�� �������� �ϴ� �����͸� ��� �������� �õ��� �Ŀ� ����.
		// false�� �����ϴ� ���, errno�� �������� üũ�� �� ������
		// true�� �����ϴ� ��쿡�� *pnwritten���� ntowrite���� ������ üũ ���.
	bool	WriteNFlush(const void* pBuf, int ntowrite, int* pnwritten);

	bool	ReadInt16(short* psInt16);
	bool	ReadInt32(int* piInt32);
	bool	ReadInt64(long long* pllInt64);
	bool	ReadN(void* pBuf, int ntoread, int* pnread);

	bool	LookAheadInt32(int* piInt32, int iOffset);	// m_iRecvStartPtr���� offset��ŭ �������� 4byte�� �а��� �ϴ°��( Buffer���� �� ���� )
	bool	LookAheadN(void* pBuf, int ntoReadSize, int iOffset);	// m_iRecvStartPtr���� offset��ŭ �������� ntoReadSize�� �а��� �ϴ°��( Buffer���� �� ���� )

	bool	ReadFromNetworkToBuffer();		// recv �Ͽ� buffer�� ����..

	// by gun 2004.12.13
	// parameter�� ��Ʈ������ ���� MAX���� �Ѱ� �޴� �Լ� �߰�
	bool	ReadFromNetworkToBuffer(int iReadSize);

	int		GetCurrentRecvDataSize();

	bool	WriteFromBufferToNetwork();		// buffer�� �ִ� ���� send�Ѵ�.
	int		GetRemainSendDataSize();

	int		StrExistInReadBuffer(const char *pStr);	// 2004.11.18 NURI
													// recvBuffer�� Ư�� str�� �����ϴ��� ã�Ƽ�
													// m_iRecvStartPtr�κ����� offset�� ����
													// �ش� str�� �������� �ʴ� ��� -1�� ����

	//	�Ʒ� �Լ��� ���� ����, ����ϴ� ���� ����
	//bool	AppendReadBuffer(const char *pStr, int *pnappend);	// 2005.1.5 NURI
	//															// pStr�� ReadBuffer�� append
	//															// ReadBuffer�� ���� ������ �����ϸ�
	//															// ������ ��ŭ append�ϰ� false�� ����

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
