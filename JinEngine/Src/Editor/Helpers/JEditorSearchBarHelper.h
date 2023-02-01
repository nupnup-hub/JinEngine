#pragma once
#include"JEditorInputBuffHelper.h"
#include"../../Core/Pointer/JOwnerPtr.h" 
#include<memory>
#include<string>
namespace JinEngine
{
	class JObject;
	namespace Editor
	{
		class JEditorSearchBarHelper
		{ 
		private:
			std::unique_ptr<JEditorInputBuffHelper> inputHelper; 
		private:
			const bool caseSensitive; 
			bool isUpdateInputData = false;
		public:
			JEditorSearchBarHelper(const bool caseSensitive);
		public: 
			void ClearInputBuffer()noexcept;
		public:
			void UpdateSearchBar(const std::string& uniqueLabel, const bool isReadonly);
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