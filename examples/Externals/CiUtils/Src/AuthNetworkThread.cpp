// AuthNetworkThread.cpp: implementation of the CAuthNetworkThread class.
//
//////////////////////////////////////////////////////////////////////

#include "internal_CiUtils.h"
#include "AuthNetworkThread.h"
#include "CiSocket.h"

#include "CiLogger.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAuthNetworkThread::CAuthNetworkThread(int iListenPortNumber
									   , int iAuthCode
									   , int iConnectedSocketSendBufferSize/*=CI_SOCKET_SEND_BUFFER_SIZE*/
									   , int iConnectedSocketRecvBufferSize/*=CI_SOCKET_RECV_BUFFER_SIZE*/)
: CNetworkThread(iListenPortNumber, iConnectedSocketSendBufferSize, iConnectedSocketRecvBufferSize)
{
	m_iAuthCode = iAuthCode;
}

CAuthNetworkThread::~CAuthNetworkThread()
{

}

// Default 3초 안에.. int값이 넘어오지 않으면 FALSE 처리... -> 최악의 경우.. 3초 동안 Block 문제점..
bool CAuthNetworkThread::ProcessWithConnectedSocket(CCiSocket* socket)
{
	int iAuthCode = 0;

	if ( socket->ReadInt32WithTimeOut( &iAuthCode ) != true ||	iAuthCode != m_iAuthCode )
	{
		return false;
	}

	return true;
}
