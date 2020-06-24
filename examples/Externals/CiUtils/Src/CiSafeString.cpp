#include "internal_CiUtils.h"
#include "CiSafeString.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

int CiStrLen(const char *pszString)
{
	int iLength = 0;
	const char *ptr = pszString;

	while ( *ptr != '\0' ) {
		iLength++;
		ptr++;
	}

	return iLength;
}

bool CiStrInit(char *pszString)
{
	*pszString = '\0';

	return true;
}

bool CiStrCpy(char *pszDest, const char *pszSrc, int iNDest, int* piNTriedToCreate)
{
	int iIndex;
	int iNTriedToCreateTemp;

	if ( pszDest == NULL || pszSrc == NULL ) {
		if ( pszSrc == NULL ) {
			iNTriedToCreateTemp = 0;
		}
		else {
			iNTriedToCreateTemp = CiStrLen(pszSrc);
		}

		if ( piNTriedToCreate != NULL ) {
			*piNTriedToCreate = iNTriedToCreateTemp;
		}

		return false;
	}

	/* to the end of the source string */
	iIndex = 0;
	while ( *(pszSrc + iIndex) != '\0' ) {
		/* the safe string copy */
		if ( iIndex < iNDest ) {
			*(pszDest + iIndex) = *(pszSrc + iIndex);
		}
		iIndex++;
	}

	/* guarantee a safe null termination */
	/* unexpected truncation */
	if ( iIndex >= iNDest ) {
		*(pszDest + iNDest - 1) = '\0';
		/* iIndex is the length of the source string */
		iNTriedToCreateTemp = iIndex;
		if ( piNTriedToCreate != NULL ) {
			*piNTriedToCreate = iNTriedToCreateTemp;
		}
		return false;
	}
	/* successful copy */
	else if ( iIndex < iNDest ) {
		*(pszDest + iIndex) = '\0';
		/* iIndex is the length of the source string */
		iNTriedToCreateTemp = iIndex;
		if ( piNTriedToCreate != NULL ) {
			*piNTriedToCreate = iNTriedToCreateTemp;
		}
		return true;
	}

	/* iIndex is the length of the source string */
	iNTriedToCreateTemp = iIndex;
	if ( piNTriedToCreate != NULL ) {
		*piNTriedToCreate = iNTriedToCreateTemp;
	}

	return false;
}

bool CiStrCat(char *pszDest, const char *pszSrc, int iNDest, int* piNTriedToCreate)
{
	int iNTriedToCreateTemp = 0;

	if ( pszDest == NULL || pszSrc == NULL ) {
		if ( pszSrc == NULL && pszDest == NULL ) {
			iNTriedToCreateTemp = 0;
		}
		else if ( pszSrc == NULL && pszDest != NULL ) {
			iNTriedToCreateTemp = CiStrLen(pszDest);
		}
		else if ( pszSrc != NULL && pszDest == NULL ) {
			iNTriedToCreateTemp = CiStrLen(pszSrc);
		}

		if ( piNTriedToCreate != NULL ) {
			*piNTriedToCreate = iNTriedToCreateTemp;
		}

		return false;
	}

	/* to the end of the source string */
	int iIndex = 0;
	int iDestLength = CiStrLen(pszDest);
	while ( *(pszSrc + iIndex) != '\0' ) {
		/* the safe string copy */
		if ( iIndex+iDestLength < iNDest ) {
			*(pszDest + iIndex + iDestLength ) = *(pszSrc + iIndex);
		}
		iIndex++;
	}

	/* guarantee a safe null termination */
	/* unexpected truncation */
	if ( iIndex+iDestLength >= iNDest ) {
		*(pszDest + iNDest - 1) = '\0';
		/* iIndex is the length of the source string */
		iNTriedToCreateTemp = iIndex + iDestLength;
		if ( piNTriedToCreate != NULL ) {
			*piNTriedToCreate = iNTriedToCreateTemp;
		}

		return false;
	}
	/* successful copy */
	else if ( iIndex+iDestLength < iNDest ) {
		*(pszDest + iIndex + iDestLength) = '\0';
		/* iIndex is the length of the source string */
		iNTriedToCreateTemp = iIndex + iDestLength;
		if ( piNTriedToCreate != NULL ) {
			*piNTriedToCreate = iNTriedToCreateTemp;
		}
		return true;
	}

	/* iIndex is the length of the source string */
	iNTriedToCreateTemp = iIndex + iDestLength;
	if ( piNTriedToCreate != NULL ) {
		*piNTriedToCreate = iNTriedToCreateTemp;
	}

	return false;
}
