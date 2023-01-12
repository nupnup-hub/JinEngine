#include"JProjectCloseConfirm.h"
#include"../../GuiLibEx/ImGuiEx/JImGuiImpl.h"

namespace JinEngine
{
	namespace Editor
	{
		void JProjectCloseConfirm::Update(bool& isPressedClose, bool& isPressedCancel)
		{
			bool isOpen = true;
			ImGui::Begin("CloseWindow", &isOpen, ImGuiWindowFlags_AlwaysAutoResize);
			if (ImGui::Button("Close"))
				isPressedClose = true;			 
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
				isPressedCancel = true;
			ImGui::SameLine();
			ImGui::End();
		}
	}
}