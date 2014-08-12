#include "internal_CiUtils.h"
#include "CiSafeString.h"
#include "CiConfig.h"
#include "limits.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

CiConfigNode::CiConfigNode()
{
	CiStrInit( m_szKey );
	CiStrInit( m_szValue );
	this->m_pcnNext = NULL;
}

CiConfig::CiConfig()
{
	m_pFile = NULL;
	m_pHead = NULL;
}

CiConfig::~CiConfig()
{
	m_szFileName = "";

	if ( m_pFile != NULL )
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}

	Clear();
}

bool CiConfig::Open( const char* pszConfigFileName )
{
	//	open하기전 기존 모든 데이터들을 초기화 한다.
	m_szFileName = "";

	if ( m_pFile != NULL )
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}

	Clear();

	//	open동작을 한다.
	m_szFileName = pszConfigFileName;

	m_pFile = fopen( pszConfigFileName , "r+t" );
	if ( m_pFile == NULL )
	{
		// No file
		m_pFile = fopen( pszConfigFileName , "w+t" );
	}

	if ( m_pFile == NULL ) {
		return false;
	}

	return Parse(m_pFile);
}

bool CiConfig::ReadInt( const char* arg_pszKey , int* piValue )
{
	int nTriedToCreate;
	char* pszKey;
	char bufKey[BUFFER_SIZE];
	if ( !CiStrCpy( bufKey , arg_pszKey , BUFFER_SIZE , &nTriedToCreate ) ) {
		return false;
	}
	pszKey = TrimWhiteSpace(bufKey);
	if ( pszKey == NULL || *pszKey == '\0' ) {
		return false;
	}

	CiConfigNode* pPrev = NULL;
	CiConfigNode* pNode = IsKey( pszKey , pPrev);
	if ( pNode != NULL )
	{
		// no conversion
		// if digit...non-digit.. conversion occurs until firsi 'non-digit'
		if ( !isdigit(pNode->m_szValue[0]) ) {
			return false;
		}

		// overflow or underflow
		long	lValue = strtol( pNode->m_szValue , NULL , 10 );
		if ( lValue == LONG_MIN || lValue == LONG_MAX ) {
			return false;
		}
		*piValue = (int)lValue;		// Data loss may occur.

		return true;
	}

	return false;
}

bool CiConfig::ReadInt64( const char* arg_pszKey , long long* pi64Value )
{
	int nTriedToCreate;
	char* pszKey;
	char bufKey[BUFFER_SIZE];
	if ( !CiStrCpy( bufKey , arg_pszKey , BUFFER_SIZE , &nTriedToCreate ) ) {
		return false;
	}
	pszKey = TrimWhiteSpace(bufKey);
	if ( pszKey == NULL || *pszKey == '\0' ) {
		return false;
	}

	CiConfigNode* pPrev = NULL;
	CiConfigNode* pNode = IsKey( pszKey , pPrev);
	if ( pNode != NULL )
	{
		// no conversion
		// if digit...non-digit.. conversion occurs until firsi 'non-digit'
		if ( !isdigit(pNode->m_szValue[0]) ) {
			return false;
		}

		// overflow or underflow
#ifdef _WIN32
		*pi64Value = _atoi64( pNode->m_szValue );
#else
		*pi64Value = strtoll( pNode->m_szValue , NULL , 10 );
		if ( *pi64Value == LONG_LONG_MIN || *pi64Value == LONG_LONG_MAX ) {
			return false;
		}
#endif

		return true;
	}

	return false;
}

bool CiConfig::ReadString( const char* arg_pszKey , char* pszValue , int iBufferSize )
{
	int nTriedToCreate;
	char* pszKey;
	char bufKey[CiConfigNode::BUFFER_KEY_SIZE];
	if ( !CiStrCpy( bufKey , arg_pszKey , CiConfigNode::BUFFER_KEY_SIZE , &nTriedToCreate ) ) {
		return false;
	}

	pszKey = TrimWhiteSpace(bufKey);
	if ( pszKey == NULL || *pszKey == '\0' ) {
		return false;
	}

	CiConfigNode* pPrev = NULL;
	CiConfigNode* pNode = IsKey( pszKey , pPrev);
	if ( pNode != NULL )
	{
		int nTriedToCreate;
		if ( CiStrCpy( pszValue , pNode->m_szValue , iBufferSize , &nTriedToCreate ) )
			return true;
	}

	return false;
}

bool CiConfig::WriteInt( const char* arg_pszKey , int iValue )
{
	int nTriedToCreate;
	char* pszKey;
	char* pszValue;
	char bufKey[CiConfigNode::BUFFER_KEY_SIZE];
	char bufValue[CiConfigNode::BUFFER_VALUE_SIZE];
	if ( !CiStrCpy( bufKey , arg_pszKey , CiConfigNode::BUFFER_KEY_SIZE , &nTriedToCreate ) ) {
		return false;
	}
	pszKey = TrimWhiteSpace(bufKey);
	if ( pszKey == NULL || *pszKey == '\0' ) {
		return false;
	}

	if ( iValue >= INT_MAX || iValue <= INT_MIN ) {
		return false;
	}
#ifdef _WIN32
	itoa( iValue , bufValue , 10 );
#else
	snprintf( bufValue, CiConfigNode::BUFFER_VALUE_SIZE-1, "%d", iValue);
#endif
	pszValue = TrimWhiteSpace(bufValue);
	if ( pszValue == NULL || pszValue == '\0' ) {
		return false;
	}

	if ( !WriteImpl(pszKey, pszValue) )
	{
		return false;
	}

	return true;
}

bool CiConfig::WriteInt64( const char* /*pszKey*/ , long long /*i64Value*/ )
{
	return false;
}

bool CiConfig::WriteString( const char* arg_pszKey , const char* arg_pszValue )
{
	int nTriedToCreate;
	char* pszKey;
	char* pszValue;
	char bufKey[CiConfigNode::BUFFER_KEY_SIZE];
	char bufValue[CiConfigNode::BUFFER_VALUE_SIZE];
	if ( !CiStrCpy( bufKey , arg_pszKey , CiConfigNode::BUFFER_KEY_SIZE , &nTriedToCreate ) ) {
		return false;
	}
	pszKey = TrimWhiteSpace(bufKey);
	if ( pszKey == NULL || *pszKey == '\0' ) {
		return false;
	}
	//if ( !CiStrCpy( bufValue , arg_pszValue , CiConfigNode::BUFFER_VALUE_SIZE , &nTriedToCreate ) ) {
	//	return false;
	//}
	//pszValue = TrimWhiteSpace(bufValue);

	if( arg_pszValue == NULL )    
	{
		pszValue = NULL;
	}
	else 
	{
		if ( !CiStrCpy( bufValue , arg_pszValue , CiConfigNode::BUFFER_VALUE_SIZE , &nTriedToCreate ) ) {
			return false;
		}
		pszValue = TrimWhiteSpace(bufValue);
	}

	if ( !WriteImpl(pszKey, pszValue) )
	{
		return false;
	}

	return true;
}

bool CiConfig::WriteImpl(const char* pszKey, const char* pszValue)
{
	int nTriedToCreate;
	CiConfigNode* pPrev = NULL;
	CiConfigNode* pNode = IsKey( pszKey , pPrev );

	if ( pNode != NULL )
	{
		if ( pszValue == NULL || *pszValue == '\0' )
		{
			// delete
			// pPrev -> pNode -> pNext
			if ( pPrev != NULL )
			{
				pPrev->m_pcnNext = pNode->m_pcnNext;
				delete pNode;
			}
			else
			{
				// means pNode(head) -> pNext
				m_pHead = pNode->m_pcnNext;
				delete pNode;
			}
			return true;
		}
		// modify
		if ( CiStrCpy( pNode->m_szValue , pszValue , CiConfigNode::BUFFER_VALUE_SIZE , &nTriedToCreate ) )
			return true;
	}
	else
	{
		if ( pszValue == NULL || *pszValue == '\0' )
		{
			// No key and No data ?
			return false;
		}

		// add
		CiConfigNode* pNode = new CiConfigNode;
		if ( pNode != NULL )
		{
			if (
				CiStrCpy( pNode->m_szKey , pszKey , CiConfigNode::BUFFER_KEY_SIZE , &nTriedToCreate )
				&& CiStrCpy( pNode->m_szValue , pszValue , CiConfigNode::BUFFER_VALUE_SIZE , &nTriedToCreate )
				)
			{
				if ( m_pHead != NULL )
				{
					CiConfigNode* parent = m_pHead;
					while ( parent->m_pcnNext ) parent = parent->m_pcnNext;
					parent->m_pcnNext = pNode;
				}
				else
					m_pHead = pNode;

				return true;
			}
		}
	}

	return false;
}

bool CiConfig::Flush()
{
	// truncate
	if ( m_pFile != NULL )
	{
		CiTruncate( fileno(m_pFile) , 0 );
		fseek( m_pFile , 0 , SEEK_SET );		/* 2003.08.27 */

		// rewrite whole
		CiConfigNode* pNode = NULL;
		pNode = m_pHead;
		while( pNode )
		{
			if ( (pNode->m_szKey == NULL) || *pNode->m_szKey == '\0' )
				fprintf(m_pFile , "%s\n" , pNode->m_szValue );
			else
				fprintf(m_pFile , "%s=%s\n" , pNode->m_szKey , pNode->m_szValue );
			pNode = pNode->m_pcnNext;
		}
		fflush( m_pFile );
	}

	return true;
}

bool CiConfig::Close()
{
	//	디스크 공간이 부족 할 경우 cfg가 날라가는 버그 수정을 위해 Flush()를 주석 처리함
	//Flush();
	if ( m_pFile != NULL )
	{
		fclose( m_pFile );
		m_pFile = NULL;
	}

	//	만약.. open하다가 파일이름은 들어왔는데 open을 실패하였고, close를 호출하게되면 해당 cfg는 삭제될것 이다.
	//if ( m_pHead == NULL )
	//{
	//	// No key - delete config file
	//	remove( m_szFileName.c_str() );
	//}

	//	그리고 close를 했다고 해서 반드시 open시 load한 값들을 구지 지울 필요는 없을것 같다.

	return true;
}

CiConfigNode* CiConfig::Clear()
{
	CiConfigNode* pNext = NULL;
	CiConfigNode* pCur = NULL;

	pCur = m_pHead;

	while( pCur )
	{
		pNext = pCur->m_pcnNext;
		delete pCur;
		pCur = pNext;
	}

	m_pHead = NULL;

	return m_pHead;
}

CiConfigNode* CiConfig::IsKey( const char* pszKey , CiConfigNode*& pPrev)
{
	CiConfigNode* pCur = NULL;
	pPrev = NULL;

	pCur = m_pHead;
	while( pCur )
	{
		if ( strcmp( pCur->m_szKey , pszKey ) == 0 )
			return pCur;

		pPrev = pCur;
		pCur = pCur->m_pcnNext;
	}
	return NULL;
}

bool CiConfig::Parse(FILE* pFile)
{
	CiConfigNode* pNode = NULL;
	int nTriedToCreate = 0;

	char buf[BUFFER_SIZE];
	char* p = NULL;
	char seps[] = "=";
	char sepsEnd [] = "\n";

	while( fgets( buf , BUFFER_SIZE , pFile ) )
	{
		p = TrimWhiteSpace(buf);
		pNode = NULL;

		if ( p == NULL )
			continue;
		else if ( *p == '=' )
			return false;
		else if ( *p == '#' )
		{
			char* saveptr = NULL;
			char* tokenValue = CiStrtok( p , sepsEnd  , &saveptr);
			tokenValue = TrimWhiteSpace( tokenValue );

			if ( tokenValue != NULL )
			{
				pNode = new CiConfigNode;
				if ( pNode == NULL ) {
					return false;
				}

				if ( !CiStrCpy( pNode->m_szValue , tokenValue , CiConfigNode::BUFFER_VALUE_SIZE , &nTriedToCreate ) )
				{
					delete pNode;
					return false;
				}
			}
		}
		else
		{
			char* saveptr = NULL;
			char* tokenKey = CiStrtok( p , seps , &saveptr);
			char* tokenValue = CiStrtok( NULL , sepsEnd , &saveptr);
			tokenKey = TrimWhiteSpace( tokenKey );
			tokenValue = TrimWhiteSpace( tokenValue );

			if ( tokenKey != NULL && *tokenKey != '\0' && tokenValue != NULL && *tokenValue != '\0')
			{
				pNode = new CiConfigNode;
				if ( pNode == NULL ) {
					return false;
				}

				if ( !CiStrCpy( pNode->m_szKey , tokenKey , CiConfigNode::BUFFER_KEY_SIZE , &nTriedToCreate )
					|| !CiStrCpy( pNode->m_szValue , tokenValue , CiConfigNode::BUFFER_VALUE_SIZE , &nTriedToCreate )
					)
				{
					delete pNode;
					return false;
				}
			}
		}

		// add node
		if ( pNode != NULL )
		{
			if ( m_pHead != NULL )
			{
				CiConfigNode* parent = m_pHead;
				while ( parent->m_pcnNext ) parent = parent->m_pcnNext;
				parent->m_pcnNext = pNode;
			}
			else
				m_pHead = pNode;
		}
	}

	if ( feof( pFile ) )
		return true;

	return false;
}

char* CiConfig::TrimWhiteSpace(char* token)
{
	char* pStart;
	char* p = token;
	if ( token == NULL ) {
		return NULL;
	}

	while(
			p!= NULL
			&& ( *p == '\t' || *p == ' ' )
		)
		p++;

	// first char not white space
	pStart = p;

	size_t len = strlen(token);
	p = token + len - 1;
	// replace white space with null-termination
	while(
			p != NULL
			&& ( *p == '\t' || *p == '\r' || *p == ' ' )
		)
	{
		*p = '\0';
		p--;
	}

	return pStart;
}

/* extract ids from id string */
bool CiConfig::GetIDsFromIDString(int *piIDs, int& iNIDs, const char *pszIDs, int iMaxNIDs)
{
	if ( piIDs == NULL ) {
		return false;
	}

	if ( pszIDs == NULL ) {
		return false;
	}

	char szIDs[256];
	char *ptrID;
	int iNIDsTemp = 0;
	char* saveptr = NULL;

	int iNTriedToCreate;
	CiStrCpy(szIDs, pszIDs, sizeof(szIDs), &iNTriedToCreate);
	ptrID = CiStrtok(szIDs, ID_DELIMITER , &saveptr);
	while ( ptrID != NULL && iNIDsTemp < iMaxNIDs ) {
		piIDs[iNIDsTemp++] = atoi(ptrID);
		ptrID = CiStrtok(NULL, ID_DELIMITER , &saveptr);
	}

	iNIDs = iNIDsTemp;

	return true;
}

/* make id string from ids */
bool CiConfig::GetIDStringFromIDs(int *piIDs, int iNIDs, char *pszIDs, int iBufferLength)
{
	if ( piIDs == NULL ) {
		return false;
	}

	int iNTriedToCreate;
	char szIDsTemp[256];
	char szID[256];
	CiStrInit(szIDsTemp);
	CiStrInit(szID);
	for ( int i = 0; i < iNIDs; i++ ) {
		sprintf(szID, "%d", piIDs[i]);
		CiStrCat(szIDsTemp, szID, sizeof(szIDsTemp), &iNTriedToCreate);
		if ( i < iNIDs - 1 ) {
			CiStrCat(szIDsTemp, ID_DELIMITER, sizeof(szIDsTemp), &iNTriedToCreate);	/*,*/
		}
	}

	CiStrCpy(pszIDs, szIDsTemp, iBufferLength, &iNTriedToCreate);

	return true;
}
