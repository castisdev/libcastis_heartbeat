- [2007-03-29] CiSocketError.h

bsd/win socket 에러 코드의 이식성을 위한 헤더

아래 코드는,

#ifdef _WIN32
if ( errno == WSAEWOULDBLOCK ) {
#else
if ( errno == EWOULDBLOCK ) {
#endif

아래 코드로 대체될 수 있다.

if ( errno == CI_EWOULDBLOCK ) {

