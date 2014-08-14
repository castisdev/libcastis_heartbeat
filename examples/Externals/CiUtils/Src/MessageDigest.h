// MessageDigest.h: interface for the MessageDigest class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MESSAGEDIGEST_H__23507B8C_DA61_4273_B541_9028958B5E0E__INCLUDED_)
#define AFX_MESSAGEDIGEST_H__23507B8C_DA61_4273_B541_9028958B5E0E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>
#include <string>


namespace castis
{
namespace security
{
	class MessageDigest  
	{
	protected:
		MessageDigest() {}
		MessageDigest(const MessageDigest& rhs) : _input(rhs._input) {}

	public:
		virtual ~MessageDigest() {}

		static MessageDigest* GetInstance(const std::string& algorithm);

		virtual std::vector<unsigned char> Digest() const = 0;

		std::vector<unsigned char> Digest(const unsigned char* input, size_t len)
		{
			Update(input, len);
			return Digest();
		}

		std::vector<unsigned char> Digest(const unsigned char* input, int offset, size_t len)
		{
			Update(input, offset, len);
			return Digest();
		}

		std::vector<unsigned char> Digest(const char* input, size_t len)
		{
			return Digest((const unsigned char*)input, len);
		}

		std::vector<unsigned char> Digest(const char* input, int offset, size_t len)
		{
			return Digest((const unsigned char*)input, offset, len);
		}


		void Update(const unsigned char* input, size_t len)
		{
			_input.assign(input, input + len);
		}

		void Update(const unsigned char* input, int offset, size_t len)
		{
			_input.assign(input + offset, input + offset + len);
		}

		void Update(const char* input, size_t len)
		{
			Update((const unsigned char*)input, len);
		}

		void Update(const char* input, int offset, size_t len)
		{
			Update((const unsigned char*)input, offset, len);
		}


		virtual std::string GetAlgorithm() const = 0;

		virtual unsigned int GetDigestLength() const = 0;

		virtual MessageDigest* Clone() const = 0;

		static std::string DigestToString(std::vector<unsigned char>& digest);


	protected:
		std::vector<unsigned char> _input;
	};
}
}

#endif // !defined(AFX_MESSAGEDIGEST_H__23507B8C_DA61_4273_B541_9028958B5E0E__INCLUDED_)
