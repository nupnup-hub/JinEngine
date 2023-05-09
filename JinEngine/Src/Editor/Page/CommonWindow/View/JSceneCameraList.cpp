#include"JSceneCameraList.h" 
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../../../Object/Resource/Scene/JScene.h"
#include"../../../../Object/GameObject/JGameObject.h"
#include"../../../../Object/Component/JComponent.h"
#include"../../../../Object/Component/JComponentType.h"
#include"../../../../Object/Component/Camera/JCamera.h"
#include"../../../../Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Editor
	{
		void JSceneCameraList::DisplayCameraList(const JUserPtr<JScene>& scene, const std::string& unqLabel, const JVector2<float> size)noexcept
		{
			const ImGuiWindowFlags flag = ImGuiWindowFlags_NoTitleBar |
				ImGuiWindowFlags_AlwaysAutoResize |
				ImGuiWindowFlags_NoNav |
				ImGuiWindowFlags_NoSavedSettings |
				ImGuiWindowFlags_NoFocusOnAppearing | 
				ImGuiWindowFlags_NoMove;

			ImGui::SetCursorPosX(0);
			JImGuiImpl::BeginWindow(("##SceneListWindow" + unqLabel).c_str(), 0, flag);
			JImGuiImpl::BeginListBox(("##SceneListBox" + unqLabel).c_str(), size);

			auto compVec = scene->GetComponentVec(J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA);
			const uint vecCount = (uint)compVec.size();
			for(uint i = 0; i < vecCount; ++i)
			{
				if (compVec[i]->GetOwner()->IsEditorObject())
					continue;

				bool isSelected = i == preSelected;
				JUserPtr<JCamera> cam = Core::ConnectChildUserPtr<JCamera>(compVec[i]);
				if (JImGuiImpl::CheckBox(JCUtil::WstrToU8Str(cam->GetName()), isSelected))
				{
					if(isSelected)
						preSelected = i;
				}
			}

			JImGuiImpl::EndListBox();
			JImGuiImpl::EndWindow();
		}
		uint JSceneCameraList::GetSelecetdIndex()const noexcept
		{
			return preSelected;
		}
		JUserPtr<JCamera> JSceneCameraList::GetSelectedCam(const JUserPtr<JScene>& scene)const noexcept
		{
			auto compVec = scene->GetComponentVec(J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA);
			if (compVec.size() <= preSelected)
				return nullptr;
			else
			{
				if (canDisplayEditorCam && compVec[preSelected]->GetOwner()->IsEditorObject())
					return nullptr;
				return Core::ConnectChildUserPtr<JCamera>(compVec[preSelected]);
			}
		}
		void JSceneCameraList::SetSelecetdIndex(const uint selected)noexcept
		{
			preSelected = selected;
		}
		void JSceneCameraList::Update(const JUserPtr<JScene>& scene)noexcept
		{
			if (preSelected >= scene->GetComponetCount(J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA))
				preSelected = 0;
		}
	}
}