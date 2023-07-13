#include"JApplicationWatcher.h"
#include"../../JEditorAttribute.h"  
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../../../Application/JApplicationProject.h"
#include"../../../../Utility/JCommonUtility.h"
#include"../../../../Core/Identity/JIdentifier.h"
#include"../../../../Core/Time/JGameTimer.h"
#include"../../../../Core/Memory/JMemoryCapture.h"
#include"../../../../Core/Unit/JByteUnit.h"

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
			EnterWindow(ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
			UpdateDocking();
			if (IsActivated())
			{ 
				DisplayTimeOnScreen();
				DisplayMemoryUsageOnScreen();
			}
			CloseWindow();
		}
		void JApplicationWatcher::DisplayTimeOnScreen()
		{
			ImGui::BeginGroup();
			JImGuiImpl::Text("Open date: " + JApplicationProject::GetOpenProjectInfo()->GetLastUpdateTime().ToString());
			JImGuiImpl::Text("Fps: " + std::to_string(Core::JGameTimer::FramePerSecond()));
			ImGui::EndGroup();
		}
		void JApplicationWatcher::DisplayMemoryUsageOnScreen()
		{
			ImGui::BeginGroup();
			auto mem = Core::JMemoryCapture::GetTotalMemory();
			auto proc = Core::JMemoryCapture::GetCurrentProcessMemory();
			JImGuiImpl::Text("Memory: " + Core::JByteUnit::ByteToString(mem.totalPhys));
			JImGuiImpl::Text("VirtualMemory: " + Core::JByteUnit::ByteToString(mem.totalVirtual));
			JImGuiImpl::Text("ProcessUsage: " + Core::JByteUnit::ByteToString(proc.privateUsage));

			static bool displayDetail = false;
			if (JImGuiImpl::Button("Detail##JApplicationWatcher"))
				displayDetail = !displayDetail;

			if (displayDetail)
			{
				JImGuiImpl::Text("PeakWorkingSetSize: " + Core::JByteUnit::ByteToString(proc.peakWorkingSetSize));
				JImGuiImpl::Text("WorkingSetSize: " + Core::JByteUnit::ByteToString(proc.workingSetSize));
				if (JImGuiImpl::BeginListBox("Type##JApplicationWatcher"))
				{
					auto typeVec = _JReflectionInfo::Instance().GetDerivedTypeInfo(Core::JIdentifier::StaticTypeInfo());
					for (const auto& data : typeVec)
					{
						if(data->IsAbstractType())
							continue;

						auto allocInfo = data->GetAllocInfo();
						JImGuiImpl::Text("Type: " + data->Name());
						JImGuiImpl::Text("Reserve: " + Core::JByteUnit::ByteToString(allocInfo.totalReserveSize));
						JImGuiImpl::Text("Committed: " + Core::JByteUnit::ByteToString(allocInfo.totalCommittedSize));
						JImGuiImpl::Text("Count: " + std::to_string(allocInfo.useBlockCount));

						auto implAllocInfo = data->GetImplTypeInfo()->GetAllocInfo();
						JImGuiImpl::Text("Impl Type: " + data->Name());
						JImGuiImpl::Text("Impl Reserve: " + Core::JByteUnit::ByteToString(implAllocInfo.totalReserveSize));
						JImGuiImpl::Text("Impl Committed: " + Core::JByteUnit::ByteToString(implAllocInfo.totalCommittedSize));
						JImGuiImpl::Text("Impl Count: " + std::to_string(implAllocInfo.useBlockCount));
						ImGui::NewLine();
					}
				}
			}
			ImGui::EndGroup();
		}
	}
}