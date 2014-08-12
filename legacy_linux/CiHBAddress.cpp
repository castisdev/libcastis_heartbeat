// CiHBAddress.cpp: implementation of the CCiHBAddress class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CiHBAddress.h"

int CCiHBAddress::m_iIDBase = 0;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCiHBAddress::CCiHBAddress(const char *pszHeartbeatIP, unsigned short usHeartbeatPort)
{
	/* ID */
	m_iID = m_iIDBase++;
	if ( m_iID == 0x7FFFFFFF ) {
		m_iIDBase = 0;
	}

	int iNTriedToCreate;
	CiStrCpy(m_szHeartbeatIP, pszHeartbeatIP, CI_MAX_IP_ADDRESS_LENGTH, &iNTriedToCreate);
	m_usHeartbeatPort = usHeartbeatPort;
	m_state = CIHB_REQUEST_STATE_INIT;
}

CCiHBAddress::~CCiHBAddress()
{

}
