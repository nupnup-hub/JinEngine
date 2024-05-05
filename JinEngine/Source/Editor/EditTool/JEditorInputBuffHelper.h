#pragma once 
#include"../../Core/JCoreEssential.h"

namespace JinEngine
{
	namespace Editor
	{
		class JEditorInputBuffHelper
		{
		public:
			std::string buff;
			std::string result;
		private:
			int capacity; 
		public:
			JEditorInputBuffHelper(const int capacity);
		public:
			void Clear();
		public:
			//Get valid value in buff
			std::string GetResult()const noexcept;
			int GetCapactiy()const noexcept;
			void SetCapacity(const int newCapacity)noexcept;
			void SetBuff(const std::string& value)noexcept; 
		};
	}
}