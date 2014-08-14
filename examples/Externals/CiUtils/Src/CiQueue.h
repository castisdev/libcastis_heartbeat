// CiQueue.h: interface for the CiQueue class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CIQUEUE_H__9EDF8C1C_6576_4FC9_B6C4_D2D93F79B124__INCLUDED_)
#define AFX_CIQUEUE_H__9EDF8C1C_6576_4FC9_B6C4_D2D93F79B124__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CiSemaphore.h"
#include "LinkedListTemplate.h"

class CiQueue
{
public:
	CiQueue(int iCacheSize=100) : m_linkItemCache(iCacheSize)
	{
		m_pHead = m_pTail = NULL;
		m_iNItems = 0;

		m_semLock.Initialize(1);
	};

	virtual ~CiQueue()
	{
		m_semLock.Finalize();
	};

private:
	CLinkItemCache m_linkItemCache;

	CCiSemaphore m_semLock;

	CLinkItem *m_pHead;
	CLinkItem *m_pTail;

	int m_iNItems;

public:
	int SafeGetNItems()
	{
		if ( m_semLock.Wait() == false ) {
			//fprintf(stderr, "CiQueue::SafeGetNItems::m_semLock.Wait() FAIL\n");
			return -1;
		}

		int iNItems = m_iNItems;

		if ( m_semLock.Post() == false ) {
			//fprintf(stderr, "CiQueue::SafeGetNItems::m_semLock.Post() FAIL\n");
			return -1;
		}

		return iNItems;
	};

	bool SafeEnqueue(void *pItem)
	{
		////KSH use available list for improving performance
		////CLinkItem *pLinkItem = new CLinkItem;
		CLinkItem *pLinkItem = m_linkItemCache.SafeAllocLinkItem();
		if ( pLinkItem == NULL ) {
			//fprintf(stderr, "CiQueue::SafeEnqueue::pLinkItem NULL\n");
			return false;
		}

		pLinkItem->m_pPrevious = NULL;
		pLinkItem->m_pNext = NULL;
		pLinkItem->m_pItem = pItem;

		if ( m_semLock.Wait() == false ) {
			//fprintf(stderr, "CiQueue::SafeEnqueue::m_semLock.Wait() FALSE\n");
			return false;
		}

		if ( m_pHead == NULL && m_pTail == NULL ) {
			m_pHead = m_pTail = pLinkItem;
		}
		else {
			m_pTail->m_pNext = pLinkItem;
			pLinkItem->m_pPrevious = m_pTail;
			m_pTail = pLinkItem;
		}

		m_iNItems++;

		if ( m_semLock.Post() == false ) {
			//fprintf(stderr, "CiQueue::SafeEnqueue::m_semLock.Post() FALSE\n");
			return false;
		}

		return true;
	};

	bool SafeDequeue(void **ppItem)
	{
		if ( m_semLock.Wait() == false ) {
			//fprintf(stderr, "CiQueue::SafeDequeue::m_semLock.Wait() FALSE\n");
			return false;
		}

		if ( m_pHead == NULL ) {
			*ppItem = NULL;

			if ( m_semLock.Post() == false ) {
				//fprintf(stderr, "CiQueue::SafeDequeue::m_pHead NULL And m_semLock.Post() FALSE\n");
				return false;
			}

			//fprintf(stderr, "CiQueue::SafeDequeue::m_pHead NULL\n");
			return false;
		}

		CLinkItem *pLinkItem = m_pHead;

		if ( m_pHead == m_pTail ) {
			/* only one item */
			m_pHead = m_pTail = NULL;
		}
		else {
			/* delete the head */
			CLinkItem *pSecond = m_pHead->m_pNext;

			pSecond->m_pPrevious = NULL;
			m_pHead = pSecond;
		}

		m_iNItems--;

		if ( m_semLock.Post() == false ) {
			//fprintf(stderr, "CiQueue::SafeDequeue::m_semLock.Post() FALSE\n");
			return false;
		}

		*ppItem = pLinkItem->m_pItem;

		/* should delete the indirect index even if do not delete pItem */
		////KSH use available list for improving performance
		////delete pLinkItem;
		m_linkItemCache.SafeReleaseLinkItem(pLinkItem);

		return true;
	};

	/* these are raw interfaces for the CiQueue */
	/* You are responsible for ensuring the safy of the queue */
	/* These raw interfaces can be used for gaining more performance */
	/* These member functions are declared as inline functions */
	inline bool Lock()
	{
		return m_semLock.Wait();
	};

	inline bool Unlock()
	{
		return m_semLock.Post();
	};

	inline int RawGetNItems()
	{
		return m_iNItems;
	};

	inline void RawSetNItems(int iNItems)
	{
		m_iNItems = iNItems;
	};

	inline bool RawEnqueue(void *pItem)
	{
		////KSH use available list for improving performance
		////CLinkItem *pLinkItem = new CLinkItem;
		CLinkItem *pLinkItem = m_linkItemCache.SafeAllocLinkItem();
		if ( pLinkItem == NULL ) {
			//fprintf(stderr, "CiQueue::RawEnqueue::Alloc FALSE\n");
			return false;
		}

		pLinkItem->m_pPrevious = NULL;
		pLinkItem->m_pNext = NULL;
		pLinkItem->m_pItem = pItem;

		if ( m_pHead == NULL && m_pTail == NULL ) {
			m_pHead = m_pTail = pLinkItem;
		}
		else {
			m_pTail->m_pNext = pLinkItem;
			pLinkItem->m_pPrevious = m_pTail;
			m_pTail = pLinkItem;
		}

		return true;
	};

	inline bool RawDequeue(void **ppItem)
	{
		if ( m_pHead == NULL ) {
			*ppItem = NULL;

			//fprintf(stderr, "CiQueue::RawDequeue::m_pHead NULL\n");
			return false;
		}

		CLinkItem *pLinkItem = m_pHead;

		if ( m_pHead == m_pTail ) {
			/* only one item */
			m_pHead = m_pTail = NULL;
		}
		else {
			/* delete the head */
			CLinkItem *pSecond = m_pHead->m_pNext;

			pSecond->m_pPrevious = NULL;
			m_pHead = pSecond;
		}

		*ppItem = pLinkItem->m_pItem;

		/* should delete the indirect index even if do not delete pItem */
		////KSH use available list for improving performance
		////delete pLinkItem;
		m_linkItemCache.SafeReleaseLinkItem(pLinkItem);

		return true;
	};

	inline bool RawLookAhead(void **ppItem)
	{
		if ( m_pHead == NULL ) {
			*ppItem = NULL;
			//fprintf(stderr, "CiQueue::RawLookAhead::m_pHead NULL\n");
			return false;
		}

		*ppItem = m_pHead->m_pItem;

		return true;
	};
};

typedef CLinkedListTemplate<CiQueue> CiQueueList;

#endif // !defined(AFX_CIQUEUE_H__9EDF8C1C_6576_4FC9_B6C4_D2D93F79B124__INCLUDED_)
