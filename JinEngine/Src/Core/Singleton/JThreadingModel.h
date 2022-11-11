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