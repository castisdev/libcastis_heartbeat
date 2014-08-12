// CiHBAddress.h: interface for the CCiHBAddress class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CIHBADDRESS_H__70397FF7_8B8D_4D94_8517_B99E3F46F9DE__INCLUDED_)
#define AFX_CIHBADDRESS_H__70397FF7_8B8D_4D94_8517_B99E3F46F9DE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CCiHBAddress
{
public:
	CCiHBAddress(const char *pszHeartbeatIP, unsigned short usHeartbeatPort);
	virtual ~CCiHBAddress();

	void ResetState()
	{
		m_state = CIHB_REQUEST_STATE_INIT;
	};

public:
	static int m_iIDBase;
	int m_iID;

	char m_szHeartbeatIP[CI_MAX_IP_ADDRESS_LENGTH+1];
	unsigned short m_usHeartbeatPort;
	CCiHBRequestState_t m_state;
};

#endif // !defined(AFX_CIHBADDRESS_H__70397FF7_8B8D_4D94_8517_B99E3F46F9DE__INCLUDED_)
