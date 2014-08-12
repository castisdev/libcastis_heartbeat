// MessageDigest.cpp: implementation of the MessageDigest class.
//
//////////////////////////////////////////////////////////////////////

#include "internal_CiUtils.h"
#include "MessageDigest.h"

#include "MessageDigestSHA1.h"
#include "MessageDigestMD5.h"
#include <sstream>

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

using std::ostringstream;

namespace castis
{
namespace security
{
	MessageDigest* MessageDigest::GetInstance(const std::string& algorithm)
	{
		if (algorithm == "SHA1" || algorithm == "SHA")
			return new MessageDigestSHA1;
		else if (algorithm == "MD5")
			return new MessageDigestMD5;

		return NULL;
	}

	std::string MessageDigest::DigestToString(std::vector<unsigned char>& digest)
	{
		ostringstream os;

		for (std::vector<unsigned char>::iterator itr = digest.begin(); itr != digest.end(); ++itr)
			os << (int)(char)*itr;

		return os.str();
	}
}
}
