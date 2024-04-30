#pragma once     
#include"JCommandContextLog.h"
#include"../Device/JGraphicDeviceUser.h"
#include"../Thread/JGraphicThreadType.h"
#include"../../Core/JCoreEssential.h"
#include"../../Core/Time/JStopWatch.h"
#include"../../Core/Singleton/JSingletonHolder.h"
#include<string>
#include<memory>
#include<vector>

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