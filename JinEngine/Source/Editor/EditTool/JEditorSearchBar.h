#pragma once
#include"JEditorInputBuffHelper.h"  

namespace JinEngine
{
	class JObject;
	namespace Editor
	{
		class JEditorSearchBar
		{ 
		private:
			std::unique_ptr<JEditorInputBuffHelper> inputHelper; 
		private:
			const size_t guid;
			const bool caseSensitive; 
			bool isUpdateInputData = false;
		public:
			JEditorSearchBar(const bool caseSensitive);
		public: 
			void ClearInputBuffer()noexcept;
		public:
			void UpdateSearchBar(const bool isReadonly = false);
		public:
			std::string GetInputData()const noexcept;
		public:
			bool CanSrcNameOnScreen(const std::string& srcName)const noexcept;
			bool CanSrcNameOnScreen(const std::wstring& srcName)const noexcept; 
			bool HasInputData()const noexcept;  
			bool IsUpdateInputData()const noexcept; 
		};
	}
}