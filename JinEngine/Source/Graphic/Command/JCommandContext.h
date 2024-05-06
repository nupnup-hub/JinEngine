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
#include"JCommandContextLog.h"
#include"../Device/JGraphicDeviceUser.h"
#include"../Thread/JGraphicThreadType.h"
#include"../../Core/JCoreEssential.h"
#include"../../Core/Time/JStopWatch.h"
#include"../../Core/Singleton/JSingletonHolder.h" 

namespace JinEngine
{
	namespace Graphic
	{
		class JCommandContextManager;
		class JCommandContext : public JGraphicDeviceUser
		{
		private:
			friend class JCommandContextManager;
		private:
			std::string name;
		private:
			JCommandContextLog log;
			Core::JStopWatch stopWatch;
			uint logUpdateCount = 0;
		private: 
			bool isAlwausActivated = false;
			bool isLastFrameUpdated = false;
			bool canUse = true; 
			bool canWriteLog = true;
		public:
			JCommandContext(const std::string& name, const bool isAlwausActivated = false);
			virtual ~JCommandContext() = default;
		public:
			virtual bool Begin();
			virtual void End(); 
		public:
			JCommandContextLog GetContextLog()const noexcept;
		protected:
			void SetUseTrigger(const bool value);
		public:
			void SetWriteLogTrigger(const bool value);
		public:
			bool CanUse()const noexcept; 
		};
		 
		class JCommandContextManager
		{  
		private:
			std::vector<std::unique_ptr<JCommandContext>> context; 
		public:
			~JCommandContextManager();
		public:
			std::vector<JCommandContextLog> GetLog();
		public:
			JCommandContext* PushCommandContext(std::unique_ptr<JCommandContext>&& newContext);
		public:
			void Begin();
			void End();
		public:
			void Clear();
		}; 
	}
}