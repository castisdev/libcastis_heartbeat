// StringLinkedList.h: interface for the CStringLinkedList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STRINGLINKEDLIST_H__CFA99E7D_4B75_411B_B34E_6B83E72AE791__INCLUDED_)
#define AFX_STRINGLINKEDLIST_H__CFA99E7D_4B75_411B_B34E_6B83E72AE791__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>

#include "LinkedListTemplate.h"

class CStringLinkedList : public CLinkedListTemplate<std::string>
{
public:
	CStringLinkedList();
	virtual ~CStringLinkedList();

public:
	virtual bool IsIDEqual(unsigned int uiID1, unsigned int uiID2);

	/* string list management */
	std::string *FindString(std::string *pString);

	std::string *SafeFindString(std::string *pString);
};

#endif // !defined(AFX_STRINGLINKEDLIST_H__CFA99E7D_4B75_411B_B34E_6B83E72AE791__INCLUDED_)
