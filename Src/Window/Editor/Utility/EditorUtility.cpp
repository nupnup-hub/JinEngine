#include"EditorUtility.h"    
#include"../../JWindows.h"
#include"../../../Lib/imgui/imgui.h"

namespace JinEngine
{
	EditorUtility::EditorUtility()
	{
		auto lam = [](const size_t& a, const size_t& b) {return a == b; };
		RegistIdenCompareCallable(lam);
	}
	EditorUtility::~EditorUtility() 
	{
		this->ClearEvent();
	}
	EditorUtility::JEventInterface* EditorUtility::EvInterface()noexcept
	{
		return this;
	}
	void EditorUtility::UpdateWindoeData()
	{
		displayWidth = JWindow::Instance().GetDisplayWidth();
		displayHeight = JWindow::Instance().GetDisplayHeight();
		clientPositionX = JWindow::Instance().GetClientPositionX();
		clientPositionY = JWindow::Instance().GetClientPositionY();
		clientWidth = JWindow::Instance().GetClientWidth();
		clientHeight = JWindow::Instance().GetClientHeight();
	}
	void EditorUtility::UpdateEditorTextSize()
	{
		textWidth = ImGui::CalcTextSize("d").x;
		textHeight = ImGui::CalcTextSize("d").y;
	}
}