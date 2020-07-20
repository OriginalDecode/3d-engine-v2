#pragma once
#ifdef _WIN32
#include <windows.h>
#endif
#include "StackWalker\StackWalker.h"

namespace Log
{
	class StackWalker : public BaseStackWalker
	{
	public:
		StackWalker();
		StackWalker( unsigned int aProcessId, HANDLE aProcess );
		virtual void OnOutput( char* aString );
	};

}; // namespace Log
