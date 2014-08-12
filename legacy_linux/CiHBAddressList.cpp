// CiHBAddressList.cpp: implementation of the CCiHBAddressList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CiHBAddressList.h"
#include "CiHBAddress.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCiHBAddressList::CCiHBAddressList()
{

}

CCiHBAddressList::~CCiHBAddressList()
{

}

unsigned long CCiHBAddressList::GetIDOf(CCiHBAddress *pHBAddress)
{
	return (unsigned long)pHBAddress->m_iID;
}

CCiHBAddress *CCiHBAddressList::FindHBAddressFromID(int iID)
{
	return (CCiHBAddress *)FindItemFromID((unsigned int)iID);
}

CCiHBAddress *CCiHBAddressList::SafeFindHBAddressFromID(int iID)
{
	if ( Lock() == false ) {
		CiUtils_PrintWithTime(stderr, "CCiHBAddressList::SafeFindHBAddressFromID :: Lock Fail");
		return NULL;
	}

	CCiHBAddress *pHBAddress = FindHBAddressFromID(iID);

	if ( Unlock() == false ) {
		CiUtils_PrintWithTime(stderr, "CCiHBAddressList::SafeFindHBAddressFromID :: Unlock Fail");
		return NULL;
	}

	return pHBAddress;
}
