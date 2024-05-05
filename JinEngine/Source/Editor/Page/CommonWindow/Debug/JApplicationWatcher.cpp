#include"JApplicationWatcher.h"
#include"../../JEditorAttribute.h"  
#include"../../../Gui/JGui.h"
#include"../../../../Application/Project/JApplicationProject.h"
#include"../../../../Core/Utility/JCommonUtility.h"
#include"../../../../Core/Identity/JIdentifier.h"
#include"../../../../Core/Time/JGameTimer.h"
#include"../../../../Core/Memory/JMemoryCapture.h"
#include"../../../../Core/Unit/JByteUnit.h"
#include"../../../../Graphic/JGraphic.h"

namespace JinEngine
{
	namespace Editor
	{
		JApplicationWatcher::JApplicationWatcher(const std::string& name,
			std::unique_ptr<JEditorAttribute> attribute, 
			const J_EDITOR_PAGE_TYPE pageType,
			const J_EDITOR_WINDOW_FLAG windowFlag)
			:JEditorWindow(name, std::move(attribute), pageType, windowFlag)
		{ 
		}
		JApplicationWatcher::~JApplicationWatcher() {}
		J_EDITOR_WINDOW_TYPE JApplicationWatcher::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::APPLICATION_WATCHER;
		}
		void JApplicationWatcher::UpdateWindow()
		{
			EnterWindow(J_GUI_WINDOW_FLAG_NO_SCROLL_BAR);
			UpdateDocking();
			if (IsActivated())
			{ 
				UpdateMouseClick();
				DisplayTimeOnScreen();
				DisplayCpuDrawingTimeOnScreen();
				DisplayMemoryUsageOnScreen();
			}
			CloseWindow();
		}
		void JApplicationWatcher::DisplayTimeOnScreen()
		{
			JGui::BeginGroup();
			JGui::Text("Open date: " + JApplicationProject::GetOpenProjectInfo()->GetLastUpdateTime().ToString());
			JGui::Text("Fps: " + std::to_string(Core::JGameTimer::FramePerSecond()));
			JGui::EndGroup();
		}
		void JApplicationWatcher::DisplayCpuDrawingTimeOnScreen()
		{
			JGui::BeginGroup();
			static bool display = false;
			if (JGui::Button("Cpu time##JApplicationWatcher"))
				display = !display;
			if (display)
			{
				auto log = JGraphic::Instance().GetCpuDrawingLog();
				for (const auto& step : log)
				{
					JGui::Text(step.name + ": " + std::to_string(step.lastUpdatedMsTime) + ".ms");
					JGui::Separator();
				}
			}
			JGui::EndGroup();
		}
		void JApplicationWatcher::DisplayMemoryUsageOnScreen()
		{
			JGui::BeginGroup();
			auto mem = Core::JMemoryCapture::GetTotalMemory();
			auto proc = Core::JMemoryCapture::GetCurrentProcessMemory();
			JGui::Text("Memory: " + Core::JByteUnit::ByteToString(mem.totalPhys));
			JGui::Text("VirtualMemory: " + Core::JByteUnit::ByteToString(mem.totalVirtual));
			JGui::Text("ProcessUsage: " + Core::JByteUnit::ByteToString(proc.privateUsage));

			static bool display = false;
			if (JGui::Button("Detail##JApplicationWatcher"))
				display = !display;

			if (display)
			{
				JGui::Text("PeakWorkingSetSize: " + Core::JByteUnit::ByteToString(proc.peakWorkingSetSize));
				JGui::Text("WorkingSetSize: " + Core::JByteUnit::ByteToString(proc.workingSetSize));
				if (JGui::BeginListBox("Type##JApplicationWatcher"))
				{
					auto typeVec = _JReflectionInfo::Instance().GetDerivedTypeInfo(Core::JIdentifier::StaticTypeInfo());
					for (const auto& data : typeVec)
					{
						if(data->IsAbstractType())
							continue;

						auto allocInfo = data->GetAllocInfo();
						JGui::Text("Type: " + data->Name());
						JGui::Text("Reserve: " + Core::JByteUnit::ByteToString(allocInfo.totalReserveSize));
						JGui::Text("Committed: " + Core::JByteUnit::ByteToString(allocInfo.totalCommittedSize));
						JGui::Text("Count: " + std::to_string(allocInfo.useBlockCount));

						auto implAllocInfo = data->GetImplTypeInfo()->GetAllocInfo();
						JGui::Text("Impl Type: " + data->Name());
						JGui::Text("Impl Reserve: " + Core::JByteUnit::ByteToString(implAllocInfo.totalReserveSize));
						JGui::Text("Impl Committed: " + Core::JByteUnit::ByteToString(implAllocInfo.totalCommittedSize));
						JGui::Text("Impl Count: " + std::to_string(implAllocInfo.useBlockCount));
						JGui::NewLine();
					}
				}
			}
			JGui::EndGroup();
		}
	}
}