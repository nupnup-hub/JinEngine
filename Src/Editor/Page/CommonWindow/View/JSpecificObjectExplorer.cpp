#include"JSpecificObjectExplorer.h" 
#include"../../JEditorAttribute.h"
#include"../../SkeletonaAssetSetting/JSkeletonSettingPageShareData.h" 
#include"../../../../../Lib/imgui/imgui.h"
#include"../../../../Object/GameObject/JGameObject.h"
#include"../../../../Core/Exception/JExceptionMacro.h"
#include"../../../../Utility/JCommonUtility.h"

//수정필요
namespace JinEngine
{
	namespace Editor
	{
		JSpecificObjectExplorer::JSpecificObjectExplorer(std::unique_ptr<JEditorAttribute> attribute,
			const size_t ownerPageGuid,
			JSkeletonSettingPageShareData* pageShareData)
			:JEditorWindow(std::move(attribute), ownerPageGuid), pageShareData(pageShareData)
		{

		}
		void JSpecificObjectExplorer::Initialize(JEditorUtility* editorUtility)noexcept
		{

		}
		void JSpecificObjectExplorer::UpdateWindow(JEditorUtility* editorUtility)
		{
			JEditorWindow::UpdateWindow(editorUtility);
			BuildObjectExplorer(editorUtility);
		}
		void JSpecificObjectExplorer::BuildObjectExplorer(JEditorUtility* editorUtility)
		{
			if (ImGui::TreeNodeEx(pageShareData->skeletonRoot->GetName().c_str(), ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_Selected))
			{
				ObjectExplorerOnScreen(pageShareData->skeletonRoot, editorUtility);
				ImGui::TreePop();
			}
		}
		void JSpecificObjectExplorer::ObjectExplorerOnScreen(JGameObject* obj, JEditorUtility* editorUtility)
		{
			const uint childrenCount = obj->GetChildrenCount();
			for (uint i = 0; i < childrenCount; ++i)
			{
				JGameObject* child = obj->GetChild(i);
				if (ImGui::TreeNodeEx(child->GetName().c_str(), ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_Selected))
				{
					ObjectExplorerOnScreen(child, editorUtility);
					ImGui::TreePop();
				}
			}
		}
	}
}