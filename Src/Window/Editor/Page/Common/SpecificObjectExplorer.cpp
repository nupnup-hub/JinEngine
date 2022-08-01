#include"SpecificObjectExplorer.h" 
#include"../EditorAttribute.h"
#include"../SkeletonaAssetSetting/SkeletonAssetSettingPageShareData.h" 
#include"../../../../../Lib/imgui/imgui.h"
#include"../../../../Object/GameObject/JGameObject.h"
#include"../../../../Core/Exception/JExceptionMacro.h"
#include"../../../../Utility/JCommonUtility.h"

//수정필요
namespace JinEngine
{
	SpecificObjectExplorer::SpecificObjectExplorer(std::unique_ptr<EditorAttribute> attribute, 
		const size_t ownerPageGuid,
		SkeletonAssetSettingPageShareData* pageShareData)
		:EditorWindow(std::move(attribute), ownerPageGuid), pageShareData(pageShareData)
	{

	}
	void SpecificObjectExplorer::Initialize(EditorUtility* editorUtility)noexcept
	{

	}
	void SpecificObjectExplorer::UpdateWindow(EditorUtility* editorUtility)
	{
		EditorWindow::UpdateWindow(editorUtility);
		BuildObjectExplorer(editorUtility);
	}
	void SpecificObjectExplorer::BuildObjectExplorer(EditorUtility* editorUtility)
	{ 
		if (ImGui::TreeNodeEx(pageShareData->skeletonRoot->GetName().c_str(), ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_Selected))
		{		 
			ObjectExplorerOnScreen(pageShareData->skeletonRoot, editorUtility);
			ImGui::TreePop();
		} 
	}
	void SpecificObjectExplorer::ObjectExplorerOnScreen(JGameObject* obj, EditorUtility* editorUtility)
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