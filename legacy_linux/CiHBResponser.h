// CiHBResponser.h: interface for the CCiHBResponser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CIHBRESPONSER_H__7681D488_0E1B_4F5D_A536_170AF89EE34F__INCLUDED_)
#define AFX_CIHBRESPONSER_H__7681D488_0E1B_4F5D_A536_170AF89EE34F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CCiHBResponser : public CNetworkThread
{
public:
	CCiHBResponser( const char *szRepresentativeIP,
					unsigned short iPortNumber,
					const char *szLocalIP,
					CiThread2Handle_t complexThreadHandle=CI_THREAD2_INVALID_THREAD_HANDLE );
	virtual ~CCiHBResponser();

	bool InitInstance();
	bool ExitInstance();

	ReceiveIntMessageResult_t ReceiveIntMessage(CCiSocket *pReadSocket, int *piReceivedMessage);
	bool OnMessage(CCiSocket *pReadSocket, int iMessage);

	bool OnHeartbeatRequest(CCiSocket *pReadSocket);

	bool OnReadSocketError(CCiSocket *pReadSocket);

	CiHBState_t GetProcessState();

	void SetProcessHWError();
	void SetProcessSWError();
	void SetProcessAlive();

protected:
	bool SendHeartbeatResponse(CCiSocket *pReadSocket, int iSeqNum);

	void SetProcessState(CiHBState_t state);

public:
	CCiSocket *m_pSocket;
	char m_szRepresentativeIP[CI_MAX_IP_ADDRESS_LENGTH+1];
	char m_szLocalIP[CI_MAX_IP_ADDRESS_LENGTH+1];
	unsigned short m_iPortNumber;

protected:
	CiHBState_t	m_processState;				/* ALIVE or SW_ERROR or HW_ERROR */

	struct sockaddr m_saRecv;				/* address of request machine */
	char m_szRecvBuf[CIHB_MAX_DATA_SIZE];

	CMTime2 m_mtRecvTime;					/* timestamp */
};

#endif // !defined(AFX_CIHBRESPONSER_H__7681D488_0E1B_4F5D_A536_170AF89EE34F__INCLUDED_)

