/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#pragma once
#include <stdio.h> 
#include"../JCoreEssential.h"

#if defined(_WIN32) || defined(_WIN64)
#if defined(_WIN32) 
#define OS_WINDOW_32 1
#else
#define OS_WINDOW_32 0
#endif

#if defined(_WIN64)
#define OS_WINDOW_64 1
#else
#define OS_WINDOW_64 0
#endif
#if defined(__CYGWIN__) && !defined(_WIN32)
#define OS_CYGWINOW 1
#else
#define OS_CYGWINOW 0
#endif
#define OS_WINDOW 1
#define PLATFORM_NAME "Window"
#else
#define OS_WINDOW 0
#endif


#if defined(__ANDROID__)
#define OS_ANDROID 1
#define PLATFORM_NAME "Android"
#else
#define OS_ANDROID 0
#endif

#if defined(__linux__)
#define OS_LINUX 1
#define PLATFORM_NAME "Linux"
#else
#define OS_LINUX 0
#endif

#if defined(__unix__) || !defined(__APPLE__) && defined(__MACH__) && defined(BSD)
#define OS_BSD 1
#define PLATFORM_NAME "BSD"
#else
#define OS_BSD 0
#endif

#if defined(__hpux)
#define OS_HP-UX 1
#define PLATFORM_NAME "HP-UX"
#else
#define OS_HP_UX 0
#endif

#if defined(_AIX)
#define OS_AIX 1
#define PLATFORM_NAME "AIX"
#else
#define OS_AIX 0
#endif

#if defined(__APPLE__) && defined(__MACH__)
#if TARGET_IPHONE_SIMULATOR == 1
#define OS_IPHONE 1
#else
#define OS_IPHONE 0
#endif

#if TARGET_OS_IPHONE == 1
#define OS_IPHONE 1
#else
#define OS_IPHONE 0
#endif

#if TARGET_OS_MAC == 1
#define OS_MAC  1
#else
#define OS_MAC  0
#endif
#define PLATFORM_NAME "IOS"
#endif


#if defined(__sun) && defined(__SVR4)
#define OS_SOLARIS 1
#define PLATFORM_NAME "Solaris"
#else
#define OS_SOLARIS 0
#endif

#define MS_ALIGN(n) __declspec(align(n))

namespace JinEngine
{
	namespace Core
	{ 
		// Return a name of platform, if determined, otherwise - an empty string
		static std::string GetOsName()
		{
			return (PLATFORM_NAME == NULL) ? "" : PLATFORM_NAME;
		}
		static std::string GetSolutionPlatform()
		{
			if (_WIN64)
				return "x64";
			else if (_WIN32)
				return "x86";
			else
			{
				//´Ù¸¥ ÇÃ·§Æû ±¸ÇöÇÊ¿ä
				return "Invalid";
			}
		}
	}
}

/*
		#if defined(__unix__) || defined(__unix)
		#define EIGEN_OS_UNIX 1
		#else
		#define EIGEN_OS_UNIX 0
		#endif
		/// \internal EIGEN_OS_LINUX set to 1 if the OS is based on Linux kernel
		#if defined(__linux__)
		#define EIGEN_OS_LINUX 1
		#else
		#define EIGEN_OS_LINUX 0
		#endif
		/// \internal EIGEN_OS_ANDROID set to 1 if the OS is Android
		// note: ANDROID is defined when using ndk_build, __ANDROID__ is defined when using a standalone toolchain.
		#if defined(__ANDROID__) || defined(ANDROID)
		#define EIGEN_OS_ANDROID 1
		#else
		#define EIGEN_OS_ANDROID 0
		#endif
		/// \internal EIGEN_OS_GNULINUX set to 1 if the OS is GNU Linux and not Linux-based OS (e.g., not android)
		#if defined(__gnu_linux__) && !(EIGEN_OS_ANDROID)
		#define EIGEN_OS_GNULINUX 1
		#else
		#define EIGEN_OS_GNULINUX 0
		#endif

		/// \internal EIGEN_OS_BSD set to 1 if the OS is a BSD variant
		#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__DragonFly__)
		#define EIGEN_OS_BSD 1
		#else
		#define EIGEN_OS_BSD 0
		#endif

		/// \internal EIGEN_OS_MAC set to 1 if the OS is MacOS
		#if defined(__APPLE__)
		#define EIGEN_OS_MAC 1
		#else
		#define EIGEN_OS_MAC 0
		#endif

		/// \internal EIGEN_OS_QNX set to 1 if the OS is QNX
		#if defined(__QNX__)
		#define EIGEN_OS_QNX 1
		#else
		#define EIGEN_OS_QNX 0
		#endif

		/// \internal EIGEN_OS_WIN set to 1 if the OS is Windows based
		#if defined(_WIN32)
		#define EIGEN_OS_WIN 1
		#else
		#define EIGEN_OS_WIN 0
		#endif

		/// \internal EIGEN_OS_WIN64 set to 1 if the OS is Windows 64bits
		#if defined(_WIN64)
		#define EIGEN_OS_WIN64 1
		#else
		#define EIGEN_OS_WIN64 0
		#endif

		/// \internal EIGEN_OS_WINCE set to 1 if the OS is Windows CE
		#if defined(_WIN32_WCE)
		#define EIGEN_OS_WINCE 1
		#else
		#define EIGEN_OS_WINCE 0
		#endif

		/// \internal EIGEN_OS_CYGWIN set to 1 if the OS is Windows/Cygwin
		#if defined(__CYGWIN__)
		#define EIGEN_OS_CYGWIN 1
		#else
		#define EIGEN_OS_CYGWIN 0
		#endif

		/// \internal EIGEN_OS_WIN_STRICT set to 1 if the OS is really Windows and not some variants
		#if EIGEN_OS_WIN && !( EIGEN_OS_WINCE || EIGEN_OS_CYGWIN )
		#define EIGEN_OS_WIN_STRICT 1
		#else
		#define EIGEN_OS_WIN_STRICT 0
		#endif

		/// \internal EIGEN_OS_SUN set to 1 if the OS is SUN
		#if (defined(sun) || defined(__sun)) && !(defined(__SVR4) || defined(__svr4__))
		#define EIGEN_OS_SUN 1  
		#else
		#define EIGEN_OS_SUN 0
		#endif

		/// \internal EIGEN_OS_SOLARIS set to 1 if the OS is Solaris
		#if (defined(sun) || defined(__sun)) && (defined(__SVR4) || defined(__svr4__))
		#define EIGEN_OS_SOLARIS 1 
		#else
		#define EIGEN_OS_SOLARIS 0		
		#endif
*/