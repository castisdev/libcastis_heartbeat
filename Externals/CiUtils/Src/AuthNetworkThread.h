// AuthNetworkThread.h: interface for the CAuthNetworkThread class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AUTHNETWORKTHREAD_H__BF20F021_577E_4338_BC93_FF847C14A243__INCLUDED_)
#define AFX_AUTHNETWORKTHREAD_H__BF20F021_577E_4338_BC93_FF847C14A243__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NetworkThread.h"

#define CI_NETWORK_CONNECT_CODE		221330

class CAuthNetworkThread : public CNetworkThread
{
public:
	CAuthNetworkThread(int iListenPortNumber
		, int iAuthCode = CI_NETWORK_CONNECT_CODE
		, int iConnectedSocketSendBufferSize = CI_SOCKET_SEND_BUFFER_SIZE
		, int iConnectedSocketRecvBufferSize = CI_SOCKET_RECV_BUFFER_SIZE);

	virtual ~CAuthNetworkThread();

    virtual bool ProcessWithConnectedSocket(CCiSocket* socket);

protected:
	int m_iAuthCode;
};

#endif // !defined(AFX_AUTHNETWORKTHREAD_H__BF20F021_577E_4338_BC93_FF847C14A243__INCLUDED_)
