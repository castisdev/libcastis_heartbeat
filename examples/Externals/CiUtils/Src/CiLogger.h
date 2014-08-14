// CiLogger.h: implementation of the CiLogger class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __CIUTILS_CILOGGER__
#define __CIUTILS_CILOGGER__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if _WIN32
#pragma warning (disable : 4786)
#endif

#include <string>
#include <list>
#include <vector>

#ifndef _WIN32
#include <stdarg.h>
#endif

#include "time.h"
#include "CiThread2.h"

#include <boost/scoped_ptr.hpp>

#include <locale>

#include <fstream>

namespace CiUtils
{
	class CiLogEvent;
	class CiLogger;
};

namespace CiUtils
{

class CiLogEvent
{
public:
	enum level_type_t { eNone=1, eDebug=2, eReport=4, eInformation=8, eSuccess=16, eWarning=32, eError=64, eFail=128, eException=256 };

protected:
	struct tm _loggingTime;
	std::string _section;
	std::string _description;
	std::string _level; //"Report", "Information” or "Warning” or "Error” or "Fail, "Success", "Debug", "Exception"
	level_type_t _levelType;
	std::string _code;
	std::string _whoName;
	std::string _whoID;
	int _millisec;

public:
	CiLogEvent();
	virtual ~CiLogEvent();
	CiLogEvent(const char* description, level_type_t levelType, const char* section="", const char* code="", const char* whoName="", const char* whoID="");
	CiLogEvent(const char* description, const char* level="", const char* section="", const char* code="", const char* whoName="", const char* whoID="");

	tm GetEventTime() const { return _loggingTime; }

	std::string GetEventTimeFullString() const;
	std::string GetEventDateString() const;
	std::string GetEventTimeString() const;
	std::string GetSection() const { return _section; }
	std::string GetCode() const { return _code; }
	std::string GetDescription() const { return _description; }
	std::string GetWhoName() const { return _whoName; }
	std::string GetWhoID() const { return _whoID; }

	virtual int GetLevelType() const { return _levelType; }
	virtual std::string GetLevel() const { return _level; }

	static std::string GetLevelTypeString(int levelType);
	static level_type_t GetLevelType(const char* levelString);
};


class CiLogger  : public CCiThread2
{

public:

	class LogQueue
	{
	private:
		std::vector<CiLogEvent*> _queue;
		CCiSemaphore _lock;

	public:
		LogQueue() {
			_lock.Initialize(1);
		}

		virtual ~LogQueue()
		{
			_lock.Finalize();
		}

		std::vector<CiLogEvent*> Get(/*int size=-1*/)
		{
			_lock.Wait();

			std::vector<CiLogEvent*> logs;

			logs = _queue;
			std::vector<CiLogEvent*>().swap(_queue);

			_lock.Post();

			return logs;
		}

		void Put(CiLogEvent* msg)
		{
			_lock.Wait();
			_queue.push_back(msg);
			_lock.Post();
		}

		int Size() {
			_lock.Wait();

			int size = static_cast<int>(_queue.size());
			_lock.Post();

			return size;
		}

	};

	enum log_format_t {eXML, eCSV};
	enum path_type_t {eAbsolute, eRelative};
	enum log_mode_t {eBySize=0, eByTime};

public:
	CiLogger();
	virtual ~CiLogger();

public:
	virtual bool InitInstance();
	virtual bool ExitInstance();
	virtual bool Run();

	// functions
	// 아래 함수 일곱 개는 thread safe 하지 않다.
	// fileSizeLimit 를 0보다 작은 값을 넣으면 file size 제한을 하지 않는다.
																																			// docType = "UTF-8" or "euc_kr"
	void SetEnv(CiLogger::path_type_t pathType, const std::string& loggingFolder, const std::string& loggingFileName, const std::string& ext, const std::string& docType="euc_kr", long fileSizeLimit=1024*1024/*Byte*/, CiLogger::log_mode_t logMode=eBySize);
	void SetValidEventLevel(int validLevels);
	void SetMessageProcessingSize(int size ) { _processingSize = size; }
	void SetSleepPeriod(int millisec) { _sleepPeriod = millisec; }
	void SetDisplayScreen(bool flag) { _displayScreenFlag = flag; }
	void SetMakeFolderForEachMonth(bool flag) { _makeFolderForEachMonthFlag = flag; }
	void SetLogMode(CiLogger::log_mode_t logMode) {_logMode = logMode; }

	void Write(const std::string& who, const std::string& whoID, CiLogEvent::level_type_t level, const std::string& section, const std::string& code, const std::string& description);
	void Write(const std::string& who, const std::string& whoID, CiLogEvent::level_type_t level, const std::string& section, const std::string& description);
	void Write(const std::string& who, const std::string& whoID, CiLogEvent::level_type_t level, const std::string& description);
	void Write(const std::string& who, const std::string& whoID, const std::string& description);
	void Write(const std::string& description);

	void Write(const std::string& who, const std::string& whoID, CiLogEvent::level_type_t level, const std::string& section, const std::string& code, char* format, ...); // 1024 limit

	void WriteWOCode(const std::string& who, const std::string& whoID, CiLogEvent::level_type_t level, const std::string& section, char* format, ...); // 1024 limit
	void WriteWOSection(const std::string& who, const std::string& whoID, CiLogEvent::level_type_t level, char* format, ...); // 1024 limit
	void WriteWOLevel(const std::string& who, const std::string& whoID, char* format, ...); // 1024 limit
	void WriteWOWho(char* format, ...); // 1024 limit


	std::string GetLastError() { return _error; }

	virtual void OnDisplayScreen(const char* logText, const CiLogEvent* log);

protected:
	virtual std::string GetCurrentFileName(const tm& time);
	virtual std::string GetCurrentFileNameBySize(const tm& time);
	virtual std::string GetCurrentFileNameByTime(const tm& time);
	virtual std::string GetCurrentFolder(const tm& time);
	virtual bool ProcessLogEvent();

	void SleepThread(int period = 1000);	/// milli
	virtual bool Logging(const CiLogEvent* log);
	bool LoggingHeaderAndTail(const std::string& fullPath);
	int GetTailSize() const;
	virtual void LookupLatestLogFile();

	virtual void SetDefaultValidEventLevel();
	virtual bool ValidateEventLevel(int level);
	int GetValidEventLevel() const { return _validEventLevels; }

	std::string GetXMLSafeString(const std::string& str);
	std::string GetCSVSafeString(const std::string& str);

	static bool dec(int left, int right) { return left > right; }
	static tm GetCurrentTime();

protected:
	std::string _loggingFolder; // logging folder
	std::string _loggingFileName;
	std::string _loggingFileNameExt;
	std::string _docType;

	CiLogger::log_mode_t _logMode;
	CiLogger::log_format_t _logFormat;

	int _validEventLevels;

	long _loggingFileSizeLimit;

	std::string _error;

	LogQueue _messageQueue;
	int _processingSize;
	int _sleepPeriod; // milli

	long _currentLoggingFileSize;
	std::string _currentLoggingFileName;
	std::string _currentLoggingDateString;
	int _currentLoggingFileIndex;

	bool _displayScreenFlag;
	bool _makeFolderForEachMonthFlag;

	bool _bfirsttime;

	//김건우
	std::fstream _out;
	bool _bChangeFile;
};

std::string where(const std::string& file, const std::string& function, int line);

class LoggerFactory;

class LoggerBase	: public CiUtils::CiLogger
{
public:
	virtual ~LoggerBase() {}
	static LoggerBase& Instance();

	void SetWhoName(const std::string& whoName) { _whoName = whoName; }
	void SetWhoID(const std::string& whoID) { _whoID = whoID; }

	void debug(const std::string& section, int code, const char* format, ...);
	void debug(const std::string& section, const char* format, ...);

	void report(const std::string& section, int code, const char* format, ...);
	void report(const std::string& section, const char* format, ...);

	void info(const std::string& section, int code, const char* format, ...);
	void info(const std::string& section, const char* format, ...);

	void success(const std::string& section, int code, const char* format, ...);
	void success(const std::string& section, const char* format, ...);

	void warning(const std::string& section, int code, const char* format, ...);
	void warning(const std::string& section, const char* format, ...);

	void error(const std::string& section, int code, const char* format, ...);
	void error(const std::string& section, const char* format, ...);

	void fail(const std::string& section, int code, const char* format, ...);
	void fail(const std::string& section, const char* format, ...);

	void exception(const std::string& section, int code, const char* format, ...);
	void exception(const std::string& section, const char* format, ...);

//2009.04.01 추가 - 유니코드

	void debug(const std::string& section, int code, const wchar_t* wFormat, ...);
	void debug(const std::string& section, const wchar_t* wFormat, ...);

	void report(const std::string& section, int code, const wchar_t* wFormat, ...);
	void report(const std::string& section, const wchar_t* wFormat, ...);

	void info(const std::string& section, int code, const wchar_t* wFormat, ...);
	void info(const std::string& section, const wchar_t* wFormat, ...);

	void success(const std::string& section, int code, const wchar_t* wFormat, ...);
	void success(const std::string& section, const wchar_t* wFormat, ...);

	void warning(const std::string& section, int code, const wchar_t* wFormat, ...);
	void warning(const std::string& section, const wchar_t* wFormat, ...);

	void error(const std::string& section, int code, const wchar_t* wFormat, ...);
	void error(const std::string& section, const wchar_t* wFormat, ...);

	void fail(const std::string& section, int code, const wchar_t* wFormat, ...);
	void fail(const std::string& section, const wchar_t* wFormat, ...);

	void exception(const std::string& section, int code, const wchar_t* wFormat, ...);
	void exception(const std::string& section, const wchar_t* wFormat, ...);

	std::string wstr2string(const wchar_t* wstr, std::locale const& loc = std::locale(""));

protected:
	LoggerBase() {}
	virtual bool Logging(const CiLogEvent* log);

protected:
	static boost::scoped_ptr<LoggerBase> _instance;
	std::string _whoName;
	std::string _whoID;
};

} // namespace end

#define WHERE() CiUtils::where(__FILE__, __FUNCTION__, __LINE__)
#define theLogger CiUtils::LoggerBase::Instance()

#endif // __CIUTILS_CILOGGER__

