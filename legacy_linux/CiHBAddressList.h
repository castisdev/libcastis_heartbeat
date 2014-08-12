// CiHBAddressList.h: interface for the CCiHBAddressList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CIHBADDRESSLIST_H__DC202860_4C14_4581_80E7_9CFE3276C2AB__INCLUDED_)
#define AFX_CIHBADDRESSLIST_H__DC202860_4C14_4581_80E7_9CFE3276C2AB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CiHBAddress.h"

class CCiHBAddressList : public CLinkedListTemplate<CCiHBAddress>
{
public:
	CCiHBAddressList();
	virtual ~CCiHBAddressList();

public:
	/* implementation */
	virtual unsigned long GetIDOf(CCiHBAddress *pHBAddress);

	/* list management */
	CCiHBAddress *FindHBAddressFromID(int iID);
	CCiHBAddress *SafeFindHBAddressFromID(int iID);
};

#endif // !defined(AFX_CIHBADDRESSLIST_H__DC202860_4C14_4581_80E7_9CFE3276C2AB__INCLUDED_)
