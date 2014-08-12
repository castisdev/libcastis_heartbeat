// LinkedListTemplate.h: interface for the CLinkedListTemplate class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LINKEDLISTTEMPLATE_H__27C93B59_3081_4D1B_AF4E_17916F41B2A4__INCLUDED_)
#define AFX_LINKEDLISTTEMPLATE_H__27C93B59_3081_4D1B_AF4E_17916F41B2A4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CiMutex.h"

#define INVALID_LINKED_LIST_INDEX	(-1)
#define INVALID_SAFE_LINKED_LIST_INDEX	(-1)

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4311 4312)
#endif

class CLinkItem
{
public:
	CLinkItem() : m_pPrevious(NULL), m_pNext(NULL), m_pItem(NULL) {};
	virtual ~CLinkItem() {};

public:
	CLinkItem *m_pPrevious;
	CLinkItem *m_pNext;
	void *m_pItem;
};

template <class LINKITEM>
class CLinkItemQueueTemplate
{
public:
	CLinkItemQueueTemplate()
	{
		m_pHead = m_pTail = NULL;
		m_iNItems = 0;
	};

	virtual ~CLinkItemQueueTemplate()
	{
		LINKITEM *pLinkItem = m_pHead;
		while (pLinkItem) {
			LINKITEM *pCurrent = pLinkItem;
			pLinkItem = (LINKITEM *)pCurrent->m_pNext;
			delete pCurrent;
        }
	};

	inline int SafeGetNItems()
	{
		if ( _mutex.Lock() == false ) {
			//fprintf(stderr, "CLinkItemQueueTemplate::SafeGetNItems::_mutex.Lock() FAIL\n");
			return -1;
		}

		int iNItems = m_iNItems;

		if ( _mutex.UnLock() == false ) {
			//fprintf(stderr, "CLinkItemQueueTemplate::SafeGetNItems::_mutex.UnLock() FAIL\n");
			return -1;
		}

		return iNItems;
	};

	inline bool SafeEnqueue(LINKITEM *pLinkItem)
	{
		if ( pLinkItem == NULL ) {
			//fprintf(stderr, "CLinkItemQueueTemplate::SafeEnqueue::pLinkItem is NULL\n");
			return false;
		}

		if ( _mutex.Lock() == false ) {
			//fprintf(stderr, "CLinkItemQueueTemplate::SafeEnqueue::_mutex.Lock() FAIL\n");
			return false;
		}

		if ( m_pHead == NULL && m_pTail == NULL ) {
			m_pHead = m_pTail = pLinkItem;
			pLinkItem->m_pPrevious = NULL;
			pLinkItem->m_pNext = NULL;
		}
		else {
			m_pTail->m_pNext = pLinkItem;
			pLinkItem->m_pPrevious = m_pTail;
			pLinkItem->m_pNext = NULL;
			m_pTail = pLinkItem;
		}

		m_iNItems++;

		if ( _mutex.UnLock() == false ) {
			//fprintf(stderr, "CLinkItemQueueTemplate::SafeEnqueue::_mutex.UnLock() FAIL\n");
			return false;
		}

		return true;
	};

	inline bool SafeDequeue(LINKITEM **ppLinkItem)
	{
		if ( ppLinkItem == NULL ) {
			//fprintf(stderr, "CLinkItemQueueTemplate::SafeDequeue::ppLinkItem is NULL\n");
			return false;
		}

		if ( _mutex.Lock() == false ) {
			//fprintf(stderr, "CLinkItemQueueTemplate::SafeDequeue::_mutex.Lock() FAIL\n");
			return false;
		}

		if ( m_pHead == NULL ) {
			*ppLinkItem = NULL;

			if ( _mutex.UnLock() == false ) {
				//fprintf(stderr, "CLinkItemQueueTemplate::SafeDequeue::m_pHead is NULL and _mutex.UnLock() FAIL\n");
				return false;
			}
			return false;
		}

		*ppLinkItem = m_pHead;

		if ( m_pHead == m_pTail ) {
			/* only one item */
			m_pHead = m_pTail = NULL;
		}
		else {
			/* delete the head */
			m_pHead = (LINKITEM *)m_pHead->m_pNext;
			m_pHead->m_pPrevious = NULL;
		}

		m_iNItems--;

		if ( _mutex.UnLock() == false ) {
			//fprintf(stderr, "CLinkItemQueueTemplate::SafeDequeue::_mutex.UnLock FAIL\n");
			return false;
		}

		return true;
	};

	/* these are raw interfaces for the CLinkItemQueueTemplate */
	/* You are responsible for ensuring the safy of the queue */
	/* These raw interfaces can be used for gaining more performance */
	/* These member functions are declared as inline functions */
	inline bool Lock()
	{
		return _mutex.Lock();
	};

	inline bool Unlock()
	{
		return _mutex.UnLock();
	};

	inline int RawGetNItems()
	{
		return m_iNItems;
	};

	inline void RawSetNItems(int iNItems)
	{
		m_iNItems = iNItems;
	};

	/* KSH you should not RawReleaseLinkItem in multi-threaded program */
	inline bool RawEnqueue(LINKITEM *pLinkItem)
	{
		if ( m_pHead == NULL && m_pTail == NULL ) {
			m_pHead = m_pTail = pLinkItem;
			pLinkItem->m_pPrevious = NULL;
			pLinkItem->m_pNext = NULL;
		}
		else {
			m_pTail->m_pNext = pLinkItem;
			pLinkItem->m_pPrevious = m_pTail;
			pLinkItem->m_pNext = NULL;
			m_pTail = pLinkItem;
		}

		return true;
	};

	/* KSH RawAllocLinkItem can only be used in single consumer program with danger */
	inline bool RawDequeue(LINKITEM **ppLinkItem)
	{
		if ( m_pHead == NULL ) {
			*ppLinkItem = NULL;
			return false;
		}

		*ppLinkItem = m_pHead;

		if ( m_pHead == m_pTail ) {
			/* only one item */
			m_pHead = m_pTail = NULL;
		}
		else {
			/* delete the head */
			m_pHead = (LINKITEM *)m_pHead->m_pNext;
			m_pHead->m_pPrevious = NULL;
		}

		return true;
	};

	inline bool RawLookAhead(LINKITEM **ppLinkItem)
	{
		if ( m_pHead == NULL ) {
			*ppLinkItem = NULL;
			//fprintf(stderr, "CLinkItemQueueTemplate::RawLookAhead::m_pHead is NULL\n");
			return false;
		}

		*ppLinkItem = m_pHead;

		return true;
	};

protected:
	CCiMutex _mutex;

	LINKITEM *m_pHead;
	LINKITEM *m_pTail;

	int m_iNItems;
};

template <class LINKITEM>
class CLinkItemCacheTemplate : public CLinkItemQueueTemplate<LINKITEM>
{
public:
	CLinkItemCacheTemplate(int iCacheSize=100)
	{
		m_iCacheSize = iCacheSize;
	};

	virtual ~CLinkItemCacheTemplate()
	{
    };

	void SafeReleaseLinkItem(LINKITEM *pLinkItem)
	{
		CLinkItemQueueTemplate<LINKITEM>::Lock();

		if (pLinkItem == NULL) {
			CLinkItemQueueTemplate<LINKITEM>::Unlock();
			return;
		}

		if ( CLinkItemQueueTemplate<LINKITEM>::m_iNItems >= m_iCacheSize ) {
			delete pLinkItem;
		}
		else {
			CLinkItemQueueTemplate<LINKITEM>::RawEnqueue(pLinkItem);
			CLinkItemQueueTemplate<LINKITEM>::m_iNItems++;
		}

		CLinkItemQueueTemplate<LINKITEM>::Unlock();
	};

	LINKITEM *SafeAllocLinkItem()
	{
		CLinkItemQueueTemplate<LINKITEM>::Lock();

		LINKITEM *pLinkItem;
		CLinkItemQueueTemplate<LINKITEM>::RawDequeue(&pLinkItem);

		if (pLinkItem == NULL) {
			pLinkItem = new LINKITEM;
		} else {
			CLinkItemQueueTemplate<LINKITEM>::m_iNItems--;
		}

		CLinkItemQueueTemplate<LINKITEM>::Unlock();

        return pLinkItem;
    };

	/* KSH do not use RawReleaseLinkItem in multi-threaded program */
	void RawReleaseLinkItem(LINKITEM *pLinkItem)
	{
		if ( CLinkItemQueueTemplate<LINKITEM>::m_iNItems >= m_iCacheSize ) {
			delete pLinkItem;
		}
		else {
			CLinkItemQueueTemplate<LINKITEM>::RawEnqueue(pLinkItem);
		}
	};

	/* KSH RawAllocLinkItem can only be used in single consumer program with danger */
	LINKITEM *RawAllocLinkItem()
	{
		LINKITEM *pLinkItem;
		CLinkItemQueueTemplate<LINKITEM>::RawDequeue(&pLinkItem);

		if (pLinkItem == NULL) {
			pLinkItem = new LINKITEM;
		}

		return pLinkItem;
    };

protected:
	int m_iCacheSize;
};

typedef CLinkItemCacheTemplate<CLinkItem> CLinkItemCache;

template <class OBJECT>
class CLinkedListTemplate
{
public:
	CLinkedListTemplate(int iCacheSize=100) : m_linkItemCache(iCacheSize)
	{
		m_pHead = m_pTail = NULL;
		m_iNItems = 0;
	};

	virtual ~CLinkedListTemplate()
	{
		Clean();
	};

private:
	CLinkItemCache m_linkItemCache;

	CCiMutex _mutex;		// for thread-safe

	CLinkItem *m_pHead;
	CLinkItem *m_pTail;

	int m_iNItems;

	/* add at the end of the link, tail */
	bool Link(CLinkItem *pLinkItem)
	{
		if ( pLinkItem == NULL ) {
			//fprintf(stderr, "CLinkedListTemplate::Link::pLinkItem is NULL\n");
			return false;
		}

		if ( m_pHead == NULL && m_pTail == NULL ) {
			pLinkItem->m_pPrevious = pLinkItem->m_pNext = NULL;
			m_pHead = m_pTail = pLinkItem;
		}
		else {
			m_pTail->m_pNext = pLinkItem;
			pLinkItem->m_pPrevious = m_pTail;
			pLinkItem->m_pNext = NULL;
			m_pTail = pLinkItem;
		}

		m_iNItems++;

		return true;
	};

	/* remove at the beginning of the list, head */
	bool Unlink(CLinkItem *pLinkItem)
	{
		if ( pLinkItem == NULL ) {
			//fprintf(stderr, "CLinkedListTemplate::Unlink::pLinkItem is NULL\n");
			return false;
		}

		if ( m_pHead == NULL || m_pTail == NULL ) {
			//fprintf(stderr, "CLinkedListTemplate::Unlink::m_pHead or m_pTail is NULL\n");
			return false;
		}

		if ( m_pHead == m_pTail ) {
			// sinma - 2006. 01. 05
			// 만일 pLinkItem 이 m_pHead 혹은 m_pTail 과 다른 경우라면 문제가 있다.
			/* only one item */
			if ( m_pHead == pLinkItem ) {
				m_pHead = m_pTail = NULL;
			}
			else {
				//fprintf(stderr, "CLinkedListTemplate::Unlink::invalid pLinkItem\n");
				return false;
			}
		}
		else if ( pLinkItem == m_pHead ) {
			/* delete the head */
			CLinkItem *pSecond = m_pHead->m_pNext;

			m_pHead->m_pNext = NULL;
			pSecond->m_pPrevious = NULL;
			m_pHead = pSecond;
		}
		else if ( pLinkItem == m_pTail ) {
			/* delete the tail */
			CLinkItem *pSecondLast = m_pTail->m_pPrevious;

			m_pTail->m_pPrevious = NULL;
			pSecondLast->m_pNext = NULL;
			m_pTail = pSecondLast;
		}
		else {
			pLinkItem->m_pPrevious->m_pNext = pLinkItem->m_pNext;
			pLinkItem->m_pNext->m_pPrevious = pLinkItem->m_pPrevious;
		}

		/* should delete the indirect index even if do not delete pItem */
		m_linkItemCache.SafeReleaseLinkItem(pLinkItem);

		m_iNItems--;

		return true;
	};

	/* find a item with ID */
	CLinkItem *FindLinkItemFromID(unsigned long ulID)
	{
		CLinkItem *pLinkItem = m_pHead;
		while ( pLinkItem != NULL ) {
			if ( IsIDEqual(GetIDOf((OBJECT *)pLinkItem->m_pItem), ulID) == true ) {
				break;
			}
			pLinkItem = pLinkItem->m_pNext;
		}

		return pLinkItem;
	}

public:
	/* locking facilities */
	bool Lock()
	{
		return _mutex.Lock();
	};

	bool Unlock()
	{
		return _mutex.UnLock();
	};

	/* find the position */
	int FindIndex(OBJECT *pItem)
	{
		CLinkItem *pLinkItem = m_pHead;
		int iIndex = 0;

		while ( pLinkItem != NULL ) {
			if ( (OBJECT *)pLinkItem->m_pItem == pItem ) {
				break;
			}
			pLinkItem = pLinkItem->m_pNext;
			iIndex++;
		}

		if ( pLinkItem == NULL ) {
			iIndex = INVALID_LINKED_LIST_INDEX;
		}

		return iIndex;
	};

	/* find a item with ID */
	OBJECT *FindItemFromID(unsigned long ulID)
	{
		CLinkItem *pLinkItem = FindLinkItemFromID(ulID);
		if ( pLinkItem == NULL ) {
			return NULL;
		}

		return (OBJECT *)pLinkItem->m_pItem;
	}

	// by sasgas 2003.8.5
	OBJECT *GetItem(int iIndex)
	{
		if ( iIndex >= m_iNItems ) {
			//fprintf(stderr, "CLinkedListTemplate::GetItem::index is greater than NItems\n");
			return NULL;
		}

		int i;
		OBJECT *pItem;
		CLinkItem *pLinkItem = m_pHead;

		for ( i = 0; i < iIndex; i++ ) {
			if ( pLinkItem == NULL ) {
				pItem = NULL;
				break;
			}
			pLinkItem = pLinkItem->m_pNext;
		}

		if ( i == iIndex ) {
			pItem = (OBJECT *)pLinkItem->m_pItem;
		}

		return pItem;
	};

	/* the number of items */
	int GetNItems() const
	{
		return m_iNItems;
	};

	/* Add, Delete, Modify */
	bool AddItem(OBJECT *pItem)
	{
		/* We will not add the item if it exists already */
		CLinkItem *pLinkItem = FindLinkItemFromID(GetIDOf(pItem));
		if ( pLinkItem != NULL ) {
			/* already exists */
			//fprintf(stderr, "CLinkedListTemplate::AddItem::Item is already exists\n");
			return false;
		}

		////KSH use available list for performance
		////new CLinkItem;
		CLinkItem *pNewLinkItem = m_linkItemCache.SafeAllocLinkItem();
		if ( pNewLinkItem == NULL ) {
			//fprintf(stderr, "CLinkedListTemplate::AddItem::Link Item allocation fail\n");
			return false;
		}
		pNewLinkItem->m_pItem = (void *)pItem;

		/* Link it now */
		return Link(pNewLinkItem);
	};

	bool DeleteItem(OBJECT *pItem, bool bPreserve=false)
	{
		/* search first */
		CLinkItem *pLinkItem = FindLinkItemFromID(GetIDOf(pItem));
		if ( pLinkItem == NULL ) {
			/* If it does not exist. */
			//fprintf(stderr, "CLinkedListTemplate::DeleteItem::Item is not found\n");
			return false;
		}

		/* Unlink it. */
		bool bResult = Unlink(pLinkItem);
		if ( bResult == true && !bPreserve ) {
			/* and destroy if necessary */
			delete (OBJECT *)pItem;
		}

		return bResult;
	};

	// sinma - 2006. 01. 05
	// TRAVERSE_LIST 를 통해서 삭제하려는 pItem의 position을 이미 얻어낸 경우
	// 다시 한번 FindLinkItemFromID 을 통해 TRAVERSE_LIST 할 필요가 없다.
	// pItem이 LIST의 Item인지 체크하는 루틴이 없으므로 사용시 주의해야 한다!
	bool DeleteItem(OBJECT *pItem, unsigned long ulPosition, bool bPreserve=false)
	{
		/* search first */
		CLinkItem *pLinkItem = (CLinkItem*)ulPosition;
		if ( pLinkItem == NULL ) {
			/* If it does not exist. */
			//fprintf(stderr, "CLinkedListTemplate::DeleteItem::Item is not found\n");
			return false;
		}

		/* Unlink it. */
		bool bResult = Unlink(pLinkItem);
		if ( bResult == true && !bPreserve ) {
			/* and destroy if necessary */
			delete (OBJECT *)pItem;
		}

		return bResult;
	};

	bool ModifyItem(OBJECT *pItemBefore, OBJECT *pItemAfter, bool bPreserve=false)
	{
		/* search first */
		CLinkItem *pLinkItem = FindLinkItemFromID(GetIDOf(pItemBefore));
		if ( pLinkItem == NULL ) {
			/* If it does not exist */
			//fprintf(stderr, "CLinkedListTemplate::ModifyItem::Item is not found\n");
			return false;
		}

		/* just change item */
		pLinkItem->m_pItem = (void *)pItemAfter;
		if ( !bPreserve ) {
			/* and destroy if necessary */
			delete pItemBefore;
		}

		return true;
	};

	/* detroy all */
	bool Clean(bool bPreserve=false)
	{
		CLinkItem *pLinkItem = m_pHead;

		while ( pLinkItem != NULL ) {
			CLinkItem *pLinkItemTemp = pLinkItem;
			pLinkItem = pLinkItem->m_pNext;
			if ( !bPreserve ) {
				delete (OBJECT *)pLinkItemTemp->m_pItem;
			}

			m_linkItemCache.SafeReleaseLinkItem(pLinkItemTemp);
			////KSH use available list for performance
			////delete pLinkItemTemp;
		}

		m_pHead = m_pTail = NULL;
		m_iNItems = 0;

		return true;
	};



	#define TRAVERSELIST(list, position, positionnext)					\
	for ( (position) = (list).GetFirstPosition((positionnext));			\
			(position) != (unsigned long)NULL;							\
			(position) = (positionnext),								\
				(positionnext) = (list).GotoNextPosition((position))	\
	)


	#define TRAVERSEITEM(list, position, positionnext, item)			\
	for ( (position) = (list).GetFirstPosition((positionnext)),			\
				(item) = (list).GetCurrentItem((position));				\
			(position) != (unsigned long)NULL && (item) != NULL;		\
			(position) = (positionnext),								\
				(item) = (list).GetCurrentItem((position)),				\
				(positionnext) = (list).GotoNextPosition((position))	\
	)

	/* concatenation */
	bool Concatenation(CLinkedListTemplate<OBJECT>& src)
	{
		/* lock source first */
		src.Lock();

		OBJECT *pItem;
		unsigned long ulPosition, ulPositionNext;
		TRAVERSELIST(src, ulPosition, ulPositionNext)
		{
			pItem = src.GetCurrentItem(ulPosition);

			if ( pItem == NULL )
				continue;

			// by sasgas 2003.8.24
			if ( !src.DeleteItem(pItem, true) || !AddItem(pItem) ) {
				src.Unlock();
				//fprintf(stderr, "CLinkedListTemplate::Concatenation::Delete or Add Fail\n");
				return false;
			}
		}

		/* unlock */
		src.Unlock();

		return true;
	};

	/* traversing facility		  */
	/* needs the external locking */
	/* You can traverse simply with OBJECT *GetItem(int iIndex). */
	/* But this is not efficient because it traverses the link repeatedly whenever */
	/* the function is called. */
	unsigned long GetFirstPosition()
	{
		return (unsigned long)m_pHead;
	};

	unsigned long GetFirstPosition(unsigned long &ulSecondPosition)
	{
		if ( m_pHead == NULL ) {
			ulSecondPosition = (unsigned long)NULL;
		}
		else {
			ulSecondPosition = (unsigned long)m_pHead->m_pNext;
		}
		return (unsigned long)m_pHead;
	}

	unsigned long GetLastPosition()
	{
		return (unsigned long)m_pTail;
	};

	unsigned long GotoNextPosition(unsigned long ulCurrentPosition)
	{
		CLinkItem *pCurrentLinkItem = (CLinkItem *)ulCurrentPosition;
		if ( pCurrentLinkItem == NULL ) {
			return (unsigned long)NULL;
		}

		return (unsigned long)pCurrentLinkItem->m_pNext;
	};

	unsigned long GotoPreviousPosition(unsigned long ulCurrentPosition)
	{
		CLinkItem *pCurrentLinkItem = (CLinkItem *)ulCurrentPosition;
		if ( pCurrentLinkItem == NULL ) {
			return (unsigned long)NULL;
		}

		return (unsigned long)pCurrentLinkItem->m_pPrevious;
	};

	OBJECT *GetCurrentItem(unsigned long ulCurrentPosition)
	{
		CLinkItem *pLinkItem = (CLinkItem *)ulCurrentPosition;

		if ( pLinkItem == NULL ) {
			return NULL;
		}

		return (OBJECT *)pLinkItem->m_pItem;
	};

	OBJECT *GetCurrentItemForward(unsigned long ulCurrentPosition, unsigned long& ulNextPosition)
	{
		CLinkItem *pCurrentLinkItem = (CLinkItem *)ulCurrentPosition;

		if ( pCurrentLinkItem == NULL ) {
			ulNextPosition = (unsigned long)NULL;
			return NULL;
		}

		CLinkItem *pNextLinkItem = pCurrentLinkItem->m_pNext;
		ulNextPosition = (unsigned long)pNextLinkItem;

		return (OBJECT *)pCurrentLinkItem->m_pItem;
	};

	OBJECT *GetCurrentItemBackward(unsigned long ulCurrentPosition, unsigned long& ulPreviousPosition)
	{
		CLinkItem *pCurrentLinkItem = (CLinkItem *)ulCurrentPosition;

		if ( pCurrentLinkItem == NULL ) {
			ulPreviousPosition = (unsigned long)NULL;
			return NULL;
		}

		CLinkItem *pPreviousLinkItem = pCurrentLinkItem->m_pPrevious;
		ulPreviousPosition = (unsigned long)pPreviousLinkItem;

		return (OBJECT *)pCurrentLinkItem->m_pItem;
	};

	/* the internal locking version just for convenience */
	int SafeFindIndex(OBJECT *pItem)
	{
		if ( _mutex.Lock() == false ) {
			//fprintf(stderr, "CLinkedListTemplate::SafeFindIndex::_mutex.Lock() FAIL\n");
			return INVALID_SAFE_LINKED_LIST_INDEX;
		}

		int iIndex = FindIndex(pItem);

		if ( _mutex.UnLock() == false ) {
			//fprintf(stderr, "CLinkedListTemplate::SafeFindIndex::_mutex.UnLock() FAIL\n");
			iIndex = INVALID_SAFE_LINKED_LIST_INDEX;
		}

		return iIndex;
	};

	OBJECT *SafeFindItemFromID(unsigned long ulID)
	{
		if ( _mutex.Lock() == false ) {
			//fprintf(stderr, "CLinkedListTemplate::SafeFindItemFromID::_mutex.Lock() FAIL\n");
			return NULL;
		}

		OBJECT *pItem = FindItemFromID(ulID);

		if ( _mutex.UnLock() == false ) {
			//fprintf(stderr, "CLinkedListTemplate::SafeFindItemFromID::_mutex.UnLock() FAIL\n");
			return NULL;
		}

		return pItem;
	}

	OBJECT *SafeGetItem(int iIndex)
	{
		if ( _mutex.Lock() == false ) {
			//fprintf(stderr, "CLinkedListTemplate::SafeGetItem::_mutex.Lock() FAIL\n");
			return NULL;
		}

		OBJECT *pItem = GetItem(iIndex);

		if ( _mutex.UnLock() == false ) {
			//fprintf(stderr, "CLinkedListTemplate::SafeGetItem::_mutex.UnLock() FAIL\n");
			return NULL;
		}

		return pItem;
	};

	int SafeGetNItems()
	{
		if ( _mutex.Lock() == false ) {
			//fprintf(stderr, "CLinkedListTemplate::SafeGetNItems::_mutex.Lock() FAIL\n");
			return -1;
		}

		int iNItems = GetNItems();

		if ( _mutex.UnLock() == false ) {
			//fprintf(stderr, "CLinkedListTemplate::SafeGetNItems::_mutex.UnLock() FAIL\n");
			return -1;
		}

		return iNItems;
	};

	bool SafeAddItem(OBJECT *pItem)
	{
		if ( _mutex.Lock() == false ) {
			//fprintf(stderr, "CLinkedListTemplate::SafeAddItem::_mutex.Lock() FAIL\n");
			return false;
		}

		bool bResult = AddItem(pItem);

		if ( _mutex.UnLock() == false ) {
			//fprintf(stderr, "CLinkedListTemplate::SafeAddItem::_mutex.UnLock() FAIL\n");
			return false;
		}

		return bResult;
	};

	bool SafeDeleteItem(OBJECT *pItem, bool bPreserve=false)
	{
		if ( _mutex.Lock() == false ) {
			//fprintf(stderr, "CLinkedListTemplate::SafeDeleteItem::_mutex.Lock() FAIL\n");
			return false;
		}

		bool bResult = DeleteItem(pItem, bPreserve);

		if ( _mutex.UnLock() == false ) {
			//fprintf(stderr, "CLinkedListTemplate::SafeDeleteItem::_mutex.UnLock() FAIL\n");
			return false;
		}

		return bResult;
	};

	bool SafeModifyItem(OBJECT *pItemBefore, OBJECT *pItemAfter, bool bPreserve=false)
	{
		if ( _mutex.Lock() == false ) {
			//fprintf(stderr, "CLinkedListTemplate::SafeModifyItem::_mutex.Lock() FAIL\n");
			return false;
		}

		bool bResult = ModifyItem(pItemBefore, pItemAfter, bPreserve);

		if ( _mutex.UnLock() == false ) {
			//fprintf(stderr, "CLinkedListTemplate::SafeModifyItem::_mutex.UnLock() FAIL\n");
			return false;
		}

		return bResult;
	};

	bool SafeClean(bool bPreserve=false)
	{
		if ( _mutex.Lock() == false ) {
			//fprintf(stderr, "CLinkedListTemplate::SafeClean::_mutex.Lock() FAIL\n");
			return false;
		}

		bool bResult = Clean(bPreserve);

		if ( _mutex.UnLock() == false ) {
			//fprintf(stderr, "CLinkedListTemplate::SafeClean::_mutex.UnLock() FAIL\n");
			return false;
		}

		return bResult;
	};

	bool SafeConcatenation(CLinkedListTemplate<OBJECT>& src)
	{
		if ( _mutex.Lock() == false ) {
			//fprintf(stderr, "CLinkedListTemplate::SafeConcatenation::_mutex.Lock() FAIL\n");
			return false;
		}

		bool bResult = Concatenation(src);

		if ( _mutex.UnLock() == false ) {
			//fprintf(stderr, "CLinkedListTemplate::SafeConcatenation::_mutex.UnLock() FAIL\n");
			return false;
		}

		return bResult;
	};

/* overridables */
	/* get the id of the object */
	virtual unsigned long GetIDOf(OBJECT *pItem)
	{
		return (unsigned long)pItem;
	}

	virtual bool IsIDEqual(unsigned long ulID1, unsigned long ulID2)
	{
		return (ulID1 == ulID2);
	}
};

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif // !defined(AFX_LINKEDLISTTEMPLATE_H__27C93B59_3081_4D1B_AF4E_17916F41B2A4__INCLUDED_)
