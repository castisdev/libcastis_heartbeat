#include "internal_CiUtils.h"
#include "MessageDigestMD5.h"

#ifdef _WIN32
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <wincrypt.h>
#else
#include <openssl/md5.h>
#endif

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

using namespace std;

namespace castis
{
namespace security
{
	std::vector<unsigned char> MessageDigestMD5::Digest() const
	{
		vector<unsigned char> digest(GetDigestLength());

		// mdk20000 091009 size가 0일 경우 CryptHashData()에서 에러발생
		// vs 7.1과 9.0의 차이같음
		if(_input.size() == 0)
			return digest;

#ifdef _WIN32
		HCRYPTPROV hprov;
		CryptAcquireContext(&hprov, 0, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);

		HCRYPTHASH hhash;
		CryptCreateHash(hprov, CALG_MD5, 0, 0, &hhash);

		CryptHashData(hhash, &_input[0], static_cast<DWORD>(_input.size()), 0);

		DWORD hashSize = 0;
		DWORD size = sizeof(DWORD);
		CryptGetHashParam(hhash, HP_HASHSIZE, reinterpret_cast<BYTE*>(&hashSize), &size, 0);

		ASSERT(GetDigestLength() == hashSize);

		CryptGetHashParam(hhash, HP_HASHVAL, &digest[0], &hashSize, 0);
		CryptDestroyHash(hhash);
		CryptReleaseContext(hprov, 0);
#else
		MD5(&_input[0], _input.size(), &digest[0]);
#endif
		return digest;
	}
}
}

