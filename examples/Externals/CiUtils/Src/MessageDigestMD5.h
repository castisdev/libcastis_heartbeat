#ifndef __MESSAGE_DIGEST_MD5_H__
#define __MESSAGE_DIGEST_MD5_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MessageDigest.h"

namespace castis
{
namespace security
{
	class MessageDigestMD5 : public MessageDigest  
	{
	public:
		MessageDigestMD5() {}

		MessageDigestMD5(const MessageDigestMD5& rhs) : MessageDigest(rhs) {}

		virtual ~MessageDigestMD5() {}

		std::vector<unsigned char> Digest() const;

		std::string GetAlgorithm() const { return "MD5"; }

		unsigned int GetDigestLength() const { return 16; }

		MessageDigest* Clone() const { return new MessageDigestMD5(*this); }
	};
}
}
#endif
