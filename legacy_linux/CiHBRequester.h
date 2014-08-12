// CiHBRequester.h: interface for the CCiHBRequester class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CIHBREQUESTER_H__EC8297B3_2463_473C_B9DD_576718830D52__INCLUDED_)
#define AFX_CIHBREQUESTER_H__EC8297B3_2463_473C_B9DD_576718830D52__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CiHBAddressList.h"

/* �ϳ��� UDP ������ ����Ͽ� ��� (ip:port) ������ request�� ������.
 * (ip:port) ���� m_HBAddressList�� ����Ǿ� �ִ�.
 * socket�� read event�� �߻��ϸ� ���� request�� ���� response�̸� response�� ���� �ּҰ� m_saRecv�� ���õȴ�.
 * ReceiveIntMessage()���� m_saRecv�� ���õǰ� OnMessage()���� �� ���� �ٷ� ���ȴ�.
 * ��, OnMessage()���� ����ϴ� m_saRecv ���� ReceiveIntMessage()���� ���õ� ���̴�.
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
		/* MY_HW_ERROR�� MY_SW_ERROR�� �Ǵ��ϱ� ���� IP��
		 * HB_RESPONSE�� �Ѿ���� LocalIP�� ���Ѵ�. */

	struct sockaddr m_saRecv;				/* address of response machine */
	char m_szRecvBuf[CIHB_MAX_DATA_SIZE];

	int m_iRequestCount;
	CMTime2 m_mtRequestTime;				/* timestamp of sending request */
	CMTime2 m_mtRecvTime;					/* timestamp of receiving response */

	bool m_bLogPrint;					/* full log message */
};

#endif // !defined(AFX_CIHBREQUESTER_H__EC8297B3_2463_473C_B9DD_576718830D52__INCLUDED_)
