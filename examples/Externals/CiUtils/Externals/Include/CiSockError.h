#ifndef __CISOCKERROR_H__
#define __CISOCKERROR_H__

#ifdef _WIN32

#define CI_EINTR                   WSAEINTR
#define CI_EBADF                   WSAEBADF
#define CI_EACCES                  WSAEACCES
#define CI_EFAULT                  WSAEFAULT
#define CI_EINVAL                  WSAEINVAL
#define CI_EMFILE                  WSAEMFILE

#define CI_EWOULDBLOCK             WSAEWOULDBLOCK
#define CI_EINPROGRESS             WSAEINPROGRESS
#define CI_EALREADY                WSAEALREADY
#define CI_ENOTSOCK                WSAENOTSOCK
#define CI_EDESTADDRREQ            WSAEDESTADDRREQ
#define CI_EMSGSIZE                WSAEMSGSIZE
#define CI_EPROTOTYPE              WSAEPROTOTYPE
#define CI_ENOPROTOOPT             WSAENOPROTOOPT
#define CI_EPROTONOSUPPORT         WSAEPROTONOSUPPORT
#define CI_ESOCKTNOSUPPORT         WSAESOCKTNOSUPPORT
#define CI_EOPNOTSUPP              WSAEOPNOTSUPP
#define CI_EPFNOSUPPORT            WSAEPFNOSUPPORT
#define CI_EAFNOSUPPORT            WSAEAFNOSUPPORT
#define CI_EADDRINUSE              WSAEADDRINUSE
#define CI_EADDRNOTAVAIL           WSAEADDRNOTAVAIL
#define CI_ENETDOWN                WSAENETDOWN
#define CI_ENETUNREACH             WSAENETUNREACH
#define CI_ENETRESET               WSAENETRESET
#define CI_ECONNABORTED            WSAECONNABORTED
#define CI_ECONNRESET              WSAECONNRESET
#define CI_ENOBUFS                 WSAENOBUFS
#define CI_EISCONN                 WSAEISCONN
#define CI_ENOTCONN                WSAENOTCONN
#define CI_ESHUTDOWN               WSAESHUTDOWN
#define CI_ETOOMANYREFS            WSAETOOMANYREFS
#define CI_ETIMEDOUT               WSAETIMEDOUT
#define CI_ECONNREFUSED            WSAECONNREFUSED
#define CI_ELOOP                   WSAELOOP
#define CI_ENAMETOOLONG            WSAENAMETOOLONG
#define CI_EHOSTDOWN               WSAEHOSTDOWN
#define CI_EHOSTUNREACH            WSAEHOSTUNREACH
#define CI_ENOTEMPTY               WSAENOTEMPTY
#define CI_EPROCLIM                WSAEPROCLIM
#define CI_EUSERS                  WSAEUSERS
#define CI_EDQUOT                  WSAEDQUOT
#define CI_ESTALE                  WSAESTALE
#define CI_EREMOTE                 WSAEREMOTE

#else

#define CI_EINTR                   EINTR
#define CI_EBADF                   EBADF
#define CI_EACCES                  EACCES
#define CI_EFAULT                  EFAULT
#define CI_EINVAL                  EINVAL
#define CI_EMFILE                  EMFILE

#define CI_EWOULDBLOCK             EWOULDBLOCK
#define CI_EINPROGRESS             EINPROGRESS
#define CI_EALREADY                EALREADY
#define CI_ENOTSOCK                ENOTSOCK
#define CI_EDESTADDRREQ            EDESTADDRREQ
#define CI_EMSGSIZE                EMSGSIZE
#define CI_EPROTOTYPE              EPROTOTYPE
#define CI_ENOPROTOOPT             ENOPROTOOPT
#define CI_EPROTONOSUPPORT         EPROTONOSUPPORT
#define CI_ESOCKTNOSUPPORT         ESOCKTNOSUPPORT
#define CI_EOPNOTSUPP              EOPNOTSUPP
#define CI_EPFNOSUPPORT            EPFNOSUPPORT
#define CI_EAFNOSUPPORT            EAFNOSUPPORT
#define CI_EADDRINUSE              EADDRINUSE
#define CI_EADDRNOTAVAIL           EADDRNOTAVAIL
#define CI_ENETDOWN                ENETDOWN
#define CI_ENETUNREACH             ENETUNREACH
#define CI_ENETRESET               ENETRESET
#define CI_ECONNABORTED            ECONNABORTED
#define CI_ECONNRESET              ECONNRESET
#define CI_ENOBUFS                 ENOBUFS
#define CI_EISCONN                 EISCONN
#define CI_ENOTCONN                ENOTCONN
#define CI_ESHUTDOWN               ESHUTDOWN
#define CI_ETOOMANYREFS            ETOOMANYREFS
#define CI_ETIMEDOUT               ETIMEDOUT
#define CI_ECONNREFUSED            ECONNREFUSED
#define CI_ELOOP                   ELOOP
#define CI_ENAMETOOLONG            ENAMETOOLONG
#define CI_EHOSTDOWN               EHOSTDOWN
#define CI_EHOSTUNREACH            EHOSTUNREACH
#define CI_ENOTEMPTY               ENOTEMPTY
#define CI_EPROCLIM                EPROCLIM
#define CI_EUSERS                  EUSERS
#define CI_EDQUOT                  EDQUOT
#define CI_ESTALE                  ESTALE
#define CI_EREMOTE                 EREMOTE

#endif

#endif
