// NetIOStream.h: interface for the NetIOStream class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NetIOStream_H__F75457F8_0582_457D_6EFE72E2F15B__INCLUDED_)
#define AFX_NetIOStream_H__F75457F8_0582_457D_6EFE72E2F15B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <exception>
#include <vector>

namespace CiUtils
{
	class NetIOStreamInterface;
}

namespace CiUtils
{
class NetIOStreamException: public std::exception
{
public:
	enum { eOutOfMemory, eOutOfBound };
	NetIOStreamException(int type)
	{
		_type = type;
	}
	int GetType() const { return _type; }

	int _type;
};
}

namespace CiUtils
{
class NetIOStream
{
public:
	NetIOStream(const char* buffer, int size);
	NetIOStream(int reserveSize=32);
	virtual ~NetIOStream();


	// throw exception
	NetIOStream& operator<< (int8_t rhs);
	NetIOStream& operator<< (uint8_t rhs);
	NetIOStream& operator<< (int16_t rhs);
	NetIOStream& operator<< (uint16_t rhs);
	NetIOStream& operator<< (int32_t rhs);
	NetIOStream& operator<< (uint32_t rhs);
#ifdef INT32_IS_NOT_INT
	NetIOStream& operator<< (int rhs) { return (*this) << static_cast<uint32_t>(rhs); }
	NetIOStream& operator<< (unsigned rhs) { return (*this) << static_cast<uint32_t>(rhs); }
#endif
	NetIOStream& operator<< (int64_t rhs);
	NetIOStream& operator<< (uint64_t rhs);
#ifdef INT64_IS_NOT_LONGLONG
	NetIOStream& operator<< (long long rhs) { return (*this) << static_cast<uint64_t>(rhs); }
	NetIOStream& operator<< (unsigned long long rhs) { return (*this) << static_cast<uint64_t>(rhs); }
#endif
	NetIOStream& operator<< (float rhs);
	NetIOStream& operator<< (double rhs);
	NetIOStream& operator<< (bool rhs);
	NetIOStream& operator<< (NetIOStreamInterface* rhs);
	NetIOStream& operator<< (const std::string& rhs);
	NetIOStream& operator<< (NetIOStream& rhs);
	// NetIOStream& operator<< (time_t rhs);   DEPRECATED. Use 'SerializeOutTime()' instead.
	NetIOStream& SerializeOutTime (time_t rhs) { return (*this) << static_cast<uint32_t>(rhs); }


	template<typename T> NetIOStream& operator<< (const std::vector<T>& rhs)
	{
		(*this) << static_cast<const unsigned int>(rhs.size());
		for (typename std::vector<T>::const_iterator itr = rhs.begin(); itr != rhs.end(); ++itr)
			(*this) << *itr;
		return *this;
	}

	NetIOStream& operator>> (int8_t& rhs);
	NetIOStream& operator>> (uint8_t& rhs);
	NetIOStream& operator>> (int16_t& rhs);
	NetIOStream& operator>> (uint16_t& rhs);
	NetIOStream& operator>> (int32_t& rhs);
	NetIOStream& operator>> (uint32_t& rhs);
#ifdef INT32_IS_NOT_INT
	NetIOStream& operator>> (int& rhs) { return (*this) >> (uint32_t&)rhs; }
	NetIOStream& operator>> (unsigned& rhs) { return (*this) >> (uint32_t&)rhs; }
#endif
	NetIOStream& operator>> (int64_t& rhs);
	NetIOStream& operator>> (uint64_t& rhs);
#ifdef INT64_IS_NOT_LONGLONG
	NetIOStream& operator>> (long long& rhs) { return (*this) >> (uint64_t&)rhs; }
	NetIOStream& operator>> (unsigned long long& rhs) { return (*this) >> (uint64_t&)rhs; }
#endif
	NetIOStream& operator>> (float& rhs);
	NetIOStream& operator>> (double& rhs);
	NetIOStream& operator>> (bool& rhs);
	NetIOStream& operator>> (NetIOStreamInterface* rhs);
	NetIOStream& operator>> (std::string& rhs);
	// NetIOStream& operator>> (time_t& rhs);	DEPRECATED. Use 'SerializeInTime()' instead.
	NetIOStream& SerializeInTime (time_t& rhs);

	template<typename T> NetIOStream& operator>> (std::vector<T>& rhs)
	{
		size_t size = 0; (*this) >> size;
		for (size_t i = 0; i < size; ++i)
		{
			T temp; (*this) >> temp;
			rhs.push_back(temp);
		}
		return *this;
	}

	NetIOStream& push(const char* byte, int size);
	NetIOStream& pop(char* byte, int& size); // byte에 copy되서 나오고, size가 나옴, new 되는 건 아님
	NetIOStream& create_pop(char*& byte, int& size);// byte에 new & copy되서 나오고, size가 나옴

	char* GetBuffer() const { return _buffer;	} // return buffer pointer
	int GetBufferSize() { return static_cast<int>(_ptrBufferIn - _buffer); } // current out buffer size

	char* CreateBuffer(); // copy buffer
	std::string str(); // convert out buffer to string : 정확하게 type check를 해서 내보는게 아니다

protected:
	void PrepareBuffer(int size); // throw
	char* GetBufferIn() const { return _ptrBufferIn; }
	char* GetBufferOut() const { return _ptrBufferOut; }
	NetIOStream& operator+= (int size); // throw
	NetIOStream& operator++(); // throw


protected:
	int _bufferSize; // 내부적으로 할당된 버퍼 사이즈

	char* _ptrBufferIn; // input pointer
	char* _ptrBufferOut; // output pointer

	char* _buffer; // inside buffer
};

template<> inline NetIOStream& NetIOStream::operator<< (const std::vector<unsigned char>& rhs)
{
	(*this) << static_cast<uint32_t>(rhs.size());
	PrepareBuffer(static_cast<int>(rhs.size()));
	memcpy(GetBufferIn(), &rhs[0], rhs.size());
	(*this) += static_cast<int>(rhs.size());
	return (*this);
}

template<> inline NetIOStream& NetIOStream::operator<< (const std::vector<char>& rhs)
{
	(*this) << static_cast<uint32_t>(rhs.size());
	PrepareBuffer(static_cast<int>(rhs.size()));
	memcpy(GetBufferIn(), &rhs[0], rhs.size());
	(*this) += static_cast<int>(rhs.size());
	return (*this);
}

template<> inline NetIOStream& NetIOStream::operator>> <unsigned char>(std::vector<unsigned char>& rhs)
{
	int iLengthVector = 0;
	(*this) >> iLengthVector;

	if (_ptrBufferOut + iLengthVector > _ptrBufferIn)
		throw NetIOStreamException(NetIOStreamException::eOutOfBound);

	rhs.resize(iLengthVector);
	memcpy(&rhs[0], _ptrBufferOut, iLengthVector);

	_ptrBufferOut += iLengthVector;

	return (*this);
}

template<> inline NetIOStream& NetIOStream::operator>> (std::vector<char>& rhs)
{
	int iLengthVector = 0;
	(*this) >> iLengthVector;

	if (_ptrBufferOut + iLengthVector > _ptrBufferIn)
		throw NetIOStreamException(NetIOStreamException::eOutOfBound);

	rhs.resize(iLengthVector);
	memcpy(&rhs[0], _ptrBufferOut, iLengthVector);

	_ptrBufferOut += iLengthVector;

	return (*this);
}

}	// namespace CiUtils


#endif // !defined(AFX_NetIOStream_H__F75457F8_0582_457D_6EFE72E2F15B__INCLUDED_)
