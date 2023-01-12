#include"JEditorUpdateCondition.h"

namespace JinEngine
{
	namespace Editor
	{
		JEditorPageUpdateCondition::JEditorPageUpdateCondition(const bool canClickPage)
			:canClickPage(canClickPage)
		{}
		JEditorWindowUpdateCondition::JEditorWindowUpdateCondition(const bool canClickWindow)
			: canClickWindow(canClickWindow)
		{}

		JEditorWindowUpdateCondition JEditorPageUpdateCondition::CreateWindowCondition()const noexcept
		{
			return JEditorWindowUpdateCondition(canClickPage);
		}
	}
}