#include "Debug.h"
#include <cassert>
#include <cstdarg>
//#include <cstdio>
#include <sstream>
#include <ctime>
#include "StackWalker.h"
#include <sys/types.h>
#include <sys/timeb.h>
#ifndef DEBUG
#include <ShlObj.h>
#endif

namespace Log
{
	Debug* Debug::m_Instance = nullptr;
	void Debug::Create()
	{
#ifdef DEBUG
		m_Instance = new Debug();
		time_t now = time(0);
		struct tm tstruct;
		char buf[30];
		localtime_s(&tstruct, &now);
		strftime(buf, sizeof(buf), "%Y-%m-%d_%H_%M_%S", &tstruct);
		std::string logFolder = "log\\";
#ifdef _WIN32
		CreateDirectory(L"log", NULL);
#endif
		std::stringstream ss;
		ss << logFolder << buf << "_log.txt";
		m_Instance->m_Stream.open(ss.str().c_str());
		assert(m_Instance->m_Stream.is_open() && "Failed to open file!");
#endif
	}

	void Debug::Destroy()
	{
		if(m_Instance->m_Stream.is_open())
			m_Instance->m_Stream.close();

		delete m_Instance;
		m_Instance = nullptr;
	}

	Debug* Debug::GetInstance()
	{
		return m_Instance;
	}

	void Debug::WriteLog(const char* fmt, ...)
	{
		// Get time and store as string in buf
		time_t now = time(0);
		struct tm tstruct;
		char buf[30];
		localtime_s(&tstruct, &now);

		strftime(buf, sizeof(buf), "%H:%M:%S:", &tstruct);

		// Get Miliseconds and store in tStructMilli
		struct _timeb tstructMilli;
		char bufMilli[128];

		_strtime_s(bufMilli);
		_ftime_s(&tstructMilli);

		// Get VA_ARGS and store as string in buffer
		char buffer[4096];
		va_list args;
		va_start(args, fmt);
		vsprintf_s(buffer, fmt, args);
		perror(buffer);
		va_end(args);

		// Merge time and VA_ARGS into string and print to log-file
		std::stringstream ss;
		ss << "[" << buf << tstructMilli.millitm << "] " << buffer;

		m_Stream << ss.str().c_str() << std::endl;
		m_Stream.flush();
	}

	void Debug::PrintMessageVA(const char* fmt, ...)
	{
		char buffer[1024];
		va_list args;
		va_start(args, fmt);
		vsprintf_s(buffer, fmt, args);
		perror(buffer);
		va_end(args);
#ifdef _WIN32
		OutputDebugStringA(buffer); 
		OutputDebugStringA("\n");
#endif
		m_Stream << buffer << std::endl;
		m_Stream.flush(); /* maybe shouldn't flush ever call, only at end of frame? Could be more performant, but this gives the most data */
	}

	void Debug::AssertMessage(bool expr, const char* fileName, int line, const char* fncName, const char* str)
	{
		if(!expr)
			AssertMessage(fileName, line, fncName, str);
	}

	void Debug::AssertMessageVA(const char* fileName, int line, const char* fncName, const char* fmt, ...)
	{
		char buffer[1024];
		va_list args;
		va_start(args, fmt);
		vsprintf_s(buffer, fmt, args);
		perror(buffer);
		va_end(args);

		AssertMessage(fileName, line, fncName, buffer);
	}

	void Debug::AssertMessage(bool expr, const char* fileName, int line, const char* fncName, const std::string& str)
	{
		AssertMessage(expr, fileName, line, fncName, str.c_str());
	}

	void Debug::AssertMessage(const char* fileName, int line, const char* fncName, const std::string& str)
	{
		std::stringstream ss;
		ss << str << std::endl << fileName << std::endl << "Line: " << line << std::endl << "Function: " << fncName << std::endl;

		m_Stream << ss.str().c_str();
		m_Stream << std::endl << std::endl << "Callstack" << std::endl;

		StackWalker sw;
		sw.ShowCallstack();
		m_Stream.flush();
		const size_t len = ss.str().length() + 1;
		wchar_t* wc = new wchar_t[len];
		size_t tempSize;
		mbstowcs_s(&tempSize, wc, len, ss.str().c_str(), len);
		_wassert(wc, _CRT_WIDE(__FILE__), __LINE__);
		delete[] wc;
	}

	void Debug::DebugMessage(const char* fileName, int line, const char* fncName, const char* fmt, ...)
	{
		m_Stream << std::endl << "File: " << fileName << std::endl << "Line: " << line << std::endl << "Function: " << fncName << std::endl;
		char buffer[1024];
		va_list args;
		va_start(args, fmt);
		vsprintf_s(buffer, fmt, args);
		perror(buffer);
		va_end(args);

		m_Stream << buffer << std::endl;
		m_Stream.flush();
	}

}; // namespace Log
