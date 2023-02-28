#include"JEditorMultiSelectable.h" 

namespace JinEngine
{

	namespace Editor
	{ 
		bool JEditorMultiSelectable::IsMultiSelected(const size_t guid)noexcept
		{
			return selectedMap.find(guid) != selectedMap.end();
		}
		bool JEditorMultiSelectable::HasMultiSelected()const noexcept
		{
			return selectedMap.size() > 0;
		}
		void JEditorMultiSelectable::AddMultiSelected(const size_t guid, Core::JIdentifier* iden)noexcept
		{
			if (!IsMultiSelected(guid))
				selectedMap.emplace(guid, iden);
		}
		void JEditorMultiSelectable::RemoveMultiSelected(const size_t guid)noexcept
		{
			selectedMap.erase(guid); 
		}
		void JEditorMultiSelectable::ClearMultiSelected()noexcept
		{
			selectedMap.clear(); 
		}
		std::unordered_map<size_t, Core::JIdentifier*>& JEditorMultiSelectable::GetSelectedSet()noexcept
		{
			return selectedMap;
		}

	}
}