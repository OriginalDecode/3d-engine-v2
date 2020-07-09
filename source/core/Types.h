#pragma once

#if defined(_MSC_VER)
typedef unsigned __int8 uint8;
typedef __int8 int8;
typedef __int8* ptr;
#elif(_CLANG_)
typedef unsigned char uint8;
typedef unsigned char int8;
#elif(_GCC_)
typedef unsigned char uint8;
typedef char int8;
#endif

typedef char int8;

#if defined(_MSC_VER)
typedef unsigned __int16 uint16;
typedef __int16 int16;
#else
typedef unsigned short uint16;
typedef short int16;
#endif

#if defined(_MSC_VER)
typedef unsigned __int32 uint32;
typedef __int32 int32;
#else
typedef unsigned int uint32;
typedef signed int int32;
#endif

#if defined(_MSC_VER)
typedef unsigned __int64 uint64;
typedef unsigned __int64 uint64_ptr;
typedef __int64 int64_ptr;
typedef __int64 int64;
#else
typedef unsigned long long uint64;
typedef long long int64;
#endif
