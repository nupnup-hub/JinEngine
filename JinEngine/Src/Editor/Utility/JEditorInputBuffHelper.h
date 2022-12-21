#pragma once
#include<string>
namespace JinEngine
{
	namespace Editor
	{
		class JEditorInputBuffHelper
		{
		public:
			std::string buff;
			std::string result;
			const int capacity;
		public:
			JEditorInputBuffHelper(const int capacity);
		public:
			void Clear();
		public:
			//Get valid value in buff
			std::string GetResult()const noexcept;
			int GetCapactiy()const noexcept;
			void SetBuff(const std::string& value)noexcept;
		};
	}
}