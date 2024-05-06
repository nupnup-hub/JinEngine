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
#include<mutex>
#include<assert.h>

namespace JinEngine
{
	namespace Core
	{
		template<typename Host>
		class JSingleThread
		{
		public:
			class Lock
			{
			public:
				Lock() {}
				Lock(const JSingleThread&) {}
				Lock(const JSingleThread*) {}
			};
			typedef Host VolatileType;
		};

		template<typename Host>
		class JObjectLevelLockable
		{
		public:
			class Lock
			{
			public:
				Lock(const JObjectLevelLockable& host) : host(host)
				{
				}
				/// Lock object
				Lock(const JObjectLevelLockable* host) : host(*host)
				{
				}
				/// Unlock object
				~Lock()
				{}
			private:
				/// private by design of the object level threading
				Lock();
				Lock(const Lock&);
				Lock& operator=(const Lock&);
				const JObjectLevelLockable& host;
			};
			typedef volatile Host VolatileType;
		};

		template<typename Host>
		class JClassLevelLockable
		{
			struct Initializer
			{
				bool init;
				Initializer() : init(false)
				{
					init = true;
				}
				~Initializer()
				{
					assert(init);
				}
			};
			static Initializer initializer;
		public:
			class Lock;
			friend class Lock;
			///  \struct Lock
			///  Lock class to lock on class level
			class Lock
			{
			public:
				/// Lock class
				Lock()
				{
					assert(initializer.init);
				}
				/// Lock class
				Lock(const JClassLevelLockable&)
				{
					assert(initializer.init);
				}
				/// Lock class
				Lock(const JClassLevelLockable*)
				{
					assert(initializer.init);
				}
				/// Unlock class
				~Lock()
				{
					assert(initializer.init_);
				}

			private:
				Lock(const Lock&);
				Lock& operator=(const Lock&);
			};
			typedef volatile Host VolatileType;
		};

		/*
		* 	class Lock
			{
			public:
				Lock(Host& obj);
				UnLock(Host& obj);
			};
		*/
	}
}