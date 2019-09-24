#pragma once

#ifdef RELEASE_BUILD

#define ASSERT_VA( ... )
#define ASSERT_OVERRIDE( string )
#define ASSERT( expression, string )

#define LOG_MESSAGE( ... )

#define LOG_DEBUG( ... ) Log::Debug::GetInstance()->DebugMessage( __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__ )

#else

#define ASSERT_VA( ... ) Log::Debug::GetInstance()->AssertMessageVA( __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__ )
#define ASSERT( expression, string ) \
	Log::Debug::GetInstance()->AssertMessage( expression, __FILE__, __LINE__, __FUNCTION__, string )

#define ASSERT_OVERRIDE( string ) Log::Debug::GetInstance()->AssertMessage( __FILE__, __LINE__, __FUNCTION__, string )

#define LOG_MESSAGE( ... ) Log::Debug::GetInstance()->PrintMessageVA( __VA_ARGS__ )

#define LOG_DEBUG( ... ) Log::Debug::GetInstance()->DebugMessage( __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__ )

#endif

#include <string>
#include <fstream>
#include "Assert.h"
#include "StackWalker.h"

namespace Log
{
	class Debug
	{
	public:
		static void Create();
		static void Destroy();
		static Debug* GetInstance();

		void WriteLog( const char* fmt, ... );

		void PrintMessageVA( const char* fmt, ... );

		void DebugMessage( const char* fileName, int line, const char* fncName, const char* fmt, ... );

		void AssertMessageVA( const char* fileName, int line, const char* fncName, const char* fmt, ... );

		void AssertMessage( bool expr, const char* fileName, int line, const char* fncName, const char* str );
		void AssertMessage( bool expr, const char* fileName, int line, const char* fncName, const std::string& str );
		void AssertMessage( const char* fileName, int line, const char* fncName, const std::string& str );

		// void ShowMessageBox( HWND hwnd, LPCSTR text, LPCSTR title, UINT type );

	private:
		Debug() = default;
		~Debug() = default;
		static Debug* m_Instance;
		std::ofstream m_Stream;
		// std::unordered_map<eFilterLog, std::pair<bool, std::string>> myFilterLogStatus;
	};
} // namespace Log
