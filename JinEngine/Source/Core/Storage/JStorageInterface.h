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