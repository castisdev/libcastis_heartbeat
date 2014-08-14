#ifndef __CIUTILS_H__
#define __CIUTILS_H__

#include <string>
#include <boost/filesystem/path.hpp>

/* make sure the string ends with "/" or "\" */
bool CiUtils_MakeDirectoryString(char *pszResult,
								 int iNResultBufferLength,
								 const char *pszSource);

//	UUID ����
std::string CiUtils_CreateUUID();

//	�Էµ� 2���� path�� �������� ��
bool CiUtils_EquivalentPath(const boost::filesystem::path& _a, const boost::filesystem::path& _b);

//	64bit longlong�� string���� ����
std::string CiUtils_LLongToString(const long long& _num);
//	string�� 64bit longlong���� ����
long long CiUtils_StringToLLong(const char* _num);

//	file copy, move, remove
bool CiUtils_MoveFile(const boost::filesystem::path& _source, const boost::filesystem::path& _target, std::string& _err);
bool CiUtils_CopyFile(const boost::filesystem::path& _source, const boost::filesystem::path& _target, std::string& _err);
bool CiUtils_RemoveFile(const boost::filesystem::path& _filePath, std::string& _err);


/* get system informations */

bool CiUtils_GetDiskUsage(const char *pszPath, long long *pllTotalSpace, long long *pllAvailableSpace); /* use 'statfs' */

// pszFileSystemPathOut�� pszMountPointOut�� CI_MAX_NAME_LENGTH �������� char �迭�̾�� �Ѵ�.
// ������ ���������� ���� �����Ǿ� �ִ�.
bool CiUtils_GetDiskInfo(const char *pszPath, char *pszFileSystemPathOut, char *pszMountPointOut);

bool CiUtils_GetMemInfo(long long *pllTotalMem, long long *pllFreeMem);	/* use 'free' command */
bool CiUtils_GetNicInfo(int *piNAllNicCount, int *piNDownNicCount);		/* use 'ethtool' command */


// CPU ������ ������ؼ��� ������ ���� ����մϴ�.
// CiCpu_Info* pInfo = CiUtils_GetCPUInfo_CreateInstance();
// CiUtils_GetCPUInfo_UsagePercent(pInfo);
// CiUtils_GetCPUInfo_DeleteInstance(pInfo);
// CiUtils_GetCPUInfo_CreateInstance() ���� �ٷ� CiUtils_GetCPUInfo_UsagePercent() �� ȣ���ϸ�
// 99 % usage �� ���� �� �ֽ��ϴ�.
// ���� �ش� ��ü�� �̿��Ͽ� ����ؼ� CiUtils_GetCPUInfo_UsagePercent() ȣ���ϸ�
// �������� ���� ��ȯ�˴ϴ�.
class CiCpu_Info;
long			CiUtils_GetCPUInfo_UsagePercent(CiCpu_Info* CiInfo);
bool			CiUtils_GetCPUInfo_DeleteInstance(CiCpu_Info* CiInfo);
CiCpu_Info*		CiUtils_GetCPUInfo_CreateInstance();

bool			CiUtils_AddAliasIP(const char *pszNICName, int iAliasID, const char *pszIPAddress, const char *pszNetmask, const char *pszBroadcastIP, const char *pszGateway);
bool			CiUtils_DeleteAliasIP(const char *pszNICName, int iAliasID, const char *pszIPAddress);

int				CiUtils_GetProcessPID(const char *pszProcessName);

void			CiUtils_PrintWithTime(FILE *stream, char *pszString);

#endif
