- [2007-03-29] CiSocketError.h

bsd/win socket ���� �ڵ��� �̽ļ��� ���� ���

�Ʒ� �ڵ��,

#ifdef _WIN32
if ( errno == WSAEWOULDBLOCK ) {
#else
if ( errno == EWOULDBLOCK ) {
#endif

�Ʒ� �ڵ�� ��ü�� �� �ִ�.

if ( errno == CI_EWOULDBLOCK ) {

