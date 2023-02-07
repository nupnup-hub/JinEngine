#include"JAppElapsedTime.h"
#include"../../JEditorAttribute.h"  
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../../../Application/JApplicationVariable.h"
#include"../../../../Utility/JCommonUtility.h"
#include"../../../../Core/Identity/JIdentifier.h"

namespace JinEngine
{
	namespace Editor
	{
		JAppElapsedTime::JAppElapsedTime(const std::string& name,
			std::unique_ptr<JEditorAttribute> attribute, 
			const J_EDITOR_PAGE_TYPE pageType,
			const J_EDITOR_WINDOW_FLAG windowFlag)
			:JEditorWindow(name, std::move(attribute), pageType, windowFlag)
		{ 
		}
		JAppElapsedTime::~JAppElapsedTime() {}
		J_EDITOR_WINDOW_TYPE JAppElapsedTime::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::DEBUG_APP_ELAPSED_TIME;
		}
		void JAppElapsedTime::UpdateWindow()
		{
			EnterWindow(ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
			UpdateDocking();
			if (IsActivated())
			{ 
				auto timeVec = JApplicationVariable::GetTime();
				for (const auto& data : timeVec)
					JImGuiImpl::Text("Func: " + std::get<0>(data) + " " + "Time: " + std::to_string(std::get<1>(data)) +"ms");
			}
			CloseWindow();
		}
	}
}