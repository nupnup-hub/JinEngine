#pragma once
#include<unordered_map> 
namespace JinEngine
{ 
	namespace Core
	{
		class JIdentifier;
	}
	namespace Editor
	{ 
		class JEditorMultiSelectable
		{
		private:
			std::unordered_map<size_t, Core::JIdentifier*> selectedMap;
		public:
			//if seleted object return true and add nowIndex + 1
			bool IsMultiSelected(const size_t guid)noexcept;
			bool HasMultiSelected()const noexcept;
		public:
			void AddMultiSelected(const size_t guid, Core::JIdentifier* iden)noexcept;
			void RemoveMultiSelected(const size_t guid)noexcept;
			void ClearMultiSelected()noexcept;
		public:
			std::unordered_map<size_t, Core::JIdentifier*>& GetSelectedSet()noexcept;
		};
	}
}