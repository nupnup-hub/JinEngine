#pragma once
#include"JLazyDestructionInfo.h"
#include"../Module/JModuleMacro.h"
#include<vector>
#include<memory>

namespace JinEngine
{
	namespace Core
	{ 
		class JLazyDestruction
		{
		private:
			class JLazyDestructionImpl;
		private:
			std::unique_ptr<JLazyDestructionImpl> impl;
		public:
			JLazyDestruction(std::unique_ptr<JLazyDestructionInfo>&& info);
			~JLazyDestruction();
		public:
			void Update(const float timeOffset = 0)noexcept;
			void Clear()noexcept;
		public:
			bool AddUser(JTypeBase* ptr)noexcept;
			bool RemoveUser(JTypeBase* ptr)noexcept;
		};
	}
}