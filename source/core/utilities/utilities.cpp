#include "utilities.h"
#include "Core/hash/Murmur3.h"
#ifdef _WIN32
#include <Windows.h>
#endif
namespace Core
{

	float RadToDegree(float rad)
	{
		return (rad * 180.f) / 3.1415926535f;
	}

	float DegreeToRad(float degree)
	{
		return (degree * 3.1415926535f) / 180.f;
	}

	std::wstring ToWString(const std::string& str)
	{
		return std::wstring(str.begin(), str.end());
	}

	std::string ToString(const std::wstring& str)
	{
		return std::string(str.begin(), str.end());
	}

	uint64 Hash(const std::string& str)
	{
		uint64 result;
		MurmurHash3_x86_32(str.c_str(), (int32)str.length(), 0, &result);
		return result;
	}

	uint64 Hash(const std::string& str, uint32 seed)
	{
		uint64 result;
		MurmurHash3_x86_32(str.c_str(), (int32)str.length(), seed, &result);
		return result;
	}

	void DebugPrintLastError()
	{
#ifdef _WIN32
		DWORD error = GetLastError();
		LPWSTR lpMsgBuf = nullptr;
		size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error,
									MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&lpMsgBuf, 0, NULL);

		std::wstring message(lpMsgBuf, size);
		OutputDebugStr(ToString(message).c_str());
#endif
	}

	void OutputDebugStr(const char* fmt, ...)
	{
		char buffer[1024];
		va_list args;
		va_start(args, fmt);
		snprintf(buffer, 1024, fmt, args);
		perror(buffer);
		va_end(args);
#ifdef _WIN32
		OutputDebugStringA(fmt);
#endif
	}

	void OutputDebugStr(const std::string& str)
	{
		OutputDebugStr(str.c_str());
	}

}; // namespace Core
