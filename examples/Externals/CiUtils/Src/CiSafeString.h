#ifndef __CISAFESTRING_H__
#define __CISAFESTRING_H__

int CiStrLen(const char *pszString);

/* Castis' string initialization function for the following safe operations */
/* the sizeof pszString buffer must be larger than 1. */
bool CiStrInit(char *pszString);

/* Castis' safe & string string copy function */
/* iNDest is the size of pszDest */
/* piNTryToCreate is a return value to check if an undesired truncation is occurred */
/* piNTryToCreate is the string length of pszSrc in CiStrCpy */
/* An undesired truncation is regarded as false, and a partial string copy will be done. */
/* You can catch the undesired truncation by comparing iNDest with piNTryToCreate */
/* if ( piNTriedToCreate >= iNDest ) then an undesired truncation */
/* CiStrCpy guarantees pszDest to be NULL-terminated in all cases. */
bool CiStrCpy(char *pszDest, const char *pszSrc, int iNDest, int* piNTriedToCreate);

/* Castis' safe & string string copy function */
/* iNDest is the size of pszDest */
/* piNTryToCreate is a return value to check if an undesired truncation is occurred */
/* piNTryToCreate is the string length of pszSrc + the string length of pszDest in CiStrCat */
/* An undesired truncation is regarded as false, and a partial string copy will be done. */
/* You can catch the undesired truncation by comparing iNDest with piNTryToCreate */
/* if ( piNTriedToCreate >= iNDest ) then an undesired truncation */
/* CiStrCat guarantees pszDest to be NULL-terminated in all cases. */
bool CiStrCat(char *pszDest, const char *pszSrc, int iNDest, int* piNTriedToCreate);

/* Please refer to the concept of strlcpy and strlcat */

#endif
