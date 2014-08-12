#ifndef __CI_CONFIG_H__
#define __CI_CONFIG_H__

#include <string>

#define ID_DELIMITER							","

class CiConfigNode
{
public:
	enum {
		BUFFER_KEY_SIZE = 256,
		BUFFER_VALUE_SIZE = 256
	};

public:
	CiConfigNode();

	char m_szKey[BUFFER_KEY_SIZE];
	char m_szValue[BUFFER_VALUE_SIZE];
	CiConfigNode* m_pcnNext;
};

///////////////////////////////////////////////////
// CiConfig : config text 파일을 작성한다.
// 멤버로 갖거나, open/close동안 갖고 있어야 함
// open -> read/write -> close
// return값이 false이면 모두 fail
///////////////////////////////////////////////
class CiConfig
{
public:
	CiConfig();
	virtual ~CiConfig();

	//////////////////////////////
	// open config file
	// when no file : create - so can't share
	// parsing .... if ( no key || no value ) -> no item is created
	//////////////////////////////////
	bool Open( const char* pszConfigFileName );

	/////////////////////////////////
	// read value with key
	// if given key is not exist : return false
	////////////////////////////////////

	// strtol.. if over/underflow/no conversion -> fail
	// if ( 1111a ) -> 1111( follows strtol )
	bool ReadInt( const char* pszKey, int* piValue);
	// not implemented yet
	bool ReadInt64( const char* pszKey , long long* pi64Value );
	// pszValue buffer에 buffersize만큼 read
	bool ReadString( const char* pszKey , char* pszValue , int iBufferSize );

	////////////////////////////////////////////
	// write value with key
	// if exist key : modify , else if key : add
	// WriteString( "key" , NULL ) , means delete key
	// if no more keys -> delete config file
	/////////////////////////////////////////////

	// if ( iValue >= LONG_MAX || iValue <= LONG_MIN ) -> fail
	bool WriteInt( const char* pszKey , int iValue);
	// Not implemented yet
	bool WriteInt64( const char* pszKey , long long i64Value );
	bool WriteString( const char* pszKey , const char* pszValue );
	bool WriteImpl(const char* pszKey, const char* pszValue);

	////////////////////////
	// if no close -> lose changes
	/////////////////////////
	// flush
	bool Flush();
	// flush and close
	bool Close();

	// Max 1 line size
	enum { BUFFER_SIZE = 256 };

	/* extract ids from id string */
	static bool GetIDsFromIDString(int *piIDs, int& iNIDs, const char *pszIDs, int iMaxNIDs);
	/* make id string from ids */
	static bool GetIDStringFromIDs(int *piIDs, int iNIDs, char *pszIDs, int iBufferLength);

protected:
	std::string m_szFileName;
	FILE* m_pFile;
	CiConfigNode* m_pHead;

	// read and parse node from file
	bool Parse(FILE* pFile);

	// is there key?
	CiConfigNode* IsKey(const char* pszKey, CiConfigNode*& pPrev);

	// cleaer node
	CiConfigNode* Clear();

	// trim white space at front and rear
	char* TrimWhiteSpace(char* token);
};

#endif //__CI_CONFIG_H__
