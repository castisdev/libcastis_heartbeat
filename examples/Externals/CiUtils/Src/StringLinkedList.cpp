// StringLinkedList.cpp: implementation of the CStringLinkedList class.
//
//////////////////////////////////////////////////////////////////////

#include "internal_CiUtils.h"
#include "CiSafeString.h"
#include "StringLinkedList.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace std;

CStringLinkedList::CStringLinkedList()
{

}

CStringLinkedList::~CStringLinkedList()
{
}

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4311 4312)
#endif

bool CStringLinkedList::IsIDEqual(unsigned int uiID1, unsigned int uiID2)
{
	string *pString1 = (string *)uiID1;
	string *pString2 = (string *)uiID2;

	return (pString1->compare(*pString2) == 0);
}

#ifdef _WIN32
#pragma warning(pop)
#endif

string *CStringLinkedList::FindString(string *pString)
{
	return FindItemFromID(GetIDOf(pString));
}

string *CStringLinkedList::SafeFindString(string *pString)
{
	if ( Lock() == false ) {
		return NULL;
	}

	string *pStringResult = FindString(pString);

	if ( Unlock() == false ) {
		return NULL;
	}

	return pStringResult;
}
