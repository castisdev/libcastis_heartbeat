#ifndef __CIUTILS_H__
#define __CIUTILS_H__

#include <string>
#include <boost/filesystem/path.hpp>

/* make sure the string ends with "/" or "\" */
bool CiUtils_MakeDirectoryString(char *pszResult,
								 int iNResultBufferLength,
								 const char *pszSource);

//	UUID 생성
std::string CiUtils_CreateUUID();

//	입력된 2개의 path가 동일한지 비교
bool CiUtils_EquivalentPath(const boost::filesystem::path& _a, const boost::filesystem::path& _b);

//	64bit longlong을 string으로 변경
std::string CiUtils_LLongToString(const long long& _num);
//	string을 64bit longlong으로 변경
long long CiUtils_StringToLLong(const char* _num);

//	file copy, move, remove
bool CiUtils_MoveFile(const boost::filesystem::path& _source, const boost::filesystem::path& _target, std::string& _err);
bool CiUtils_CopyFile(const boost::filesystem::path& _source, const boost::filesystem::path& _target, std::string& _err);
bool CiUtils_RemoveFile(const boost::filesystem::path& _filePath, std::string& _err);


/* get system informations */

bool CiUtils_GetDiskUsage(const char *pszPath, long long *pllTotalSpace, long long *pllAvailableSpace); /* use 'statfs' */

// pszFileSystemPathOut과 pszMountPointOut은 CI_MAX_NAME_LENGTH 사이즈의 char 배열이어야 한다.
// 리눅스 버전에서만 현재 구현되어 있다.
bool CiUtils_GetDiskInfo(const char *pszPath, char *pszFileSystemPathOut, char *pszMountPointOut);

bool CiUtils_GetMemInfo(long long *pllTotalMem, long long *pllFreeMem);	/* use 'free' command */
bool CiUtils_GetNicInfo(int *piNAllNicCount, int *piNDownNicCount);		/* use 'ethtool' command */


// CPU 정보를 얻기위해서는 다음과 같이 사용합니다.
// CiCpu_Info* pInfo = CiUtils_GetCPUInfo_CreateInstance();
// CiUtils_GetCPUInfo_UsagePercent(pInfo);
// CiUtils_GetCPUInfo_DeleteInstance(pInfo);
// CiUtils_GetCPUInfo_CreateInstance() 이후 바로 CiUtils_GetCPUInfo_UsagePercent() 를 호출하면
// 99 % usage 가 나올 수 있습니다.
// 이후 해당 객체를 이용하여 계속해서 CiUtils_GetCPUInfo_UsagePercent() 호출하면
// 정상적인 값이 반환됩니다.
class CiCpu_Info;
long			CiUtils_GetCPUInfo_UsagePercent(CiCpu_Info* CiInfo);
bool			CiUtils_GetCPUInfo_DeleteInstance(CiCpu_Info* CiInfo);
CiCpu_Info*		CiUtils_GetCPUInfo_CreateInstance();

bool			CiUtils_AddAliasIP(const char *pszNICName, int iAliasID, const char *pszIPAddress, const char *pszNetmask, const char *pszBroadcastIP, const char *pszGateway);
bool			CiUtils_DeleteAliasIP(const char *pszNICName, int iAliasID, const char *pszIPAddress);

int				CiUtils_GetProcessPID(const char *pszProcessName);

void			CiUtils_PrintWithTime(FILE *stream, char *pszString);

#endif
