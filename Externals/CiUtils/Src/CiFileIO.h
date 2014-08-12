#pragma once

#include "CiGlobals.h"
#include <string>

class
#if defined(_WIN32) && defined(_USRDLL)
__declspec(dllexport)
#endif
CiFileIO
{
public:
	CiFileIO() : m_iFileID(-1) {}
	virtual ~CiFileIO() {}

	// same return value : -1(error) , fd(success)
	virtual int Open(const std::string& strFileName , int oflag , int pmode = CI_S_IREAD | CI_S_IWRITE ) = 0;
	// same return value : -1(error) , 0(success)
	virtual int Close() = 0;
	// same return value : -1(error) , read byte(success)
	virtual int Read( void* pBuffer , size_t count ) = 0;
	// same return value : -1L(error) , current offset(success)
	virtual long long Seek(long long llOffset , int origin ) = 0;
	// same return value : -1(error) , written byte(success)
	virtual int Write( const void* pBuffer , size_t count ) = 0;
	// same return value : -1(error) , 0(success)
	virtual int Truncate(long long llSize) = 0;
	// same return value : -1(error) , 0(success)
	virtual int Flush() = 0;


	/*
	// -1(error) , 0(semantic-error) , 1(success)
	virtual int SetOpt(int Option , int iValueSize, void* pValue ) = 0;
	virtual int GetOpt(int Option , int* piValueSize, void* pValue ) = 0;
	*/

public:
	const std::string GetFileName() { return _strFileName; }
	const std::string GetFileDescription() { return _strFileDescription; }


public:
	/*
	enum
	{
	} Option_t;
	*/

protected:
	int m_iFileID;
	std::string _strFileName;
	std::string _strFileDescription;
};
