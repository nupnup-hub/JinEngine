#pragma once
#include"../../Core/JCoreEssential.h"
#include<string>
namespace JinEngine
{
	namespace Editor
	{
		class JEditorPageCounter
		{
		private:
			uint pageIndex = 0; 
		private:
			uint pagePerCount = 0;
			uint pageStartIndex = 0;
			uint pageEndIndex = 0;
		private:
			uint count = 0;
		public:
			/**
			* @return if selected other page
			*/
			bool DisplayUI(const std::string& uniqueLabel);
		public:
			uint GetPageInedx()const noexcept;
			uint GetMaxPageIndex()const noexcept;
		public:
			void SetPageIndex(uint value);
			void SetPagePerCount(uint value);
		public:
			void BeginCounting();
			void EndCounting();
			void Count();
			bool IsValidIndex();
		private:
			void UpdateData();
		};
	}
}