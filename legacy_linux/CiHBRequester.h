// CiHBRequester.h: interface for the CCiHBRequester class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CIHBREQUESTER_H__EC8297B3_2463_473C_B9DD_576718830D52__INCLUDED_)
#define AFX_CIHBREQUESTER_H__EC8297B3_2463_473C_B9DD_576718830D52__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CiHBAddressList.h"

/* 하나의 UDP 소켓을 사용하여 모든 (ip:port) 쌍으로 request를 보낸다.
 * (ip:port) 쌍은 m_HBAddressList에 저장되어 있다.
 * socket에 read event가 발생하면 보낸 request에 대한 response이며 response를 보낸 주소가 m_saRecv에 세팅된다.
 * ReceiveIntMessage()에서 m_saRecv이 세팅되고 OnMessage()에서 이 값이 바로 사용된다.
 * 즉, OnMessage()에서 사용하는 m_saRecv 값은 ReceiveIntMessage()에서 세팅된 값이다.
 */
class CCiHBRequester : public CNetworkThread
{
public:
	CCiHBRequester(const char *pszServiceName,
				   const char *pszMyIPAddress,
				   CiThread2Handle_t complexThreadHandle=CI_THREAD2_INVALID_THREAD_HANDLE,
				   bool bLogPrint=false);
	virtual ~CCiHBRequester();

	virtual bool InitInstance();
	virtual bool ExitInstance();

	virtual ReceiveIntMessageResult_t ReceiveIntMessage(CCiSocket *pReadSocket, int *piReceivedMessage);
	virtual bool OnMessage(CCiSocket *pReadSocket, int iMessage);

	bool OnHeartbeatResponse();

	virtual bool OnReadSocketError(CCiSocket *pReadSocket);

	bool SendRequest();

	bool AddHBAddress(const char *pszIPAddress, unsigned short usPortNumber);

	bool IsAllResponseArrived();
	bool IsResponseMyHWError();
	bool IsResponseMySWError();
	void ResetAllResponseArrived();

	void ResetRequestTime();
	bool IsRequestTimeOut(int iTimeOutMSec);

	void ResetRequestCount();
	int GetRequestCount();
	void IncreaseRequestCount();

protected:
	bool ParseHeartbeatResponse();

public:
	int m_seqNum;

	CCiSocket *m_pSocket;
	CCiHBAddressList m_HBAddressList;

protected:
	char m_szServiceName[CI_MAX_NAME_LENGTH+1];
	char m_szMyIPAddress[CI_MAX_IP_ADDRESS_LENGTH+1];
		/* MY_HW_ERROR와 MY_SW_ERROR를 판단하기 위한 IP로
		 * HB_RESPONSE로 넘어오는 LocalIP와 비교한다. */

	struct sockaddr m_saRecv;				/* address of response machine */
	char m_szRecvBuf[CIHB_MAX_DATA_SIZE];

	int m_iRequestCount;
	CMTime2 m_mtRequestTime;				/* timestamp of sending request */
	CMTime2 m_mtRecvTime;					/* timestamp of receiving response */

	bool m_bLogPrint;					/* full log message */
};

#endif // !defined(AFX_CIHBREQUESTER_H__EC8297B3_2463_473C_B9DD_576718830D52__INCLUDED_)
