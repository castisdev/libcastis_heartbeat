// NetIOStream.cpp: implementation of the NetIOStream class.
//
//////////////////////////////////////////////////////////////////////
#include "internal_CiUtils.h"

#include "NetIOStream.h"
#include "NetIOStreamInterface.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

using std::string;
using namespace CiUtils;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
NetIOStream::NetIOStream(const char* buffer, int size)
{
	int reserveSize = size*2;

	_buffer = static_cast<char*>(malloc(reserveSize));
	memcpy(_buffer, buffer, size);

	_ptrBufferIn = _buffer + size;
	_ptrBufferOut = _buffer;
	_bufferSize = size;
}

NetIOStream::NetIOStream(int reserveSize/*=32*/)
{
	if (reserveSize < 0 )
		reserveSize = 1;

	_buffer = static_cast<char*>(malloc(reserveSize));
	_ptrBufferIn = _buffer;
	_ptrBufferOut = _buffer;
	_bufferSize = reserveSize;
}

NetIOStream::~NetIOStream()
{
	if (_buffer != NULL)
		free(_buffer);
}

//////////////////////////////////////////////////////////////////////////
// util

std::string NetIOStream::str()
{
	char* temp = new char[GetBufferSize()+1];
	memcpy(temp, GetBuffer(), GetBufferSize());
	temp[GetBufferSize()] = '\0';
	for (int i=0,n=GetBufferSize() ; i<n; i++)
		if (temp[i]=='\0') temp[i]='_';

	string tempStr(temp);
	delete []temp;
	return tempStr;
}

char* NetIOStream::CreateBuffer()
{
	char* temp = new char[GetBufferSize()];
	memcpy(temp, GetBuffer(), GetBufferSize());
	return temp;
}


void NetIOStream::PrepareBuffer(int size)
{
	// realloc case
	if (_ptrBufferIn - _buffer + size >= _bufferSize)
	{
		int newSize = _bufferSize * 2 + size;
		char* newBufferPtr = static_cast<char*>(realloc(_buffer, newSize));
		if (newBufferPtr == NULL)
		{
			//memory exception
			throw NetIOStreamException(NetIOStreamException::eOutOfMemory);
		}
		else
		{
			int outIndex = static_cast<int>(_ptrBufferOut - _buffer) ;
			int inIndex = static_cast<int>(_ptrBufferIn - _buffer);
			_buffer = newBufferPtr;
			_ptrBufferIn = _buffer + inIndex;
			_ptrBufferOut = _buffer + outIndex;
		}

		_bufferSize = newSize;
	}
}

NetIOStream& NetIOStream::operator+= (int size)
{
	if (_ptrBufferIn - _buffer + size >= _bufferSize)
	{
		//exception
		throw NetIOStreamException(NetIOStreamException::eOutOfBound);
	}
	else
	{
		_ptrBufferIn += size;
	}

	return *this;
}

NetIOStream& NetIOStream::operator++()
{
	if (_ptrBufferIn - _buffer + 1 >= _bufferSize)
	{
		//memory exception
		throw NetIOStreamException(NetIOStreamException::eOutOfBound);
	}
	else
	{
		_ptrBufferIn += 1;
	}
	return *this;
}


//////////////////////////////////////////////////////////////////////////
// in
NetIOStream& NetIOStream::operator<< (int8_t rhs)
{
	return (*this) << static_cast<uint8_t>(rhs);
}

NetIOStream& NetIOStream::operator<< (uint8_t rhs)
{
	PrepareBuffer(1);
	memcpy(_ptrBufferIn, &rhs, 1);
	(*this) += 1;

	return (*this);
}

NetIOStream& NetIOStream::operator<< (int16_t rhs)
{
	return (*this) << static_cast<uint16_t>(rhs);
}

NetIOStream& NetIOStream::operator<< (uint16_t rhs)
{
	uint16_t usNetworkOrder = htons(rhs);
	PrepareBuffer(2);
	memcpy(_ptrBufferIn, &usNetworkOrder, 2);
	(*this) += 2;

	return (*this);
}

NetIOStream& NetIOStream::operator<< (int32_t rhs)
{
	return (*this) << static_cast<uint32_t>(rhs);
};

NetIOStream& NetIOStream::operator<< (uint32_t rhs)
{
	uint32_t	ulNetworkOrder = htonl(rhs);
	PrepareBuffer(4);
	memcpy(_ptrBufferIn, &ulNetworkOrder, 4);
	(*this) += 4;
	
	return (*this);
};

NetIOStream& NetIOStream::operator<< (int64_t rhs)
{
	return (*this) << static_cast<uint64_t>(rhs);
}

NetIOStream& NetIOStream::operator<< (uint64_t rhs)
{
	uint64_t	int64_temp = rhs;

	if ( 1 != htonl(1) ) {
		uint32_t	higher = static_cast<uint32_t>((int64_temp & 0xFFFFFFFF00000000LL) >> 32);
		uint32_t	lower = static_cast<uint32_t>(int64_temp & 0x00000000FFFFFFFFLL);
		int64_temp = ( ((uint64_t)htonl(lower) << 32) | htonl(higher) );
	}

	PrepareBuffer(8);
	memcpy(_ptrBufferIn, &int64_temp, 8);
	(*this) += 8;

	return (*this);
}

NetIOStream& NetIOStream::operator<< (float rhs)
{
	float float_temp = rhs;

	if ( 1 != htonl(1) )
	{
		unsigned char* ptr = (unsigned char *)&rhs;
		unsigned char* ptr_temp = (unsigned char *)&float_temp;

		for (unsigned int i = 0; i < sizeof(float); i++)
			ptr_temp[i] = ptr[sizeof(float) - 1 - i];
	}

	PrepareBuffer(sizeof(float));
	memcpy(_ptrBufferIn, &float_temp, sizeof(float));
	(*this) += sizeof(float);

	return (*this);
}

NetIOStream& NetIOStream::operator<< (double rhs)
{
	double double_temp = rhs;

	if ( 1 != htonl(1) )
	{
		unsigned char* ptr = (unsigned char *)&rhs;
		unsigned char* ptr_temp = (unsigned char *)&double_temp;

		for (unsigned int i = 0; i < sizeof(double); i++)
			ptr_temp[i] = ptr[sizeof(double) - 1 - i];
	}

	PrepareBuffer(sizeof(double));
	memcpy(_ptrBufferIn, &double_temp, sizeof(double));
	(*this) += sizeof(double);

	return (*this);
}

NetIOStream& NetIOStream::operator<< (bool rhs)
{
	return (*this) << static_cast<uint32_t>(rhs);
};

NetIOStream& NetIOStream::operator<< (NetIOStreamInterface* rhs)
{
	rhs->NetIOStreamOut(*this);
	return (*this);
}

NetIOStream& NetIOStream::operator<< (const string& rhs)
{
	int iLengthNullTerminateString = static_cast<int>(rhs.length() + 1);
	(*this) << iLengthNullTerminateString;

	PrepareBuffer(iLengthNullTerminateString);
	memcpy(GetBufferIn(), rhs.c_str(), iLengthNullTerminateString);
	(*this) += iLengthNullTerminateString;

	return (*this);
}

NetIOStream& NetIOStream::operator<<(NetIOStream& rhs)
{
	PrepareBuffer(rhs.GetBufferSize());
	memcpy(_ptrBufferIn, rhs.GetBuffer(), rhs.GetBufferSize());
	(*this) +=rhs.GetBufferSize();

	return (*this);
}

//////////////////////////////////////////////////////////////////////////
// out

NetIOStream& NetIOStream::operator>> (int8_t& rhs)
{
	uint8_t	ulRhs = 0;
	(*this) >> ulRhs;
	rhs = static_cast<int8_t>(ulRhs);
	return (*this);
}

NetIOStream& NetIOStream::operator>> (uint8_t& rhs)
{
	if (_ptrBufferOut + 1 > _ptrBufferIn)
		throw NetIOStreamException(NetIOStreamException::eOutOfBound);

	memcpy((void*) &rhs, _ptrBufferOut, 1);

	_ptrBufferOut += 1;

	return (*this);
}

NetIOStream& NetIOStream::operator>> (int16_t& rhs)
{
	uint16_t	usRhs = 0;
	(*this) >> usRhs;
	rhs = static_cast<int16_t>(usRhs);
	return (*this);
}

NetIOStream& NetIOStream::operator>> (uint16_t& rhs)
{
	// FIXME: sizeof(u_short) 대신 magic number 를 쓰는게 맞을지도 모르겠다.
	if (_ptrBufferOut + sizeof(u_short) > _ptrBufferIn)
		throw NetIOStreamException(NetIOStreamException::eOutOfBound);

	uint16_t	usNetworkOrder = 0;
	memcpy(&usNetworkOrder, GetBufferOut(), 2);
	rhs = ntohs(usNetworkOrder);

	_ptrBufferOut += 2;

	return (*this);
}

NetIOStream& NetIOStream::operator>> (int32_t& rhs)
{
	uint32_t	ulRhs = 0;
	(*this) >> ulRhs;
	rhs = static_cast<int32_t>(ulRhs);
	return (*this);
}

NetIOStream& NetIOStream::operator>> (uint32_t& rhs)
{
	if (_ptrBufferOut + 4 > _ptrBufferIn)
		throw NetIOStreamException(NetIOStreamException::eOutOfBound);

	uint32_t	ulNetworkOrder = 0;
	memcpy(&ulNetworkOrder, GetBufferOut(), 4);
	rhs = ntohl(ulNetworkOrder);

	_ptrBufferOut += 4;

	return (*this);
}

NetIOStream& NetIOStream::operator>> (int64_t& rhs)
{
	uint64_t	ulRhs = 0;
	(*this) >> ulRhs;
	rhs = ulRhs;
	return (*this);
}

NetIOStream& NetIOStream::operator>> (uint64_t& rhs)
{
	if (_ptrBufferOut + 8 > _ptrBufferIn)
		throw NetIOStreamException(NetIOStreamException::eOutOfBound);

	uint64_t	int64_temp0;
	uint64_t	int64_temp1;

	memcpy(&int64_temp0, _ptrBufferOut, 8);
	int64_temp1 = int64_temp0;
	if ( 1 != ntohl(1) ) {
		uint32_t	higher = static_cast<uint32_t>((int64_temp0 & 0xFFFFFFFF00000000LL) >> 32);
		uint32_t	lower = static_cast<uint32_t>(int64_temp0 & 0x00000000FFFFFFFFLL);
		int64_temp1 = ( ((uint64_t)ntohl(lower) << 32) | ntohl(higher) );
	}
	rhs = int64_temp1;

	_ptrBufferOut += 8;

	return (*this);
}

NetIOStream& NetIOStream::operator>> (float& rhs)
{
	if (_ptrBufferOut + sizeof(float) > _ptrBufferIn)
		throw NetIOStreamException(NetIOStreamException::eOutOfBound);

	float float_temp0 = 0;
	float float_temp1 = 0;

	memcpy(&float_temp0, _ptrBufferOut, sizeof(float));
	float_temp1 = float_temp0;
	if ( 1 != ntohl(1) )
	{
		unsigned char* ptr_temp0 = (unsigned char *)&float_temp0;
		unsigned char* ptr_temp1 = (unsigned char *)&float_temp1;

		for (unsigned int i = 0; i < sizeof(float); i++)
			ptr_temp1[i] = ptr_temp0[sizeof(float) - 1 - i];
	}
	rhs = float_temp1;

	_ptrBufferOut += sizeof(float);

	return (*this);
}

NetIOStream& NetIOStream::operator>> (double& rhs)
{
	if (_ptrBufferOut + sizeof(double) > _ptrBufferIn)
		throw NetIOStreamException(NetIOStreamException::eOutOfBound);

	double double_temp0 = 0;
	double double_temp1 = 0;

	memcpy(&double_temp0, _ptrBufferOut, sizeof(double));
	double_temp1 = double_temp0;
	if ( 1 != ntohl(1) )
	{
		unsigned char* ptr_temp0 = (unsigned char *)&double_temp0;
		unsigned char* ptr_temp1 = (unsigned char *)&double_temp1;

		for (unsigned int i = 0; i < sizeof(double); i++)
			ptr_temp1[i] = ptr_temp0[sizeof(double) - 1 - i];
	}
	rhs = double_temp1;

	_ptrBufferOut += sizeof(double);

	return (*this);
}

NetIOStream& NetIOStream::operator>> (bool& rhs)
{
	uint32_t	ulRhs = 0;
	(*this) >> ulRhs;
	rhs = (ulRhs != 0);
	return (*this);
}

NetIOStream& NetIOStream::operator>> (NetIOStreamInterface* rhs)
{
	rhs->NetIOStreamIn((*this));
	return (*this);
}

NetIOStream& NetIOStream::operator>> (std::string& rhs)
{
	int iLengthNullTerminateString = 0;
	(*this) >> iLengthNullTerminateString;

	if ( _ptrBufferOut[iLengthNullTerminateString-1] != '\0' )
		throw NetIOStreamException(NetIOStreamException::eOutOfBound);

	rhs = _ptrBufferOut;
	_ptrBufferOut += iLengthNullTerminateString;

	return (*this);
}

NetIOStream& NetIOStream::SerializeInTime (time_t& rhs)
{
	uint32_t	ulRhs = 0;
	(*this) >> ulRhs;
	rhs = static_cast<time_t>(ulRhs);
	return (*this);
}

NetIOStream& NetIOStream::push(const char* byte, int size)
{
	(*this) << size;

	PrepareBuffer(size);
	memcpy(GetBufferIn(), byte, size);
	(*this) += size;

	return (*this);
}

// byte에 copy되서 나오고, size가 나옴, new 되는 건 아님
NetIOStream& NetIOStream::pop(char* byte, int& size)
{
	(*this) >> size;

	memcpy(byte, GetBufferOut(), size);
	_ptrBufferOut += size;

	return (*this);
}

// byte에 new & copy되서 나오고, size가 나옴
NetIOStream& NetIOStream::create_pop(char*& byte, int& size)
{
	(*this) >> size;
	byte = new char[size];
	memcpy(byte, GetBufferOut(), size);
	_ptrBufferOut += size;

	return (*this);
}

