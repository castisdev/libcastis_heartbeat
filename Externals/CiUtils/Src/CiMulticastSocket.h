// MulticastSocket.h: interface for the CCiMulticastSocket class.
//
//////////////////////////////////////////////////////////////////////
#ifndef _MULTICAST_SOCKET_H__
#define _MULTICAST_SOCKET_H__

#ifndef _WIN32
#include <netinet/in.h>
#endif

class CCiMulticastSocket
{
public:
	CCiMulticastSocket();
	virtual ~CCiMulticastSocket();

public:

	bool CreateBlockingReceiveSocket( const char *groupIP, unsigned short groupPort );
	bool StopReceiving();

	bool CreateBlockingSendSocket( const char *groupIP, unsigned short groupPort, const char *SourceInterfaceIP, unsigned int nTTL, bool bLoopBack );
	bool StopSending();

	bool CreateNonBlockingSendSocket( const char *groupIP, unsigned short groupPort, const char *SourceInterfaceIP, unsigned int nTTL, bool bLoopBack );
	bool CreateNonBlockingReceiveSocket( const char *groupIP, unsigned short groupPort );

	void SetLoopBack(bool loopback);
	bool SetTTL(unsigned int nTTL);
	bool SetPriority( int priority );

	bool SendInt32(unsigned int value);
	bool SendN(const void* buf, int size);

// buffering
	unsigned char m_sendBuffer[300*1024];
	unsigned int m_iSendBufferPosition;
	bool ClearSendBuffer();
	bool SendInt32ToSendBuffer(unsigned int value);
	bool SendNToSendBuffer(const void* buf, int size);
	bool FlushSendBuffer();

	unsigned char m_recvBuffer[300*1024];
	unsigned int m_iRecvBufferPosition;
	unsigned int m_iNBytesReceived;
	bool FillRecvBufferFromNetwork();
	bool ReceiveInt32FromRecvBuffer(int &iReceivedIntMessage);
	bool ReceiveNFromRecvBuffer(char *recvBuffer, int nBytesToReceive, int *nReceivedBytes);
	bool ClearRecvBuffer();

	bool ReceiveN( char *recvBuffer, int nBytesToReceive, int *nReceivedBytes );
	bool ReceiveInt32( int &iReceivedInt32Message );

// Implementation
public:
	bool SetSendSocketBufferSize( int nBufferSize );
	bool SetReceiveSocketBufferSize( int nBufferSize );
	bool SetTimeOutForReceiveSocket( unsigned int uiSec, unsigned int uiUsec );

	bool ReceiveNWithTimeOut( char *pReceiveBuffer, int iNBytesToReceive, int *piNReceivedDateBytes, unsigned int uiSec );
	bool SendNWithTimeOut( unsigned int uiSec );
	bool SetReceiveSocketNonBlocking();
	bool SetSendSocketNonBlocking();

	struct sockaddr_in m_saHostGroup;	// SOCKADDR structure to hold IP/Port of the Host group to send data to it
	struct sockaddr_in m_saSourceAddress;	// SOCKADDR structure to hold IP/Port of My own.

	struct ip_mreq m_mrMReq;			// Contains IP and interface of the host group
	struct in_addr m_MyInterfaceAddress;

	char	m_strSendersIP[16];		// Hold IP of the socket from which the last packet was received
	char	m_strLocalIP[16];		// IP Address of the local host or your machine
	unsigned int m_uiSendersPort;		// Holds Port No. of the socket from which last packet was received
	unsigned int m_uiLocalPort;			// Ephemeral port number of the sending port

	bool	m_bForceNoLoopback;		// If interface does not support lopback and the service is required, the bool is set to true
	int		m_iReceiveSocket;
	int		m_iSendSocket;	// Socket for sending data to the host group
};


#endif
