#include "internal_CiUtils.h"
#include "CiSafeString.h"
#include "CiUtils.h"
#include "MTime2.h"

#if !defined(_WIN32) || (_MSC_VER > 1200)
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/classification.hpp>
#endif

#include "CiLogger.h"

#ifndef _WIN32
#include <dirent.h>
#include <sys/vfs.h>
#else
#include <afxtempl.h>
#include <pdh.h>
#include <pdhmsg.h>
#include "perfmon.h"
#include <Iphlpapi.h>
#define PROCESSER_INFO_STRING _T("\\Processor(_Total)\\% Processor Time")
#define IP_ALIAS_ADDRESS_STRING "netsh interface ip add address \"%s\" %s %s gateway=%s gwmetric=%d"
#define IP_ALIAS_DELETE_STRING "netsh interface ip delete address \"%s\" %s"
#include <process.h>
#include <tlhelp32.h>
#endif

//	CiUtils_CreateUUID의 한 header파일
#ifdef WIN32
#pragma comment(lib, "Rpcrt4.lib")
#include <Rpc.h>
#else
#include <uuid/uuid.h>
#endif

#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/convenience.hpp>

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

using namespace std;

/* make sure the string ends with "/" or "\" */
bool CiUtils_MakeDirectoryString(char *pszResult,
								 int iNResultBufferLength,
								 const char *pszSource)
{
	if ( pszSource == NULL || pszResult == NULL ) {
		return false;
	}

	int iNTriedToCreate;
	CiStrCpy(pszResult, pszSource, iNResultBufferLength, &iNTriedToCreate);

	if ( iNTriedToCreate >= iNResultBufferLength ) {
		/* undesirable truncation */
		return false;
	}
	else if ( iNTriedToCreate == iNResultBufferLength - 1 ) {
		if ( pszResult[iNTriedToCreate-1] == DIRECTORY_DELIMITER ) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		if ( pszResult[iNTriedToCreate-1] != DIRECTORY_DELIMITER ) {
			pszResult[iNTriedToCreate] = DIRECTORY_DELIMITER;
			pszResult[iNTriedToCreate+1] = '\0';
		}

		return true;
	}
}

std::string CiUtils_CreateUUID()
{
	char buf[64] = {0};

#ifdef WIN32
	UUID uuid;
	UuidCreate(&uuid);
	_snprintf(buf, sizeof(buf), "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
		uuid.Data1, uuid.Data2, uuid.Data3, uuid.Data4[0], uuid.Data4[1], uuid.Data4[2], uuid.Data4[3], uuid.Data4[4], uuid.Data4[5], uuid.Data4[6], uuid.Data4[7]);
#else
	uuid_t uuid;
	uuid_generate(uuid);
	uuid_unparse(uuid, buf);
#endif

	return std::string(buf);
}

bool CiUtils_EquivalentPath(const boost::filesystem::path& _a, const boost::filesystem::path& _b)
{
	std::string aa = _a.string();
	std::string bb = _b.string();

	if ( !aa.empty() && (aa[aa.length() - 1] == '/' || aa[aa.length() - 1] == '\\') )
		aa = aa.substr(0, aa.length() - 1);

	if ( !bb.empty() && (bb[bb.length() - 1] == '/' || bb[bb.length() - 1] == '\\') )
		bb = bb.substr(0, bb.length() - 1);

	return aa == bb;
}

std::string CiUtils_LLongToString(const long long& _num)
{
	char a[32] = "";

#ifdef _WIN32
	sprintf(a, "%I64d", _num);
#else
	sprintf(a, "%lld", _num);
#endif
	string num(a);

	return num;
}

long long CiUtils_StringToLLong(const char* _num)
{
	int numsize = static_cast<int>(strlen(_num));
	if(numsize > 8)
	{
		//	max 20자리까지 복사를 한다.
		char tempnum[21];
		memcpy(tempnum, _num, numsize);
		tempnum[numsize] = 0;

		int lowval = atoi(&tempnum[numsize-8]);
		tempnum[numsize-8] = 0;

		return static_cast<long long>(atoi(tempnum)) * 100000000LL + lowval;
	}
	else
	{
		return atoi(_num);
	}
}

bool CiUtils_MoveFile(const boost::filesystem::path& _source, const boost::filesystem::path& _target, std::string& _err)
{
	bool ret = true;
	try
	{
		if( exists(_target) )
			remove_all(_target);

		if( exists(_source) == false )
		{
			_err = "not exist the source file";
			return false;
		}

		if( _source.root_name() == _target.root_name() )
			rename(_source, _target);
		else
		{
			copy_file(_source, _target);
			remove_all(_source);
		}
	}
	catch (boost::filesystem::filesystem_error& e)
	{
		_err = e.what();
		ret = false;
	}
	return ret;
}

bool CiUtils_CopyFile(const boost::filesystem::path& _source, const boost::filesystem::path& _target, std::string& _err)
{
	bool ret = true;
	try
	{
		if ( exists(_target) )
			remove_all(_target);

		if( exists(_source) == false )
		{
			_err = "not exist the source file";
			return false;
		}

		copy_file(_source, _target);
	}
	catch (boost::filesystem::filesystem_error& e)
	{
		_err = e.what();
		ret = false;
	}
	return ret;
}

bool CiUtils_RemoveFile(const boost::filesystem::path& _filePath, std::string& _err)
{
	bool ret = true;
	try
	{
		if ( exists(_filePath) )
			remove_all(_filePath);
	}
	catch (boost::filesystem::filesystem_error& e)
	{
		_err = e.what();
		ret = false;
	}
	return ret;
}


/* CiUtils_GetDiskUsage
 * get total disk space & available disk space
 */
bool CiUtils_GetDiskUsage(const char *pszPath,
							   long long *pllTotalSpace,
							   long long *pllAvailableSpace)
{
	if ( pszPath == NULL || pllTotalSpace == NULL || pllAvailableSpace == NULL )
		return false;

#ifdef _WIN32

	ULARGE_INTEGER ulAvailable, ulTotal, ulFree;
	if ( GetDiskFreeSpaceEx((LPCTSTR)pszPath, &ulAvailable, &ulTotal, &ulFree) == 0 )
	{
		return false;
	}

	*pllTotalSpace = (long long)ulTotal.QuadPart;
	*pllAvailableSpace = (long long)ulAvailable.QuadPart;

	return true;

#else

	struct statfs s_status_fs;
	if ( statfs(pszPath, &s_status_fs) < 0 )
	{
		//printf("CiUtils_GetDiskUsage2 :: statfs (%s) fail (%s)\n", pszPath, strerror(errno));
		return false;
	}

	*pllTotalSpace = (long long)(s_status_fs.f_blocks) * (long long)(s_status_fs.f_bsize);
	*pllAvailableSpace = (long long)(s_status_fs.f_bavail) * (long long)(s_status_fs.f_bsize);

	return true;

#endif

}

/* CiUtils_GetDiskInfo
 * get file system path & mount point
 * NOTE : no Win32 Version
 */
bool CiUtils_GetDiskInfo(const char *pszPath,
							  char *pszFileSystemPathOut,
							  char *pszMountPointOut)
{
	if ( pszPath == NULL || pszFileSystemPathOut == NULL || pszMountPointOut == NULL )
		return false;

#ifndef _WIN32

	/* using 'df' command */

	char szCommandString[3+CI_MAX_FILE_PATH_LENGTH+1];
	char szResultFile[CI_MAX_FILE_NAME_LENGTH];
	int iNTriedToCreate;

	CiStrCpy(szResultFile, "df_result", CI_MAX_FILE_NAME_LENGTH, &iNTriedToCreate);
	snprintf(szCommandString, 3+CI_MAX_FILE_PATH_LENGTH,
				"%s %s > %s", "df", pszPath, szResultFile);

	int ret = system(szCommandString);
	if ( ret < 0 )
	{
		//printf("failed to system '%s' (%s)\n", szCommandString, strerror(errno));
		remove(szResultFile);
		return false;
	}

	/* read result file and get disk usage.
	 * followings are the example of result file :
	 *		Filesystem           1K-blocks      Used Available Use% Mounted on
	 *		/dev/cciss/c0d1p2     64916716  60767432    851624  99% /data
	 * open the result file and read 2nd line.
	 */

	FILE *fp = fopen(szResultFile, "r");
	if ( fp == NULL )
	{
		remove(szResultFile);
		return false;
	}

	char buf[256];
	for ( int i = 0; i < 2; i ++ )
	{
		if ( fgets(buf, 256, fp) == NULL )
		{
			fclose(fp);
			remove(szResultFile);
			return false;
		}
	}
	fclose(fp);

	/* get disk usage from BUF. each attribute is delimited by '\t' */
	char szFileSystemPath[CI_MAX_FILE_PATH_LENGTH];
	char szMountPoint[CI_MAX_FILE_PATH_LENGTH];
	char szUsedPercent[CI_MAX_FILE_PATH_LENGTH];
	long long llTotalSpace, llUsedSpace, llAvailSpace;
	sscanf(buf, "%s	%lld	%lld	%lld	%s	%s\n",
		szFileSystemPath, &llTotalSpace, &llUsedSpace, &llAvailSpace, szUsedPercent, szMountPoint);

	CiStrCpy(pszFileSystemPathOut, szFileSystemPath, CI_MAX_FILE_PATH_LENGTH+1, &iNTriedToCreate);
	CiStrCpy(pszMountPointOut, szMountPoint, CI_MAX_FILE_PATH_LENGTH+1, &iNTriedToCreate);

	remove(szResultFile);

#endif

	return true;
}

/* CiUtils_GetCPUInfo
 * get cpu info (user time, nice time, system time, idle time)
 */
class CiCpu_Info
{
public:
	CiCpu_Info();
	~CiCpu_Info();

public:
#ifdef _WIN32
	std::auto_ptr<CPerfMon> _perfMon;
#else
	long long _llAllCPUTime;
	long long _llIdleTime;
#endif
};

CiCpu_Info::CiCpu_Info() :
#ifdef _WIN32
_perfMon(new CPerfMon)
#else
_llAllCPUTime(0),
_llIdleTime(0)
#endif
{
#ifdef _WIN32
	_perfMon->Initialize();
	_perfMon->AddCounter(PROCESSER_INFO_STRING);
#endif
}

CiCpu_Info::~CiCpu_Info()
{
#ifdef _WIN32
	if ( _perfMon.get() != NULL )
	{
		_perfMon->Uninitialize();
	}
#endif
}

long CiUtils_GetCPUInfo_UsagePercent(CiCpu_Info* CiInfo)
{
	if(CiInfo == NULL)
		return -1;

#ifdef _WIN32
	int cpu = 0;
	int nCPU = 0;
	if (!CiInfo->_perfMon->CollectQueryData())
	{
		return -1;
	}
	cpu = CiInfo->_perfMon->GetCounterValue(nCPU);
	return cpu;
#else
	FILE *fp = fopen("/proc/stat", "r");
	if ( fp == NULL )
	{
		return -1;
	}

	char buf[256];
	char *ptr = NULL;
	while ( fgets(buf, 256, fp) != NULL )
	{
		if ( (ptr = strstr(buf, "cpu")) != NULL )
			break;
	}
	fclose(fp);

	if ( ptr == NULL )
	{
		return -1;
	}

	ptr += CiStrLen("cpu");
	while( isspace(*ptr) )
		ptr++;

	CiLongLong_t llUserTime = 0, llNiceTime = 0, llSystemTime = 0, llIdleTime = 0;
	if ( sscanf(ptr, "%lld %lld %lld %lld", &llUserTime, &llNiceTime, &llSystemTime, &llIdleTime) != 4 )
	{
		return -1;
	}
	CiLongLong_t llAllCPUTime = llUserTime + llNiceTime + llSystemTime + llIdleTime;
	double dIdlePercent = 0.0;
	unsigned int uiCpuUsagePercent = 0;
	if ( llAllCPUTime > CiInfo->_llAllCPUTime )
	{
		dIdlePercent = (double)(llIdleTime - CiInfo->_llIdleTime)/(double)(llAllCPUTime - CiInfo->_llAllCPUTime) * 100;
		uiCpuUsagePercent = 100 - (unsigned int)dIdlePercent;
		CiInfo->_llAllCPUTime = llAllCPUTime;
		CiInfo->_llIdleTime = llIdleTime;
	}
	return 	uiCpuUsagePercent;
#endif
}

CiCpu_Info* CiUtils_GetCPUInfo_CreateInstance()
{
	return new CiCpu_Info;
}

bool CiUtils_GetCPUInfo_DeleteInstance(CiCpu_Info* CiInfo)
{
	if(CiInfo == NULL)
		return false;

	delete CiInfo;
	return true;
}

bool CiUtils_GetMemInfo(long long *pllTotalMem,
							 long long *pllFreeMem)
{
#ifdef _WIN32
	MEMORYSTATUS ms;
	memset(&ms, 0, sizeof(ms) );
	ms.dwLength = sizeof(ms);

	GlobalMemoryStatus(&ms);
	//linux 기준, MEMORYSTATUS는 byte 로 구성되어 1024 나눔 ->kbyte변경
	*pllTotalMem = ms.dwTotalPhys/1024;
	*pllFreeMem  = ms.dwAvailPhys/1024;

	return true;

#else

	long long llTotalMem = 0, llUsedMem = 0, llAvailMem = 0;

	char szResultFile[CI_MAX_FILE_PATH_LENGTH+1];
	char szCommandString[CI_MAX_FILE_PATH_LENGTH+21];

	int iNTriedToCreate;
	CiStrCpy(szResultFile, "/dev/shm/free_result", CI_MAX_FILE_NAME_LENGTH, &iNTriedToCreate);
	snprintf(szCommandString, CI_MAX_FILE_PATH_LENGTH+20,
				"%s > %s 2> /dev/null", "free", szResultFile);

	int ret = system(szCommandString);
	if ( ret < 0 )
	{
		//printf("failed to system '%s' (%s)\n", szCommandString, strerror(errno));
		return false;
	}

	/* read result file and get available memory info.
	 * followings are the example of result file :
	 *
	 *		         total       used       free     shared    buffers     cached
	 *	Mem:        514268     508292       5976          0     202080     167316
	 *	-/+ buffers/cache:     138896     375372
	 *	Swap:      1004052       1044    1003008
	 *
	 * open the result file and read the last field of third line.
	 */

	FILE *fp = fopen(szResultFile, "r");
	if ( fp == NULL )
		return false;

	char buf[256];
	char *ptr = NULL;
	while ( fgets(buf, 256, fp) != NULL )
	{
		if ( (ptr = strstr(buf, "-/+ buffers/cache:")) != NULL )
		{
			/* found */
			break;
		}
	}
	fclose(fp);

	if ( ptr != NULL )
	{
		/* available memory info found */

		ptr += CiStrLen("-/+ buffers/cache:");
		while( isspace(*ptr) )
			ptr++;

		/* 'usedmem' tab 'availmem' */
		if ( sscanf(ptr, "%lld	%lld\n", &llUsedMem, &llAvailMem) < 2 )
			return false;

		llTotalMem = llUsedMem + llAvailMem;
	}

	remove(szResultFile);

	*pllTotalMem = llTotalMem;
	*pllFreeMem = llAvailMem;

	return true;

#endif
}

bool CiUtils_GetNicInfo(int *piNAllNicCount, int *piNDownNicCount)
{
#ifdef _WIN32
	IP_ADAPTER_INFO AdapterInfo[16];

	DWORD dwBufLen = sizeof(AdapterInfo);
	DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);

	if(dwStatus != ERROR_SUCCESS)
	{
		return false;
	}
	int iHavwinsCount=0;
	int iTotalCount=0;
	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
	while(pAdapterInfo)
	{
		if(AdapterInfo->HaveWins)
		{
			iHavwinsCount++;
		}
		iTotalCount++;
		pAdapterInfo = pAdapterInfo->Next;
	}
	*piNAllNicCount = iTotalCount;
	*piNDownNicCount = iTotalCount-iHavwinsCount;

	return true;

#else

	/* using 'ethtool' command, get network link info */

	int iNAllNicCount = 0, iNDownNicCount = 0;
	char szCommandString[3+CI_MAX_FILE_PATH_LENGTH+1];
	char szResultFile[CI_MAX_FILE_NAME_LENGTH];
	int iNTriedToCreate;
	int i;

	CiStrCpy(szResultFile, "/dev/shm/ethtool_result", CI_MAX_FILE_NAME_LENGTH, &iNTriedToCreate);

	for ( i = 1; i < 3; i++ )		/* only check eth1 & eth2 */
	{
		char szNicName[10];
		snprintf(szNicName, 9, "eth%d", i);
		snprintf(szCommandString, 3+CI_MAX_FILE_PATH_LENGTH,
					"%s %s > %s	2> /dev/null", "ethtool", szNicName, szResultFile);

		int ret = system(szCommandString);
		if ( ret < 0 )
		{
			//printf("failed to system '%s' (%s)\n", szCommandString, strerror(errno));
			continue;
		}

		/* read result file and get network link info.
		 * followings are the example of result file :
		 *		.....
		 *		Link detected: eth0
		 * open the result file and read (maybe) the last line.
		 */

		FILE *fp = fopen(szResultFile, "r");
		if ( fp == NULL )
			continue;

		char buf[256];
		char *ptr = NULL;
		while ( fgets(buf, 256, fp) != NULL )
		{
			if ( (ptr = strstr(buf, "Link detected:")) != NULL )
			{
				/* found */
				break;
			}
		}
		fclose(fp);

		if ( ptr != NULL )
		{
			/* link info found */
			iNAllNicCount++;

			ptr += CiStrLen("Link detected:");
			while( isspace(*ptr) )
				ptr++;

			char *ptr2 = NULL;
			if ( (ptr2 = strstr(ptr, "no")) != NULL )
			{
				/* link down */
				iNDownNicCount++;
			}
		}
	}

	remove(szResultFile);

	*piNAllNicCount = iNAllNicCount;
	*piNDownNicCount = iNDownNicCount;

	return true;

#endif
}

#ifdef _WIN32
bool CiUtils_AddAliasIP(const char *pszNICName, int iAliasID, const char *pszIPAddress, const char *pszNetmask, const char * /*pszBroadcastIP*/, const char *pszGateway)
{
	char smux[225] = { 0, };
	sprintf(smux, IP_ALIAS_ADDRESS_STRING, pszNICName, pszIPAddress, pszNetmask, pszGateway, iAliasID);
	system(smux);
	return true;
}
#else
bool CiUtils_AddAliasIP(const char *pszNICName, int iAliasID, const char *pszIPAddress, const char *pszNetmask, const char *pszBroadcastIP, const char *pszGateway)
{
	char pszCommand[256];

	if ( iAliasID > 0 )
		sprintf(pszCommand, "/sbin/ifconfig %s:%d %s netmask %s broadcast %s up", pszNICName, iAliasID, pszIPAddress, pszNetmask, pszBroadcastIP);
	else
		sprintf(pszCommand, "/sbin/ifconfig %s %s netmask %s broadcast %s up", pszNICName, pszIPAddress, pszNetmask, pszBroadcastIP);

	if ( system((const char*)pszCommand) < 0 )
	{
		return false;
	}

	sprintf(pszCommand, "/sbin/arping -fqbDUA -c 2 -I %s -s %s %s &", pszNICName, pszIPAddress, pszGateway);
	if ( system((const char*)pszCommand) < 0 )
	{
		return false;
	}


	return true;
}
#endif

#ifdef _WIN32
bool CiUtils_DeleteAliasIP(const char *pszNICName, int /*iAliasID*/, const char *pszIPAddress)
{
	char smux[225] = { 0, };
	sprintf(smux, IP_ALIAS_DELETE_STRING, pszNICName, pszIPAddress);
	system(smux);
	return true;
}
#else
bool CiUtils_DeleteAliasIP(const char *pszNICName, int iAliasID, const char *pszIPAddress)
{
	char pszCommand[256];

	if ( iAliasID > 0 )
		sprintf(pszCommand, "/sbin/ifconfig %s:%d down", pszNICName, iAliasID);
	else
		sprintf(pszCommand, "/sbin/ifconfig %s down", pszNICName);

	if ( system((const char*)pszCommand) < 0 )
	{
		return false;
	}


	return true;
}
#endif

int CiUtils_GetProcessPID(const char *pszProcessName)
{
#ifdef _WIN32
	PROCESSENTRY32 proc;
	proc.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if ( snapshot == INVALID_HANDLE_VALUE )
		return -1;

	Process32First(snapshot, &proc);
	char execFile[CI_MAX_FILE_NAME_LENGTH+1];
	execFile[CI_MAX_FILE_NAME_LENGTH] = '\0';

	do
	{
#ifdef _UNICODE
		WideCharToMultiByte(CP_ACP, 0, proc.szExeFile, -1, execFile, CI_MAX_FILE_NAME_LENGTH, NULL, NULL);
#else
		strncpy(execFile, proc.szExeFile, CI_MAX_FILE_NAME_LENGTH);
#endif
		if ( strcmp(execFile, pszProcessName) == 0 )
		{
			CloseHandle(snapshot);
			return proc.th32ProcessID;
		}
	} while ( TRUE == Process32Next(snapshot, &proc) );

	CloseHandle(snapshot);
	return -1;
#else
	int pid = -1;
	DIR *dir = NULL;
	struct dirent *dir_entry = NULL;
	char status_file[40];

	if ( (dir = opendir("/proc")) == NULL )
	{
		return pid;
	}

	while( (dir_entry = readdir(dir)) != NULL )
	{
		if ( strcmp(dir_entry->d_name, ".") && strcmp(dir_entry->d_name, "..") )	/* .과 .. 제외 */
		{
			sprintf(status_file, "/proc/%s/cmdline", dir_entry->d_name);
			if ( access(status_file, F_OK) < 0 )		/* status 파일 존재하는지 확인 */
				continue;

			/* 디렉토리 이름이 숫자인지 확인한다
			 * 디렉토리 이름이 숫자이면 /proc/[pid]/cmdline 파일을 열어
			 * 주어진 프로세스 이름과 일치하는지 확인한다
			 */
			if ( boost::algorithm::all(dir_entry->d_name, boost::algorithm::is_digit()) == true )
			{
				FILE *fp = fopen(status_file, "r");
				if ( fp == NULL )
					continue;

				char buf[256];
				if ( fgets(buf, 256, fp) == NULL )
				{
					fclose(fp);
					continue;
				}
				fclose(fp);

				/* cmdline 파일에서 얻어낸 스트링의 맨 끝이 pszProcessName과 일치하는가 */
				char *p = strstr(buf, pszProcessName);
				if ( p == NULL )
					continue;

				if ( !strcmp(p, pszProcessName) )
				{
					pid = atoi(dir_entry->d_name);
					break;
				}
			}
		}
	}

	if ( dir != NULL )
		closedir(dir);

	return pid;
#endif
}

void CiUtils_PrintWithTime(FILE *stream, char *pszString)
{
	CMTime2 mtCurrentTime;
	char *pszCurrentTime = NULL;
	mtCurrentTime.GetTime(&pszCurrentTime);

	fprintf(stream, "[%s]%s\n", pszCurrentTime, pszString);

	delete [] pszCurrentTime;
}
