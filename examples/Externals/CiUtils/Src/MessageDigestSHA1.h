// MessageDigestSHA1.h: interface for the MessageDigestSHA1 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MESSAGEDIGESTSHA1_H__674C8013_FB95_4FA8_A606_9D12BC4727EB__INCLUDED_)
#define AFX_MESSAGEDIGESTSHA1_H__674C8013_FB95_4FA8_A606_9D12BC4727EB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MessageDigest.h"

namespace castis
{
namespace security
{
	class MessageDigestSHA1 : public MessageDigest  
	{
	public:
		MessageDigestSHA1() {}

		MessageDigestSHA1(const MessageDigestSHA1& rhs) : MessageDigest(rhs) {}

		virtual ~MessageDigestSHA1() {}

		std::vector<unsigned char> Digest() const;

		std::string GetAlgorithm() const { return "SHA1"; }

		unsigned int GetDigestLength() const { return 20; }

		MessageDigest* Clone() const { return new MessageDigestSHA1(*this); }
	};
}
}

#endif // !defined(AFX_MESSAGEDIGESTSHA1_H__674C8013_FB95_4FA8_A606_9D12BC4727EB__INCLUDED_)
