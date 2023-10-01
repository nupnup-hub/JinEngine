#include"JSceneCameraList.h" 
#include"../../../Gui/JGui.h"
#include"../../../../Core/Utility/JCommonUtility.h"
#include"../../../../Object/Resource/Scene/JScene.h"
#include"../../../../Object/GameObject/JGameObject.h"
#include"../../../../Object/Component/JComponent.h"
#include"../../../../Object/Component/JComponentType.h"
#include"../../../../Object/Component/Camera/JCamera.h"

namespace JinEngine
{
	namespace Editor
	{
		void JSceneCameraList::DisplayCameraList(const JUserPtr<JScene>& scene, const std::string& unqLabel, const JVector2<float> size)noexcept
		{ 
			const J_GUI_WINDOW_FLAG_ flag = J_GUI_WINDOW_FLAG_NO_TITLE_BAR |
				J_GUI_WINDOW_FLAG_AUTO_RESIZE |
				J_GUI_WINDOW_FLAG_NO_NAV |
				J_GUI_WINDOW_FLAG_NO_SAVE |
				J_GUI_WINDOW_FLAG_NO_FOCUS_ON_APPEARING | 
				J_GUI_WINDOW_FLAG_NO_MOVE;

			JGui::SetCursorPosX(0);
			JGui::BeginWindow(("##SceneListWindow" + unqLabel), 0, flag);
			JGui::BeginListBox(("##SceneListBox" + unqLabel), size);

			auto compVec = scene->GetComponentVec(J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA);
			const uint vecCount = (uint)compVec.size();
			for(uint i = 0; i < vecCount; ++i)
			{
				if (compVec[i]->GetOwner()->IsEditorObject())
					continue;

				bool isSelected = i == preSelected;
				JUserPtr<JCamera> cam = Core::ConnectChildUserPtr<JCamera>(compVec[i]);
				if (JGui::CheckBox(JGui::CreateGuiLabel(cam, "JSceneCameraListCheckBox"), isSelected))
				{
					if(isSelected)
						preSelected = i;
				}
			}

			JGui::EndListBox();
			JGui::EndWindow();
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