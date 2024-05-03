#pragma once
#include"../JCoreEssential.h"

namespace JinEngine
{
	namespace Core
	{
		class JStorageInterface
		{
		private:
			bool clearRequest = false;
		public:
			void SetClearTrigger()noexcept;
			void OffClearTrigger()noexcept; 
		public:
			bool HasClearRequest()const noexcept;
		};

		class JStorageUpdateInterface
		{
		private:
			uint updateCount = 0;
		private:
			int waitFrame = 0;
		public:
			uint GetUpdateCount()const noexcept;
		public:  
			void SetWaitFrame(const uint frameCount)noexcept;
		public:
			void AddUpdateCount()noexcept;
			void MinusWaitFrame()noexcept;
		public:
			bool HasWaitFrame()const noexcept;
		};

		class JVolatileStorageInterface : public JStorageInterface, public JStorageUpdateInterface
		{
		private:
			bool canAlive = false;
		public:
			void SetAliveTrigger()noexcept;
			void OffAliveTrigger()noexcept;
		public:
			bool CanAlive()const noexcept;
		};
	}
}