// NetIOStreamInterface.h: interface for the CNetIOStreamInterface class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NetIOStreamInterface_H__0FE35530_44C2_4DA6_A963_6A2255F219E8__INCLUDED_)
#define AFX_NetIOStreamInterface_H__0FE35530_44C2_4DA6_A963_6A2255F219E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace CiUtils
{
	class NetIOStream;
}

namespace CiUtils
{
class NetIOStreamInterface
{
public:
	NetIOStreamInterface()	{}
	virtual ~NetIOStreamInterface()	{}
	virtual bool NetIOStreamOut(NetIOStream& niostream) = 0;
	virtual bool NetIOStreamIn(NetIOStream& niostream) = 0;
};
}


#endif // !defined(AFX_NetIOStreamInterface_H__0FE35530_44C2_4DA6_A963_6A2255F219E8__INCLUDED_)
