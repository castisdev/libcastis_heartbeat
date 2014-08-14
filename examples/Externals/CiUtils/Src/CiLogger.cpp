// CiLogger.cpp: implementation of the CiLogger class.
//
// 2005-11-23
//	LookupLatestLogFile() 함수에서 extension 을 xml 만 찾게 되어있던 것 수정
//	-> SenEnv에서 설정한 걸로 ext를 찾게 수정함
// 2005-04-18
//	Thread safe 하게 수정됨
//	interface 가 바뀜
//	milli sec 추가
//	message size 는 default 1024 한글 512
//
//	const char* -> char* 로 바꾸었음 ( const char* 에는 string은 호환되지만, char* 와 string은 호환안된다.)
//
// UNICODE 지원 안됨
//	2009-04-03
//	UNICODE 지원 
//
//
//////////////////////////////////////////////////////////////////////
#include "internal_CiUtils.h"
#include "CiLogger.h"
#include "CiUtils.h"

#include "utf8_codecvt.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4245 4702)
#endif
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/lexical_cast.hpp>
#ifdef _WIN32
#pragma warning(pop)
#endif

#ifdef _WIN32
#include <direct.h>
#endif

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

#ifdef _WIN32
#pragma warning (disable: 4101)
#endif

using namespace CiUtils;
using namespace std;

#define MAX_LOG_LENGTH 1024

#ifdef _WIN32
#ifndef snprintf
#define snprintf _snprintf
#endif
#ifndef vsnprintf
#define vsnprintf _vsnprintf
#endif
#endif

static char to_lower(const char toLower)
{
	if ((toLower >= 'A') && (toLower <= 'Z'))
		return char(toLower + 0x20);
	return toLower;
}


CiLogEvent::~CiLogEvent()
{
}

CiLogEvent::CiLogEvent()
: _section(""), _description(""), _level(""), _levelType(CiLogEvent::eNone), _code(""), _whoName(""), _whoID(""), _millisec(0)
{
}

CiLogEvent::CiLogEvent(const char* description, level_type_t levelType, const char* section/* ="" */, const char* code/* ="" */, const char* whoName/* ="" */, const char* whoID/* ="" */)
: _section(section), _description(description), _level(""), _levelType(levelType), _code(code), _whoName(whoName), _whoID(whoID)

{
#ifdef _WIN32
	struct _timeb buffer;
	_ftime( &buffer);
	_loggingTime = *localtime(&buffer.time);
	_millisec = buffer.millitm;
#else
	struct timeval buffer;
	gettimeofday(&buffer, NULL);
	localtime_r(&buffer.tv_sec, &_loggingTime);
	_millisec = buffer.tv_usec / 1000;
#endif

	_level = GetLevelTypeString(levelType);
}

CiLogEvent::CiLogEvent(const char* description, const char* level/* ="" */, const char* section/* ="" */, const char* code/* ="" */, const char* whoName/* ="" */, const char* whoID/* ="" */)
: _section(section), _description(description), _level(level), _levelType(CiLogEvent::eNone), _code(code), _whoName(whoName), _whoID(whoID)
{
#ifdef _WIN32
	struct _timeb buffer;
	_ftime( &buffer );
	_loggingTime = *localtime(&buffer.time);
	_millisec = buffer.millitm;
#else
	struct timeval buffer;
	gettimeofday(&buffer, NULL);
	localtime_r(&buffer.tv_sec, &_loggingTime);
	_millisec = buffer.tv_usec / 1000;
#endif

	_levelType= GetLevelType(_level.c_str());
}

// static function
std::string CiLogEvent::GetLevelTypeString(int levelType)
{
	if (levelType == eNone)
		return "";
	else if (levelType == eReport)
		return "Report";
	else if (levelType == eInformation)
		return "Information";
	else if (levelType == eError)
		return "Error";
	else if (levelType == eFail)
		return "Fail";
	else if (levelType == eSuccess)
		return "Success";
	else if (levelType == eWarning)
		return "Warning";
	else if (levelType == eDebug)
		return "Debug";
	else if (levelType == eException)
		return "Exception";

	return "";
}

// static function
CiLogEvent::level_type_t CiLogEvent::GetLevelType(const char* levelString)
{
	std::string temp = levelString;
	transform(temp.begin(), temp.end(), temp.begin(), to_lower);

	if (temp == "report")
		return eReport;
	else if (temp == "information")
		return eInformation;
	else if (temp == "error")
		return eError;
	else if (temp == "fail")
		return eFail;
	else if (temp == "success")
		return eSuccess;
	else if (temp == "warning")
		return eWarning;
	else if (temp == "debug")
		return eDebug;
	else if (temp == "exception")
		return eException;


	return eNone;
}


//struct tm {
//        int tm_sec;     /* seconds after the minute - [0,59] */
//        int tm_min;     /* minutes after the hour - [0,59] */
//        int tm_hour;    /* hours since midnight - [0,23] */
//        int tm_mday;    /* day of the month - [1,31] */
//        int tm_mon;     /* months since January - [0,11] */
//        int tm_year;    /* years since 1900 */
//        int tm_wday;    /* days since Sunday - [0,6] */
//        int tm_yday;    /* days since January 1 - [0,365] */
//        int tm_isdst;   /* daylight savings time flag */
//};

std::string CiLogEvent::GetEventTimeFullString() const
{
	char str[30];
	sprintf (str, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
		_loggingTime.tm_year+1900, _loggingTime.tm_mon+1, _loggingTime.tm_mday, _loggingTime.tm_hour, _loggingTime.tm_min, _loggingTime.tm_sec, _millisec);
	std::string timeStr = str;

	return timeStr;
}
std::string CiLogEvent::GetEventDateString() const
{
	char str[30];
	sprintf (str, "%04d-%02d-%02d",
		_loggingTime.tm_year+1900, _loggingTime.tm_mon+1, _loggingTime.tm_mday);
	std::string timeStr = str;


	return timeStr;
}
std::string CiLogEvent::GetEventTimeString() const
{
	char str[30];
	sprintf (str, "%02d:%02d:%02d.%03d",
		_loggingTime.tm_hour, _loggingTime.tm_min, _loggingTime.tm_sec, _millisec);
	std::string timeStr = str;

	return timeStr;
}

//////////////////////////////////////////////////////////////////////////
// CiLogger

CiLogger::CiLogger()
:CCiThread2()
{
	_processingSize = 5;
	_loggingFolder= "./Logs";
	_loggingFileName = "log";
	_loggingFileNameExt = "csv";
	_sleepPeriod = 10; // milli

	_currentLoggingFileSize = 0L;
	_currentLoggingFileIndex = 0;
	_currentLoggingFileName="";
	_currentLoggingFileName.reserve(1024);
	_currentLoggingDateString = "";

	_error="";
	_logFormat = eCSV;
	_displayScreenFlag = false;

	_makeFolderForEachMonthFlag = true;

	_bfirsttime = true;
	_bChangeFile = false;;

	SetDefaultValidEventLevel();
}

CiLogger::~CiLogger()
{
	ProcessLogEvent();
}

bool CiLogger::InitInstance()
{
//#ifdef _DEBUG
//	fprintf(stderr, "CiLogger Thread start\n");
//#endif

	LookupLatestLogFile();
	return true;
}

bool CiLogger::ExitInstance()
{
	ProcessLogEvent();

//#ifdef _DEBUG
//	fprintf(stderr, "CiLogger Thread end\n");
//#endif
	return true;
}

bool CiLogger::Run()
{
	ProcessLogEvent();
	SleepThread(_sleepPeriod); // milli

	return true;
}


bool CiLogger::ProcessLogEvent()
{
	vector<CiLogEvent*> logs;
	logs = _messageQueue.Get();

	//2010.11.04 김건우 큐에 쌓인 로그는 한번의 open으로 처리하도록 수정.. 과부하시 성능 이슈 처리 
	if( logs.empty() == true )
	{
		return true;
	}

	std::string folderPath = GetCurrentFolder(logs[0]->GetEventTime());
	std::string fileName = GetCurrentFileName(logs[0]->GetEventTime());
	std::string pathName = folderPath + DIRECTORY_DELIMITER + fileName;

	if( _bChangeFile == true )
	{
		_bChangeFile = false;
		LoggingHeaderAndTail(pathName);
	}

	if( _out.is_open() == true )
	{
		_out.close();
	}
#ifdef _WIN32
	_out.open(pathName.c_str(), ios::in | ios::out | ios::app);
#else
	_out.open(pathName.c_str(), ios::out | ios::app);
#endif

	for (size_t i=0; i<logs.size(); i++)
	{
		Logging(logs[i]);
		delete logs[i];
	}

	if( _out.is_open() == true )
	{
		_out.close();
	}

	return true;
}

void CiLogger::SleepThread(int period /* = 1000 */) // millie sec
{
	int milliPeriod = 0;
	if (period>= 1000)
		milliPeriod = period% 1000;
	else
		milliPeriod = period;

	int secPeriod = period / 1000;

#ifdef _WIN32
	Sleep(milliPeriod);

	HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL );
	if( hEvent )
	{
		int waitCount = (secPeriod*1000)/100;
		for (int i=0; i< waitCount ; i++ )
		{
			WaitForSingleObject( hEvent, 100 );

			if (m_bExit)
			{
				if( hEvent )
				{
					CloseHandle( hEvent );
					hEvent = NULL;
					return;
				}
			}
		}

	}

	if( hEvent )
	{
		CloseHandle( hEvent );
		hEvent = NULL;
	}

#else
	usleep(milliPeriod * 1000);

	// 1 sec loop sleep -> check m_bExit !!
	for ( int i = 0; i < secPeriod; i++ )
	{
		if ( m_bExit )
			return;

		usleep( 1000000 );
	}

#endif
}


void CiLogger::SetEnv(CiLogger::path_type_t pathType, const std::string& loggingFolder, const std::string& loggingFileName, const std::string& ext, const std::string& docType, long fileSizeLimit/*=1024*1024 Byte*/, CiLogger::log_mode_t logMode)
{
	_loggingFolder= loggingFolder;

#ifdef _WIN32
	if (pathType==eRelative)
	{
		char buffer[CI_MAX_FILE_PATH_LENGTH];
		/* Get the current working directory: */
		if( _getcwd( buffer, CI_MAX_FILE_PATH_LENGTH ) == NULL )
		{
			// exception
		}
		std::string cwd = buffer;
		_loggingFolder = cwd + DIRECTORY_DELIMITER  + _loggingFolder;
	}

#else
	if (pathType==eRelative)
	{

		char buffer[CI_MAX_FILE_PATH_LENGTH];
		/* Get the current working directory: */
		if( getcwd( buffer, CI_MAX_FILE_PATH_LENGTH ) == NULL )
		{
			// exception
		}

		std::string cwd = buffer;
		_loggingFolder = cwd + DIRECTORY_DELIMITER  + _loggingFolder;
	}


#endif

	namespace fs = boost::filesystem;
	try {
#if BOOST_VERSION < 104100
		fs::create_directories(fs::path(_loggingFolder, fs::native));
#else
		fs::create_directories(fs::path(_loggingFolder));
#endif
	} catch (fs::filesystem_error& e) {
		TRACE("%s\n", e.what());
	}

	_loggingFileName = loggingFileName;

	std::string temp = ext;
	transform(temp.begin(), temp.end(), temp.begin(), to_lower);
	if (temp == "xml")
		_logFormat = eXML;
	else if (temp== "csv" || temp== "log")
		_logFormat = eCSV;
	else
		_logFormat = eCSV;

	_loggingFileNameExt = temp;
	_loggingFileSizeLimit = fileSizeLimit;
	_docType = docType;
	_logMode = logMode;
}

void CiLogger::SetValidEventLevel(int validLevels)
{
	_validEventLevels = validLevels;
}

// static function
tm CiLogger::GetCurrentTime()
{
#ifdef _WIN32
	time_t tTime = time(NULL);
	struct tm* pstTime = localtime(&tTime); // thread safe 해야 함
	return *pstTime;
#else
	struct timeval buffer;
	gettimeofday(&buffer, NULL);
	struct tm stTime;
	localtime_r(&buffer.tv_sec, &stTime);

	return stTime;
#endif
}


// 월 단위로 folder를 만든다.
std::string CiLogger::GetCurrentFolder(const tm& time)
{
	std::string monthFolder;monthFolder.reserve(10);monthFolder = "";

	if (_makeFolderForEachMonthFlag)
	{
		char str[30];
		sprintf (str, "%04d-%02d",
			time.tm_year+1900, time.tm_mon+1);

		monthFolder = str;
	}
	else
	{
		monthFolder = "";
	}

	namespace fs = boost::filesystem;
	try {
#if BOOST_VERSION < 104100
		fs::create_directories(fs::path(_loggingFolder, fs::native) / fs::path(monthFolder, fs::native));
#else
		fs::create_directories(fs::path(_loggingFolder) / fs::path(monthFolder));
#endif
	} catch (fs::filesystem_error& e) {
		TRACE("%s\n", e.what());
	}

	return _loggingFolder + DIRECTORY_DELIMITER + monthFolder;
}

std::string CiLogger::GetCurrentFileName(const tm& time)
{
	if (_logMode == eByTime)
		return GetCurrentFileNameByTime(time);
	else
		return GetCurrentFileNameBySize(time);
}

std::string CiLogger::GetCurrentFileNameBySize(const tm& time)
{
	char str[30];
	sprintf (str, "%04d-%02d-%02d",
		time.tm_year+1900, time.tm_mon+1, time.tm_mday);
	std::string loggingDateStr = str;

	if (_currentLoggingDateString != loggingDateStr) // 날짜가 달라져서 파일 이름이 달라지는 경우
	{
		_currentLoggingFileIndex=0;
		_currentLoggingFileSize = 0L;
		_currentLoggingDateString = loggingDateStr;

		_currentLoggingFileName = loggingDateStr + "_" + _loggingFileName + "." + _loggingFileNameExt;

		_bfirsttime = false;
		_bChangeFile = true;

	}
	else if (_currentLoggingFileSize > _loggingFileSizeLimit && _loggingFileSizeLimit > 0 ) // index가 증가해서 파일이름이 달라지는 경우
	{
		_currentLoggingFileIndex++;
		_currentLoggingFileSize = 0L;

		char buffer[60];
		sprintf(buffer, "%d", _currentLoggingFileIndex);
		_currentLoggingFileName =  loggingDateStr + "[" + buffer+ "]_" + _loggingFileName + "." + _loggingFileNameExt;

		_bfirsttime = false;
		_bChangeFile = true;

	}

	return _currentLoggingFileName;
}

std::string CiLogger::GetCurrentFileNameByTime(const tm& time)
{
	char str[30];
	sprintf (str, "%04d-%02d-%02d[%d]",
		time.tm_year+1900, time.tm_mon+1, time.tm_mday, time.tm_hour);
	std::string loggingDateStr = str;

	// 시간이 달라져서 파일이름이 달라지는 경우
	if (_currentLoggingDateString != loggingDateStr)
	{
		_currentLoggingDateString = loggingDateStr;
		_currentLoggingFileName = loggingDateStr + "_" + _loggingFileName + "." + _loggingFileNameExt;

		_bfirsttime = false;
		_bChangeFile = true;
	}

	return _currentLoggingFileName;
}

#define EXTRA_LENGTH 256

bool CiLogger::Logging(const CiLogEvent* log)
{
	if ( log == NULL )
		return true;

	std::string eventBody;
	eventBody.reserve(1024);

	if (_logFormat == eXML)
	{
		//<Event Who="who" WhoID="id" Date="2005-12-31" Time="10:00:00" Level="Information" Code="0" Section="Chatter::Run()">
		//<Desc>Merry Christmas!</Desc></Event>

		char buf[MAX_LOG_LENGTH + EXTRA_LENGTH];
		int strLength = snprintf(buf, sizeof(buf) - 1
			, "<Event Who=\"%s\" WhoID=\"%s\" Date=\"%s\" Time=\"%s\" Level=\"%s\" Code=\"%s\" Section=\"%s\">\n<Desc>%s</Desc></Event>\n"
			, GetXMLSafeString(log->GetWhoName()).c_str()
			, GetXMLSafeString(log->GetWhoID()).c_str()
			, GetXMLSafeString(log->GetEventDateString()).c_str()
			, GetXMLSafeString(log->GetEventTimeString()).c_str()
			, GetXMLSafeString(log->GetLevel()).c_str()
			, GetXMLSafeString(log->GetCode()).c_str()
			, GetXMLSafeString(log->GetSection()).c_str()
			, GetXMLSafeString(log->GetDescription()).c_str()
			);

		eventBody = buf;
		strLength += snprintf(&buf[strLength], sizeof(buf) - strLength - 1, "</Log>\n");

		//2010.11.04 김건우 파일이름이 바뀔때 바로 적용할수 있도록 하기 위해...
		std::string fileName = GetCurrentFileName(log->GetEventTime());

		if( _bChangeFile == true )
		{
			_bChangeFile = false;
			if( _out.is_open() == true )
			{
				_out.close();
			}
			std::string folderPath = GetCurrentFolder(log->GetEventTime());
			std::string pathName = folderPath + DIRECTORY_DELIMITER + fileName;

			LoggingHeaderAndTail(pathName);
#ifdef _WIN32
			_out.open(pathName.c_str(), ios_base::in | ios_base::out | ios_base::ate);
#else
			_out.open(pathName.c_str(), ios_base::in | ios_base::out | ios_base::ate);
#endif
		}
		_out.seekp(-GetTailSize(),ios_base::end);
#ifdef _WIN32
		if(!strcmp(_docType.c_str(),"UTF-8" ))
		{
			unsigned char header[] = { 0xEF, 0xBB, 0xBF, 0 };	
			_out << header;
		}
#endif

		_out<< buf;

		_currentLoggingFileSize += strLength + 3; // 3를 더한 이유는 \n 때문에


		if (_displayScreenFlag)
		{
			OnDisplayScreen(eventBody.c_str(), log);
		}

	}
	else if (_logFormat == eCSV)
	{
		char buf[MAX_LOG_LENGTH + EXTRA_LENGTH];
		int strLength = snprintf(buf, sizeof(buf) - 1
			, "%s,%s,%s,%s,%s,%s,%s,%s\n"
			, GetCSVSafeString(log->GetWhoName()).c_str()
			, GetCSVSafeString(log->GetWhoID()).c_str()
			, log->GetEventDateString().c_str()
			, log->GetEventTimeString().c_str()
			, GetCSVSafeString(log->GetLevel()).c_str()
			, GetCSVSafeString(log->GetSection()).c_str()
			, GetCSVSafeString(log->GetCode()).c_str()
			, GetCSVSafeString(log->GetDescription()).c_str()
			);

		//2010.11.04 김건우 파일이름이 바뀔때 바로 적용할수 있도록 하기 위해...
		std::string fileName = GetCurrentFileName(log->GetEventTime());

		if( _bChangeFile == true )
		{
			_bChangeFile = false;
			if( _out.is_open() == true )
			{
				_out.close();
			}
			std::string folderPath = GetCurrentFolder(log->GetEventTime());
			std::string pathName = folderPath + DIRECTORY_DELIMITER + fileName;

			LoggingHeaderAndTail(pathName);
#ifdef _WIN32
			_out.open(pathName.c_str(), ios::in | ios::out | ios::app);
#else
			_out.open(pathName.c_str(), ios::out | ios::app);
#endif
		}

#ifdef _WIN32
		if(!strcmp(_docType.c_str(),"UTF-8" ))
		{
			unsigned char header[] = { 0xEF, 0xBB, 0xBF, 0 };	
			_out << header;
		}
#endif
		if(_bfirsttime == true)
		{
			_bfirsttime = false;
			_out<< "\n";
			_currentLoggingFileSize += 1;
		}

		_out<< buf;

		_currentLoggingFileSize += strLength + 1; // 1을 더한 이유는 \n 때문에

		if (_displayScreenFlag)
		{
			OnDisplayScreen(buf, log);
		}
	}

	return true;
}

void CiLogger::OnDisplayScreen(const char* logText, const CiLogEvent* /*log*/)
{
	fprintf(stderr, "%s", logText);
}

int CiLogger::GetTailSize() const
{
	stringstream tail;
	tail<< "</Log>\n";
	return static_cast<int>(tail.str().length()+1); // +1 를 해줘야 제대로 동작, 이유는 \n 때문일까?
}

bool CiLogger::LoggingHeaderAndTail(const std::string& fullPath)
{
	if (_logFormat == eXML)
	{
		stringstream header;

		if(!strcmp(_docType.c_str(),"UTF-8" ))
		{
			header << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n";
			header << "<Log>\n";
		}
		else if(!strcmp(_docType.c_str(),"euc_kr" ))
		{
			header << "<?xml version=\"1.0\" encoding=\"euc-kr\" standalone=\"no\" ?>\n";
			header << "<Log>\n";
		}

	#ifdef _WIN32
		if (access(fullPath.c_str(), 6)==0)
	#else
		if (access(fullPath.c_str(), 6)==0)
	#endif
		{
			// 이미 있으면 header 와 tail이 있을 것이다.
		}
		else
		{
			std::string headerString = header.str();
			std::fstream out;

#ifdef _WIN32
			out.open(fullPath.c_str(), ios::in | ios::out | ios::app);
#else
			out.open(fullPath.c_str(), ios::out | ios::app);
#endif
			out << headerString;

			stringstream tail;
			tail<< "</Log>\n";

			out << tail.str();

			out.close();

			_currentLoggingFileSize += static_cast<long>(headerString.length());
		}
	}
	else if (_logFormat==eCSV)
	{
		stringstream header;
		header << "NAME,ID, DATE,TIME,LEVEL,SECTION,CODE,DESCRIPTION\n";
#ifdef _WIN32
		if (access(fullPath.c_str(), 6)==0)
#else
		if (access(fullPath.c_str(), 6)==0)
#endif
		{
			// 이미 있으면 header 와 tail이 있을 것이다.
		}
		else
		{
			std::string headerString = header.str();
			std::fstream out;
#ifdef _WIN32
			out.open(fullPath.c_str(), ios::in | ios::out | ios::app);
#else
			out.open(fullPath.c_str(), ios::out | ios::app);
#endif

			out << headerString;
			out.close();

			_currentLoggingFileSize += static_cast<long>(headerString.length());
		}
	}

	return true;
}

void CiLogger::LookupLatestLogFile()
{
	tm current = GetCurrentTime();
	string folder = GetCurrentFolder(current);

	_currentLoggingFileSize=0L;
	_currentLoggingFileIndex = 0;

	char currentDateStr[30];
	sprintf (currentDateStr, "%04d-%02d-%02d",
		current.tm_year+1900, current.tm_mon+1, current.tm_mday);

	vector<string> logFileNames;

	string ext = "." +_loggingFileNameExt;

	namespace fs = boost::filesystem;
	try {
		fs::directory_iterator end_itr;
#if BOOST_VERSION < 104100
		for (fs::directory_iterator itr(fs::path(folder, fs::native)); itr != end_itr; ++itr)
#else
		fs::path folderPath = folder;
		for (fs::directory_iterator itr(folderPath); itr != end_itr; ++itr)
#endif
		{
			if ( fs::is_directory(*itr) )
				continue;

#if BOOST_VERSION < 104100
			string fileName = itr->leaf();
#elif BOOST_VERSION < 104600
			string fileName = itr->path().filename();
#else
			string fileName = itr->path().filename().string();
#endif

			if (fileName.rfind(ext.c_str()) == fileName.length()-4 && fileName.find(currentDateStr) != string::npos)
			{
				logFileNames.push_back(fileName.c_str());
			}
		}
	} catch (fs::filesystem_error& e) {
		TRACE("%s\n", e.what());
		return;
	}

	// file name is like
	// 2005-11-15_HAS.xml
	// 2005-11-15[1]_HAS.xml
	// 2005-11-15[2]_HAS.xml
	vector<int> indexes;
	for (size_t i=0,n=logFileNames.size(); i< n; i++)
	{
		string fname = logFileNames[i];
		fname = fname.substr(10); // _HAS.xml // [1]_HAS.xml

		if (fname[0] != '_' )
		{
			fname = fname.substr(1, fname.find(']')-1);
			//fprintf(stderr, "look up files...index[%s]\n", fname.c_str());
			indexes.push_back(atoi(fname.c_str()));
		}
	}

	int index = 0;
	if (!indexes.empty())
	{
		sort(indexes.begin(), indexes.end(), dec);
		index = indexes.front();
	}


	string latestFileFullPath;
	char buffer[60];
	if (index==0)
		latestFileFullPath = folder + DIRECTORY_DELIMITER + currentDateStr + "_" + _loggingFileName + "." + _loggingFileNameExt;
	else
	{
		sprintf(buffer, "%d", index);
		latestFileFullPath = folder + DIRECTORY_DELIMITER + currentDateStr + "[" + buffer+ "]_" + _loggingFileName + "." + _loggingFileNameExt;
	}

	try {
#if BOOST_VERSION < 104100
		fs::path fsLatestFileFullPath(latestFileFullPath, fs::native);
#else
		fs::path fsLatestFileFullPath(latestFileFullPath);
#endif

		if ( !fs::exists(fsLatestFileFullPath) )
		{
			_currentLoggingDateString ="";
			_currentLoggingFileName = "";
			_currentLoggingFileSize=0L;
			_currentLoggingFileIndex = 0;
			return;
		}

		long size =  static_cast<long>(fs::file_size(fsLatestFileFullPath));
		if ( size > _loggingFileSizeLimit && _loggingFileSizeLimit > 0 )
		{
			_currentLoggingFileSize = 0L;
			_currentLoggingFileIndex = index + 1;
			sprintf(buffer, "%d", _currentLoggingFileIndex);
			_currentLoggingDateString = currentDateStr;
			_currentLoggingFileName = _currentLoggingDateString + "[" + buffer+ "]_" + _loggingFileName + "." + _loggingFileNameExt;
		}
		else
		{
			_currentLoggingFileSize = size;
			_currentLoggingFileIndex = index;
			_currentLoggingDateString = currentDateStr;

			if (index>0)
				_currentLoggingFileName = _currentLoggingDateString + "[" + buffer+ "]_" + _loggingFileName + "." + _loggingFileNameExt;
			else
				_currentLoggingFileName = _currentLoggingDateString + "_" + _loggingFileName + "." + _loggingFileNameExt;
		}
	} catch (fs::filesystem_error& e) {
		TRACE("%s\n", e.what());
		_currentLoggingDateString ="";
		_currentLoggingFileName = "";
		_currentLoggingFileSize=0L;
		_currentLoggingFileIndex = 0;
	}
}


bool CiLogger::ValidateEventLevel(int level)
{
	return (_validEventLevels & level)==0 ? false: true;
}

void CiLogger::SetDefaultValidEventLevel()
{
	//eNone, eDebug, eReport, eInformation, eWarning, eError, eFail, eSuccess
	_validEventLevels =
		CiLogEvent::eNone | CiLogEvent::eDebug| CiLogEvent::eReport | CiLogEvent::eInformation| CiLogEvent::eSuccess | CiLogEvent::eWarning| CiLogEvent::eError| CiLogEvent::eFail | CiLogEvent::eException;
}


std::string CiLogger::GetCSVSafeString(const std::string& str)
{
	char special_chars[3] = { ',', '"', 0 };

	std::string safeStr;
	const std::string& token = str;

	// 일반적인('"' 또는 ','를 포함하지 않은)
	// 토큰이라면 그냥 저장하면 된다.
	if (token.find_first_of(special_chars) == std::string::npos)
	{
		return str;
	}
	// 특수문자를 포함한 토큰이라면 문자열 좌우에 '"'를 붙여주고,
    // 문자열 내부의 '"'를 두 개로 만들어줘야한다.
    else
    {
		safeStr += '"';

		for (size_t k=0; k<token.size(); k++)
		{
			if (token[k] == '"') safeStr += "\"\"";
			else safeStr += token[k];
		}
		safeStr += '"';
    }

	return safeStr;
}


std::string CiLogger::GetXMLSafeString(const std::string& str)
{
	char exeptional_chars[4] = { '&', '<', '>', 0 };

	std::string safeStr;
	safeStr.reserve(str.size()*2); // 적당한 크기를 미리 잡아놓는다.

	const std::string& token = str;


	// 일반적인 경우라면 그냥 저장하면 된다.
	if (token.find_first_of(exeptional_chars) == std::string::npos)
	{
		return str;
	}
	else // 그렇지 않음 경우
    {
		safeStr = "";
		for (size_t k=0; k<token.size(); k++)
		{
			if (token[k] == '&') safeStr += "&amp;";
			else if (token[k] == '<') safeStr += "&lt;";
			else if (token[k] == '>') safeStr += "&gt;";

			else safeStr += token[k];
		}
    }

	return safeStr;
}


void CiLogger::Write(const std::string& who, const std::string& whoID, CiLogEvent::level_type_t level, const std::string& section, const std::string& code, const std::string& description)
{
	Lock();
	if (!ValidateEventLevel(level))
	{
		Unlock();
		return;
	}
	_messageQueue.Put(new CiLogEvent(description.c_str(), level, section.c_str(), code.c_str(), who.c_str(), whoID.c_str()));

	Unlock();
}

void CiLogger::Write(const std::string& who, const std::string& whoID, CiLogEvent::level_type_t level, const std::string& section, const std::string& description)
{
	Lock();
	if (!ValidateEventLevel(level))
	{
		Unlock();
		return;
	}

	_messageQueue.Put(new CiLogEvent(description.c_str(), level,section.c_str(),"", who.c_str(), whoID.c_str()));
	Unlock();
}


void CiLogger::Write(const std::string& who, const std::string& whoID, CiLogEvent::level_type_t level, const std::string& description)
{
	Lock();
	if (!ValidateEventLevel(level))
	{
		Unlock();
		return;
	}

	_messageQueue.Put(new CiLogEvent(description.c_str(), level,"","", who.c_str(), whoID.c_str()));
	Unlock();
}

void CiLogger::Write(const std::string& who, const std::string& whoID, const std::string& description)
{
	Lock();
	if (!ValidateEventLevel(CiLogEvent::eNone))
	{
		Unlock();
		return;
	}

	_messageQueue.Put(new CiLogEvent(description.c_str(), CiLogEvent::eNone ,"","", who.c_str(), whoID.c_str()));
	Unlock();
}

void CiLogger::Write(const std::string& description)
{
	Lock();
	if (!ValidateEventLevel(CiLogEvent::eNone))
	{
		Unlock();
		return;
	}

	_messageQueue.Put(new CiLogEvent(description.c_str(),"","","", "", ""));
	Unlock();
}

void CiLogger::Write(const std::string& who, const std::string& whoID, CiLogEvent::level_type_t level, const std::string& section, const std::string& code, char* format, ...)
{
	Lock();
	if (!ValidateEventLevel(level))
	{
		Unlock();
		return;
	}

	va_list args;
	va_start (args, format);

	char buf[MAX_LOG_LENGTH];

	int length = vsnprintf(buf, sizeof(buf), format, args);
	if (length == -1)
		length = MAX_LOG_LENGTH;
	std::string description(buf, length);

	va_end (args);

	_messageQueue.Put(new CiLogEvent(description.c_str(), level, section.c_str(), code.c_str(), who.c_str(), whoID.c_str()));
	Unlock();
}

void CiLogger::WriteWOCode(const std::string& who, const std::string& whoID, CiLogEvent::level_type_t level, const std::string& section, char* format, ...)
{
	Lock();
	if (!ValidateEventLevel(level))
	{
		Unlock();
		return;
	}

	va_list args;
	va_start (args, format);

	char buf[MAX_LOG_LENGTH];

	int length = vsnprintf(buf, sizeof(buf), format, args);
	if (length == -1)
		length = MAX_LOG_LENGTH;
	std::string description(buf, length);

	va_end (args);

	_messageQueue.Put(new CiLogEvent(description.c_str(), level, section.c_str(),"", who.c_str(), whoID.c_str()));
	Unlock();

}

void CiLogger::WriteWOSection(const std::string& who, const std::string& whoID, CiLogEvent::level_type_t level, char* format, ...)
{
	Lock();
	if (!ValidateEventLevel(level))
	{
		Unlock();
		return;
	}

	va_list args;
	va_start (args, format);

	char buf[MAX_LOG_LENGTH];

	int length = vsnprintf(buf, sizeof(buf), format, args);
	if (length == -1)
		length = MAX_LOG_LENGTH;
	std::string description(buf, length);

	va_end (args);

	_messageQueue.Put(new CiLogEvent(description.c_str(), level,"","", who.c_str(), whoID.c_str()));
	Unlock();
}

void CiLogger::WriteWOLevel(const std::string& who, const std::string& whoID, char* format, ...) // 1024 limit
{
	Lock();
	if (!ValidateEventLevel(CiLogEvent::eNone))
	{
		Unlock();
		return;
	}

	va_list args;
	va_start (args, format);

	char buf[MAX_LOG_LENGTH];

	int length = vsnprintf(buf, sizeof(buf), format, args);
	if (length == -1)
		length = MAX_LOG_LENGTH;
	std::string description(buf, length);

	va_end (args);

	_messageQueue.Put(new CiLogEvent(description.c_str(), "","","", who.c_str(), whoID.c_str()));
	Unlock();
}



void CiLogger::WriteWOWho(char* format, ...)
{
	Lock();
	if (!ValidateEventLevel(CiLogEvent::eNone))
	{
		Unlock();
		return;
	}

	va_list args;
	va_start (args, format);

	char buf[MAX_LOG_LENGTH];

	int length = vsnprintf(buf, sizeof(buf), format, args);
	if (length == -1)
		length = MAX_LOG_LENGTH;
	std::string description(buf, length);

	va_end (args);

	_messageQueue.Put(new CiLogEvent(description.c_str(),"","","", "", ""));
	Unlock();
}


/////////////////////////////////////////////////////////////////////////////////////////////////

using namespace boost;

string CiUtils::where(const string& file, const string& function, int line)
{
	string::size_type pos = file.rfind(DIRECTORY_DELIMITER);
	return (pos == string::npos ? file : file.substr(pos+1)) + ':' + function + '(' + lexical_cast<string>(line) + ')';
}

boost::scoped_ptr<LoggerBase> LoggerBase::_instance(NULL);

void LoggerBase::debug(const std::string& section, int code, const char* format, ...)
{
	if ( m_ciThreadHandle == CI_THREAD2_INVALID_THREAD_HANDLE || !ValidateEventLevel(CiLogEvent::eDebug) )
		return;

	va_list args;
	va_start (args, format);
	char buf[MAX_LOG_LENGTH];
	vsnprintf(buf, sizeof(buf), format, args);
	buf[MAX_LOG_LENGTH-1] = '\0';
	va_end (args);

	Lock();
	_messageQueue.Put(new CiLogEvent(buf, CiLogEvent::eDebug, section.c_str(), lexical_cast<string>(code).c_str(), _whoName.c_str(), _whoID.c_str()));
	Unlock();
}

void LoggerBase::debug(const std::string& section, const char* format, ...)
{
	if ( m_ciThreadHandle == CI_THREAD2_INVALID_THREAD_HANDLE || !ValidateEventLevel(CiLogEvent::eDebug) )
		return;

	va_list args;
	va_start (args, format);
	char buf[MAX_LOG_LENGTH];
	vsnprintf(buf, sizeof(buf), format, args);
	buf[MAX_LOG_LENGTH-1] = '\0';
	va_end (args);

	Lock();
	_messageQueue.Put(new CiLogEvent(buf, CiLogEvent::eDebug, section.c_str(), "", _whoName.c_str(), _whoID.c_str()));
	Unlock();
}

void LoggerBase::report(const std::string& section, int code, const char* format, ...)
{
	if ( m_ciThreadHandle == CI_THREAD2_INVALID_THREAD_HANDLE || !ValidateEventLevel(CiLogEvent::eReport) )
		return;

	va_list args;
	va_start (args, format);
	char buf[MAX_LOG_LENGTH];
	vsnprintf(buf, sizeof(buf), format, args);
	buf[MAX_LOG_LENGTH-1] = '\0';
	va_end (args);

	Lock();
	_messageQueue.Put(new CiLogEvent(buf, CiLogEvent::eReport, section.c_str(), lexical_cast<string>(code).c_str(), _whoName.c_str(), _whoID.c_str()));
	Unlock();
}

void LoggerBase::report(const std::string& section, const char* format, ...)
{
	if ( m_ciThreadHandle == CI_THREAD2_INVALID_THREAD_HANDLE || !ValidateEventLevel(CiLogEvent::eReport) )
		return;

	va_list args;
	va_start (args, format);
	char buf[MAX_LOG_LENGTH];
	vsnprintf(buf, sizeof(buf), format, args);
	buf[MAX_LOG_LENGTH-1] = '\0';
	va_end (args);

	Lock();
	_messageQueue.Put(new CiLogEvent(buf, CiLogEvent::eReport, section.c_str(), "", _whoName.c_str(), _whoID.c_str()));
	Unlock();
}


void LoggerBase::info(const std::string& section, int code, const char* format, ...)
{
	if ( m_ciThreadHandle == CI_THREAD2_INVALID_THREAD_HANDLE || !ValidateEventLevel(CiLogEvent::eInformation) )
		return;

	va_list args;
	va_start (args, format);
	char buf[MAX_LOG_LENGTH];
	vsnprintf(buf, sizeof(buf), format, args);
	buf[MAX_LOG_LENGTH-1] = '\0';
	va_end (args);

	Lock();
	_messageQueue.Put(new CiLogEvent(buf, CiLogEvent::eInformation, section.c_str(), lexical_cast<string>(code).c_str(), _whoName.c_str(), _whoID.c_str()));
	Unlock();
}

void LoggerBase::info(const std::string& section, const char* format, ...)
{
	if ( m_ciThreadHandle == CI_THREAD2_INVALID_THREAD_HANDLE || !ValidateEventLevel(CiLogEvent::eInformation) )
		return;

	va_list args;
	va_start (args, format);
	char buf[MAX_LOG_LENGTH];
	vsnprintf(buf, sizeof(buf), format, args);
	buf[MAX_LOG_LENGTH-1] = '\0';
	va_end (args);

	Lock();
	_messageQueue.Put(new CiLogEvent(buf, CiLogEvent::eInformation, section.c_str(), "", _whoName.c_str(), _whoID.c_str()));
	Unlock();
}

void LoggerBase::success(const std::string& section, int code, const char* format, ...)
{
	if ( m_ciThreadHandle == CI_THREAD2_INVALID_THREAD_HANDLE || !ValidateEventLevel(CiLogEvent::eSuccess) )
		return;

	va_list args;
	va_start (args, format);
	char buf[MAX_LOG_LENGTH];
	vsnprintf(buf, sizeof(buf), format, args);
	buf[MAX_LOG_LENGTH-1] = '\0';
	va_end (args);

	Lock();
	_messageQueue.Put(new CiLogEvent(buf, CiLogEvent::eSuccess, section.c_str(), lexical_cast<string>(code).c_str(), _whoName.c_str(), _whoID.c_str()));
	Unlock();
}

void LoggerBase::success(const std::string& section, const char* format, ...)
{
	if ( m_ciThreadHandle == CI_THREAD2_INVALID_THREAD_HANDLE || !ValidateEventLevel(CiLogEvent::eSuccess) )
		return;

	va_list args;
	va_start (args, format);
	char buf[MAX_LOG_LENGTH];
	vsnprintf(buf, sizeof(buf), format, args);
	buf[MAX_LOG_LENGTH-1] = '\0';
	va_end (args);

	Lock();
	_messageQueue.Put(new CiLogEvent(buf, CiLogEvent::eSuccess, section.c_str(), "", _whoName.c_str(), _whoID.c_str()));
	Unlock();
}

void LoggerBase::warning(const std::string& section, int code, const char* format, ...)
{
	if ( m_ciThreadHandle == CI_THREAD2_INVALID_THREAD_HANDLE || !ValidateEventLevel(CiLogEvent::eWarning) )
		return;

	va_list args;
	va_start (args, format);
	char buf[MAX_LOG_LENGTH];
	vsnprintf(buf, sizeof(buf), format, args);
	buf[MAX_LOG_LENGTH-1] = '\0';
	va_end (args);

	Lock();
	_messageQueue.Put(new CiLogEvent(buf, CiLogEvent::eWarning, section.c_str(), lexical_cast<string>(code).c_str(), _whoName.c_str(), _whoID.c_str()));
	Unlock();
}

void LoggerBase::warning(const std::string& section, const char* format, ...)
{
	if ( m_ciThreadHandle == CI_THREAD2_INVALID_THREAD_HANDLE || !ValidateEventLevel(CiLogEvent::eWarning) )
		return;

	va_list args;
	va_start (args, format);
	char buf[MAX_LOG_LENGTH];
	vsnprintf(buf, sizeof(buf), format, args);
	buf[MAX_LOG_LENGTH-1] = '\0';
	va_end (args);

	Lock();
	_messageQueue.Put(new CiLogEvent(buf, CiLogEvent::eWarning, section.c_str(), "", _whoName.c_str(), _whoID.c_str()));
	Unlock();
}

void LoggerBase::error(const std::string& section, int code, const char* format, ...)
{
	if ( m_ciThreadHandle == CI_THREAD2_INVALID_THREAD_HANDLE || !ValidateEventLevel(CiLogEvent::eError) )
		return;

	va_list args;
	va_start (args, format);
	char buf[MAX_LOG_LENGTH];
	vsnprintf(buf, sizeof(buf), format, args);
	buf[MAX_LOG_LENGTH-1] = '\0';
	va_end (args);

	Lock();
	_messageQueue.Put(new CiLogEvent(buf, CiLogEvent::eError, section.c_str(), lexical_cast<string>(code).c_str(), _whoName.c_str(), _whoID.c_str()));
	Unlock();
}

void LoggerBase::error(const std::string& section, const char* format, ...)
{
	if ( m_ciThreadHandle == CI_THREAD2_INVALID_THREAD_HANDLE || !ValidateEventLevel(CiLogEvent::eError) )
		return;

	va_list args;
	va_start (args, format);
	char buf[MAX_LOG_LENGTH];
	vsnprintf(buf, sizeof(buf), format, args);
	buf[MAX_LOG_LENGTH-1] = '\0';
	va_end (args);

	Lock();
	_messageQueue.Put(new CiLogEvent(buf, CiLogEvent::eError, section.c_str(), "", _whoName.c_str(), _whoID.c_str()));
	Unlock();
}

void LoggerBase::fail(const std::string& section, int code, const char* format, ...)
{
	if ( m_ciThreadHandle == CI_THREAD2_INVALID_THREAD_HANDLE || !ValidateEventLevel(CiLogEvent::eFail) )
		return;

	va_list args;
	va_start (args, format);
	char buf[MAX_LOG_LENGTH];
	vsnprintf(buf, sizeof(buf), format, args);
	buf[MAX_LOG_LENGTH-1] = '\0';
	va_end (args);

	Lock();
	_messageQueue.Put(new CiLogEvent(buf, CiLogEvent::eFail, section.c_str(), lexical_cast<string>(code).c_str(), _whoName.c_str(), _whoID.c_str()));
	Unlock();
}

void LoggerBase::fail(const std::string& section, const char* format, ...)
{
	if ( m_ciThreadHandle == CI_THREAD2_INVALID_THREAD_HANDLE || !ValidateEventLevel(CiLogEvent::eFail) )
		return;

	va_list args;
	va_start (args, format);
	char buf[MAX_LOG_LENGTH];
	vsnprintf(buf, sizeof(buf), format, args);
	buf[MAX_LOG_LENGTH-1] = '\0';
	va_end (args);

	Lock();
	_messageQueue.Put(new CiLogEvent(buf, CiLogEvent::eFail, section.c_str(), "", _whoName.c_str(), _whoID.c_str()));
	Unlock();
}

void LoggerBase::exception(const std::string& section, int code, const char* format, ...)
{
	if ( m_ciThreadHandle == CI_THREAD2_INVALID_THREAD_HANDLE || !ValidateEventLevel(CiLogEvent::eException) )
		return;

	va_list args;
	va_start (args, format);
	char buf[MAX_LOG_LENGTH];
	vsnprintf(buf, sizeof(buf), format, args);
	buf[MAX_LOG_LENGTH-1] = '\0';
	va_end (args);

	Lock();
	_messageQueue.Put(new CiLogEvent(buf, CiLogEvent::eException, section.c_str(), lexical_cast<string>(code).c_str(), _whoName.c_str(), _whoID.c_str()));
	Unlock();
}

void LoggerBase::exception(const std::string& section, const char* format, ...)
{
	if ( m_ciThreadHandle == CI_THREAD2_INVALID_THREAD_HANDLE || !ValidateEventLevel(CiLogEvent::eException) )
		return;

	va_list args;
	va_start (args, format);
	char buf[MAX_LOG_LENGTH];
	vsnprintf(buf, sizeof(buf), format, args);
	buf[MAX_LOG_LENGTH-1] = '\0';
	va_end (args);

	Lock();
	_messageQueue.Put(new CiLogEvent(buf, CiLogEvent::eException, section.c_str(), "", _whoName.c_str(), _whoID.c_str()));
	Unlock();
}
//2009.04.02 추가 -유니코드
void LoggerBase::debug(const std::string& section, int code, const wchar_t* wFormat, ...)
{
	if ( m_ciThreadHandle == CI_THREAD2_INVALID_THREAD_HANDLE || !ValidateEventLevel(CiLogEvent::eDebug) )
		return;

	va_list args;
	va_start (args, wFormat);
	wchar_t buf[MAX_LOG_LENGTH];
	vswprintf(buf, MAX_LOG_LENGTH-1, wFormat, args);
	buf[(MAX_LOG_LENGTH-1)] = '\0';
	va_end (args);

	std::string format;
	if( !strcmp(_docType.c_str(),"UTF-8" ))
	{
#if defined(_WIN32) && defined(_DEBUG)
#pragma push_macro("new")
#undef new
#endif
		std::locale loc(std::locale(""), new utf8_codecvt);
#if defined(_WIN32) && defined(_DEBUG)
#pragma pop_macro("new")
#endif
		format = wstr2string(const_cast<wchar_t*>(buf), loc);

	}
	else
	{
		format = wstr2string(const_cast<wchar_t*>(buf));
	}

	Lock();
	_messageQueue.Put(new CiLogEvent(format.c_str(), CiLogEvent::eDebug, section.c_str(), lexical_cast<string>(code).c_str(), _whoName.c_str(), _whoID.c_str()));
	Unlock();
}

void LoggerBase::debug(const std::string& section, const wchar_t* wFormat, ...)
{
	if ( m_ciThreadHandle == CI_THREAD2_INVALID_THREAD_HANDLE || !ValidateEventLevel(CiLogEvent::eDebug) )
		return;

	va_list args;
	va_start (args, wFormat);
	wchar_t buf[MAX_LOG_LENGTH];
	vswprintf(buf, MAX_LOG_LENGTH-1, wFormat, args);
	buf[(MAX_LOG_LENGTH-1)] = '\0';
	va_end (args);

	std::string format;
	if( !strcmp(_docType.c_str(),"UTF-8" ))
	{
#if defined(_WIN32) && defined(_DEBUG)
#pragma push_macro("new")
#undef new
#endif
		std::locale loc(std::locale(""), new utf8_codecvt);
#if defined(_WIN32) && defined(_DEBUG)
#pragma pop_macro("new")
#endif
		format = wstr2string(const_cast<wchar_t*>(buf), loc);

	}
	else
	{
		format = wstr2string(const_cast<wchar_t*>(buf));
	}

	Lock();
	_messageQueue.Put(new CiLogEvent(format.c_str(), CiLogEvent::eDebug, section.c_str(), "", _whoName.c_str(), _whoID.c_str()));
	Unlock();
}

void LoggerBase::report(const std::string& section, int code, const wchar_t* wFormat, ...)
{
	if ( m_ciThreadHandle == CI_THREAD2_INVALID_THREAD_HANDLE || !ValidateEventLevel(CiLogEvent::eReport) )
		return;

	va_list args;
	va_start (args, wFormat);
	wchar_t buf[MAX_LOG_LENGTH];
	vswprintf(buf, MAX_LOG_LENGTH-1, wFormat, args);
	buf[(MAX_LOG_LENGTH-1)] = '\0';
	va_end (args);

	std::string format;
	if( !strcmp(_docType.c_str(),"UTF-8" ))
	{
#if defined(_WIN32) && defined(_DEBUG)
#pragma push_macro("new")
#undef new
#endif
		std::locale loc(std::locale(""), new utf8_codecvt);
#if defined(_WIN32) && defined(_DEBUG)
#pragma pop_macro("new")
#endif
		format = wstr2string(const_cast<wchar_t*>(buf), loc);

	}
	else
	{
		format = wstr2string(const_cast<wchar_t*>(buf));
	}

	Lock();
	_messageQueue.Put(new CiLogEvent(format.c_str(), CiLogEvent::eReport, section.c_str(), lexical_cast<string>(code).c_str(), _whoName.c_str(), _whoID.c_str()));
	Unlock();
}

void LoggerBase::report(const std::string& section, const wchar_t* wFormat, ...)
{
	if ( m_ciThreadHandle == CI_THREAD2_INVALID_THREAD_HANDLE || !ValidateEventLevel(CiLogEvent::eReport) )
		return;

	va_list args;
	va_start (args, wFormat);
	wchar_t buf[MAX_LOG_LENGTH];
	vswprintf(buf, MAX_LOG_LENGTH-1, wFormat, args);
	buf[(MAX_LOG_LENGTH-1)] = '\0';
	va_end (args);

	std::string format;
	if( !strcmp(_docType.c_str(),"UTF-8" ))
	{
#if defined(_WIN32) && defined(_DEBUG)
#pragma push_macro("new")
#undef new
#endif
		std::locale loc(std::locale(""), new utf8_codecvt);
#if defined(_WIN32) && defined(_DEBUG)
#pragma pop_macro("new")
#endif
		format = wstr2string(const_cast<wchar_t*>(buf), loc);

	}
	else
	{
		format = wstr2string(const_cast<wchar_t*>(buf));
	}

	Lock();
	_messageQueue.Put(new CiLogEvent(format.c_str(), CiLogEvent::eReport, section.c_str(), "", _whoName.c_str(), _whoID.c_str()));
	Unlock();
}

void LoggerBase::info(const std::string& section, int code, const wchar_t* wFormat, ...)
{
	if ( m_ciThreadHandle == CI_THREAD2_INVALID_THREAD_HANDLE || !ValidateEventLevel(CiLogEvent::eInformation) )
		return;

	va_list args;
	va_start (args, wFormat);
	wchar_t buf[MAX_LOG_LENGTH];
	vswprintf(buf, MAX_LOG_LENGTH-1, wFormat, args);
	buf[(MAX_LOG_LENGTH-1)] = '\0';
	va_end (args);

	std::string format;
	if( !strcmp(_docType.c_str(),"UTF-8" ))
	{
#if defined(_WIN32) && defined(_DEBUG)
#pragma push_macro("new")
#undef new
#endif
		std::locale loc(std::locale(""), new utf8_codecvt);
#if defined(_WIN32) && defined(_DEBUG)
#pragma pop_macro("new")
#endif
		format = wstr2string(const_cast<wchar_t*>(buf), loc);

	}
	else
	{
		format = wstr2string(const_cast<wchar_t*>(buf));
	}

	Lock();
	_messageQueue.Put(new CiLogEvent(format.c_str(), CiLogEvent::eInformation, section.c_str(), lexical_cast<string>(code).c_str(), _whoName.c_str(), _whoID.c_str()));
	Unlock();
}

void LoggerBase::info(const std::string& section, const wchar_t* wFormat, ...)
{
	if ( m_ciThreadHandle == CI_THREAD2_INVALID_THREAD_HANDLE || !ValidateEventLevel(CiLogEvent::eInformation) )
		return;

	va_list args;
	va_start (args, wFormat);
	wchar_t buf[MAX_LOG_LENGTH];
	vswprintf(buf, MAX_LOG_LENGTH-1, wFormat, args);
	buf[(MAX_LOG_LENGTH-1)] = '\0';
	va_end (args);

	std::string format;
	if( !strcmp(_docType.c_str(),"UTF-8" ))
	{
#if defined(_WIN32) && defined(_DEBUG)
#pragma push_macro("new")
#undef new
#endif
		std::locale loc(std::locale(""), new utf8_codecvt);
#if defined(_WIN32) && defined(_DEBUG)
#pragma pop_macro("new")
#endif
		format = wstr2string(const_cast<wchar_t*>(buf), loc);

	}
	else
	{
		format = wstr2string(const_cast<wchar_t*>(buf));
	}

	Lock();
	_messageQueue.Put(new CiLogEvent(format.c_str(), CiLogEvent::eInformation, section.c_str(), "", _whoName.c_str(), _whoID.c_str()));
	Unlock();
}

void LoggerBase::success(const std::string& section, int code, const wchar_t* wFormat, ...)
{
	if ( m_ciThreadHandle == CI_THREAD2_INVALID_THREAD_HANDLE || !ValidateEventLevel(CiLogEvent::eSuccess) )
		return;

	va_list args;
	va_start (args, wFormat);
	wchar_t buf[MAX_LOG_LENGTH];
	vswprintf(buf, MAX_LOG_LENGTH-1, wFormat, args);
	buf[(MAX_LOG_LENGTH-1)] = '\0';
	va_end (args);

	std::string format;
	if( !strcmp(_docType.c_str(),"UTF-8" ))
	{
#if defined(_WIN32) && defined(_DEBUG)
#pragma push_macro("new")
#undef new
#endif
		std::locale loc(std::locale(""), new utf8_codecvt);
#if defined(_WIN32) && defined(_DEBUG)
#pragma pop_macro("new")
#endif
		format = wstr2string(const_cast<wchar_t*>(buf), loc);

	}
	else
	{
		format = wstr2string(const_cast<wchar_t*>(buf));
	}

	Lock();
	_messageQueue.Put(new CiLogEvent(format.c_str(), CiLogEvent::eSuccess, section.c_str(), lexical_cast<string>(code).c_str(), _whoName.c_str(), _whoID.c_str()));
	Unlock();
}

void LoggerBase::success(const std::string& section, const wchar_t* wFormat, ...)
{
	if ( m_ciThreadHandle == CI_THREAD2_INVALID_THREAD_HANDLE || !ValidateEventLevel(CiLogEvent::eSuccess) )
		return;

	va_list args;
	va_start (args, wFormat);
	wchar_t buf[MAX_LOG_LENGTH];
	vswprintf(buf, MAX_LOG_LENGTH-1, wFormat, args);
	buf[(MAX_LOG_LENGTH-1)] = '\0';
	va_end (args);

	std::string format;
	if( !strcmp(_docType.c_str(),"UTF-8" ))
	{
#if defined(_WIN32) && defined(_DEBUG)
#pragma push_macro("new")
#undef new
#endif
		std::locale loc(std::locale(""), new utf8_codecvt);
#if defined(_WIN32) && defined(_DEBUG)
#pragma pop_macro("new")
#endif
		format = wstr2string(const_cast<wchar_t*>(buf), loc);

	}
	else
	{
		format = wstr2string(const_cast<wchar_t*>(buf));
	}

	Lock();
	_messageQueue.Put(new CiLogEvent(format.c_str(), CiLogEvent::eSuccess, section.c_str(), "", _whoName.c_str(), _whoID.c_str()));
	Unlock();
}

void LoggerBase::warning(const std::string& section, int code, const wchar_t* wFormat, ...)
{
	if ( m_ciThreadHandle == CI_THREAD2_INVALID_THREAD_HANDLE || !ValidateEventLevel(CiLogEvent::eWarning) )
		return;

	va_list args;
	va_start (args, wFormat);
	wchar_t buf[MAX_LOG_LENGTH];
	vswprintf(buf, MAX_LOG_LENGTH-1, wFormat, args);
	buf[(MAX_LOG_LENGTH-1)] = '\0';
	va_end (args);

	std::string format;
	if( !strcmp(_docType.c_str(),"UTF-8" ))
	{
#if defined(_WIN32) && defined(_DEBUG)
#pragma push_macro("new")
#undef new
#endif
		std::locale loc(std::locale(""), new utf8_codecvt);
#if defined(_WIN32) && defined(_DEBUG)
#pragma pop_macro("new")
#endif
		format = wstr2string(const_cast<wchar_t*>(buf), loc);

	}
	else
	{
		format = wstr2string(const_cast<wchar_t*>(buf));
	}

	Lock();
	_messageQueue.Put(new CiLogEvent(format.c_str(), CiLogEvent::eWarning, section.c_str(), lexical_cast<string>(code).c_str(), _whoName.c_str(), _whoID.c_str()));
	Unlock();
}

void LoggerBase::warning(const std::string& section, const wchar_t* wFormat, ...)
{
	if ( m_ciThreadHandle == CI_THREAD2_INVALID_THREAD_HANDLE || !ValidateEventLevel(CiLogEvent::eWarning) )
		return;

	va_list args;
	va_start (args, wFormat);
	wchar_t buf[MAX_LOG_LENGTH];
	vswprintf(buf, MAX_LOG_LENGTH-1, wFormat, args);
	buf[(MAX_LOG_LENGTH-1)] = '\0';
	va_end (args);

	std::string format;
	if( !strcmp(_docType.c_str(),"UTF-8" ))
	{
#if defined(_WIN32) && defined(_DEBUG)
#pragma push_macro("new")
#undef new
#endif
		std::locale loc(std::locale(""), new utf8_codecvt);
#if defined(_WIN32) && defined(_DEBUG)
#pragma pop_macro("new")
#endif
		format = wstr2string(const_cast<wchar_t*>(buf), loc);

	}
	else
	{
		format = wstr2string(const_cast<wchar_t*>(buf));
	}

	Lock();
	_messageQueue.Put(new CiLogEvent(format.c_str(), CiLogEvent::eWarning, section.c_str(), "", _whoName.c_str(), _whoID.c_str()));
	Unlock();
}

void LoggerBase::error(const std::string& section, int code, const wchar_t* wFormat, ...)
{
	if ( m_ciThreadHandle == CI_THREAD2_INVALID_THREAD_HANDLE || !ValidateEventLevel(CiLogEvent::eError) )
		return;

	va_list args;
	va_start (args, wFormat);
	wchar_t buf[MAX_LOG_LENGTH];
	vswprintf(buf, MAX_LOG_LENGTH-1, wFormat, args);
	buf[(MAX_LOG_LENGTH-1)] = '\0';
	va_end (args);

	std::string format;
	if( !strcmp(_docType.c_str(),"UTF-8" ))
	{
#if defined(_WIN32) && defined(_DEBUG)
#pragma push_macro("new")
#undef new
#endif
		std::locale loc(std::locale(""), new utf8_codecvt);
#if defined(_WIN32) && defined(_DEBUG)
#pragma pop_macro("new")
#endif
		format = wstr2string(const_cast<wchar_t*>(buf), loc);

	}
	else
	{
		format = wstr2string(const_cast<wchar_t*>(buf));
	}

	Lock();
	_messageQueue.Put(new CiLogEvent(format.c_str(), CiLogEvent::eError, section.c_str(), lexical_cast<string>(code).c_str(), _whoName.c_str(), _whoID.c_str()));
	Unlock();
}

void LoggerBase::error(const std::string& section, const wchar_t* wFormat, ...)
{
	if ( m_ciThreadHandle == CI_THREAD2_INVALID_THREAD_HANDLE || !ValidateEventLevel(CiLogEvent::eError) )
		return;

	va_list args;
	va_start (args, wFormat);
	wchar_t buf[MAX_LOG_LENGTH];
	vswprintf(buf, MAX_LOG_LENGTH-1, wFormat, args);
	buf[(MAX_LOG_LENGTH-1)] = '\0';
	va_end (args);

	std::string format;
	if( !strcmp(_docType.c_str(),"UTF-8" ))
	{
#if defined(_WIN32) && defined(_DEBUG)
#pragma push_macro("new")
#undef new
#endif
		std::locale loc(std::locale(""), new utf8_codecvt);
#if defined(_WIN32) && defined(_DEBUG)
#pragma pop_macro("new")
#endif
		format = wstr2string(const_cast<wchar_t*>(buf), loc);

	}
	else
	{
		format = wstr2string(const_cast<wchar_t*>(buf));
	}

	Lock();
	_messageQueue.Put(new CiLogEvent(format.c_str(), CiLogEvent::eError, section.c_str(), "", _whoName.c_str(), _whoID.c_str()));
	Unlock();
}

void LoggerBase::fail(const std::string& section, int code, const wchar_t* wFormat, ...)
{
	if ( m_ciThreadHandle == CI_THREAD2_INVALID_THREAD_HANDLE || !ValidateEventLevel(CiLogEvent::eFail) )
		return;

	va_list args;
	va_start (args, wFormat);
	wchar_t buf[MAX_LOG_LENGTH];
	vswprintf(buf, MAX_LOG_LENGTH-1, wFormat, args);
	buf[(MAX_LOG_LENGTH-1)] = '\0';
	va_end (args);

	std::string format;
	if( !strcmp(_docType.c_str(),"UTF-8" ))
	{
#if defined(_WIN32) && defined(_DEBUG)
#pragma push_macro("new")
#undef new
#endif
		std::locale loc(std::locale(""), new utf8_codecvt);
#if defined(_WIN32) && defined(_DEBUG)
#pragma pop_macro("new")
#endif
		format = wstr2string(const_cast<wchar_t*>(buf), loc);

	}
	else
	{
		format = wstr2string(const_cast<wchar_t*>(buf));
	}

	Lock();
	_messageQueue.Put(new CiLogEvent(format.c_str(), CiLogEvent::eFail, section.c_str(), lexical_cast<string>(code).c_str(), _whoName.c_str(), _whoID.c_str()));
	Unlock();
}

void LoggerBase::fail(const std::string& section, const wchar_t* wFormat, ...)
{
	if ( m_ciThreadHandle == CI_THREAD2_INVALID_THREAD_HANDLE || !ValidateEventLevel(CiLogEvent::eFail) )
		return;

	va_list args;
	va_start (args, wFormat);
	wchar_t buf[MAX_LOG_LENGTH];
	vswprintf(buf, MAX_LOG_LENGTH-1, wFormat, args);
	buf[(MAX_LOG_LENGTH-1)] = '\0';
	va_end (args);

	std::string format;
	if( !strcmp(_docType.c_str(),"UTF-8" ))
	{
#if defined(_WIN32) && defined(_DEBUG)
#pragma push_macro("new")
#undef new
#endif
		std::locale loc(std::locale(""), new utf8_codecvt);
#if defined(_WIN32) && defined(_DEBUG)
#pragma pop_macro("new")
#endif
		format = wstr2string(const_cast<wchar_t*>(buf), loc);

	}
	else
	{
		format = wstr2string(const_cast<wchar_t*>(buf));
	}

	Lock();
	_messageQueue.Put(new CiLogEvent(format.c_str(), CiLogEvent::eFail, section.c_str(), "", _whoName.c_str(), _whoID.c_str()));
	Unlock();
}

void LoggerBase::exception(const std::string& section, int code, const wchar_t* wFormat, ...)
{
	if ( m_ciThreadHandle == CI_THREAD2_INVALID_THREAD_HANDLE || !ValidateEventLevel(CiLogEvent::eException) )
		return;

	va_list args;
	va_start (args, wFormat);
	wchar_t buf[MAX_LOG_LENGTH];
	vswprintf(buf, MAX_LOG_LENGTH-1, wFormat, args);
	buf[(MAX_LOG_LENGTH-1)] = '\0';
	va_end (args);

	std::string format;
	if( !strcmp(_docType.c_str(),"UTF-8" ))
	{
#if defined(_WIN32) && defined(_DEBUG)
#pragma push_macro("new")
#undef new
#endif
		std::locale loc(std::locale(""), new utf8_codecvt);
#if defined(_WIN32) && defined(_DEBUG)
#pragma pop_macro("new")
#endif
		format = wstr2string(const_cast<wchar_t*>(buf), loc);

	}
	else
	{
		format = wstr2string(const_cast<wchar_t*>(buf));
	}

	Lock();
	_messageQueue.Put(new CiLogEvent(format.c_str(), CiLogEvent::eException, section.c_str(), lexical_cast<string>(code).c_str(), _whoName.c_str(), _whoID.c_str()));
	Unlock();
}

void LoggerBase::exception(const std::string& section, const wchar_t* wFormat, ...)
{
	if ( m_ciThreadHandle == CI_THREAD2_INVALID_THREAD_HANDLE || !ValidateEventLevel(CiLogEvent::eException) )
		return;

	va_list args;
	va_start (args, wFormat);
	wchar_t buf[MAX_LOG_LENGTH];
	vswprintf(buf, MAX_LOG_LENGTH-1, wFormat, args);
	buf[(MAX_LOG_LENGTH-1)] = '\0';
	va_end (args);

	std::string format;
	if( !strcmp(_docType.c_str(),"UTF-8" ))
	{
#if defined(_WIN32) && defined(_DEBUG)
#pragma push_macro("new")
#undef new
#endif
		std::locale loc(std::locale(""), new utf8_codecvt);
#if defined(_WIN32) && defined(_DEBUG)
#pragma pop_macro("new")
#endif
		format = wstr2string(const_cast<wchar_t*>(buf), loc);
	
	}
	else
	{
		format = wstr2string(const_cast<wchar_t*>(buf));
	}

	Lock();
	_messageQueue.Put(new CiLogEvent(format.c_str(), CiLogEvent::eException, section.c_str(), "", _whoName.c_str(), _whoID.c_str()));
	Unlock();
}

std::string LoggerBase::wstr2string(const wchar_t* wstr, std::locale const& loc)
{
	typedef std::codecvt<wchar_t, char, std::mbstate_t> t_codecvt;
	t_codecvt const& codecvt = std::use_facet<t_codecvt>(loc);
	std::mbstate_t state = std::mbstate_t();
	std::vector<char> temp((wcslen(wstr) + 1) * codecvt.max_length());

	wchar_t const* input = wstr;
	char* output = &temp[0];
	std::codecvt_base::result r = codecvt.out(state, wstr, wstr + wcslen(wstr), input, &temp[0], &temp[0] + temp.size(), output);
	if( r == std::codecvt_base::error)
		return std::string(" ");

	return std::string(&temp[0]);
}

bool LoggerBase::Logging(const CiLogEvent* log)
{
	fprintf(stderr, "[%s][%s %s][%s %s] %s\n"
		, log->GetEventTimeFullString().c_str()
		, log->GetWhoName().c_str(), log->GetSection().c_str()
		, log->GetLevel().c_str(), log->GetCode().c_str()
		, log->GetDescription().c_str()
		);

	return true;
}
