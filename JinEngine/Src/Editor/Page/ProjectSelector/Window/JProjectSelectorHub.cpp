#include"JProjectSelectorHub.h"
#include"../../JEditorAttribute.h"
#include"../../../Align/JEditorAlignCalculator.h"
#include"../../../Helpers/JEditorInputBuffHelper.h"
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../../../Application/JApplicationVariable.h" 
#include"../../../../Object/Resource/JResourceManager.h"
#include"../../../../Object/Resource/JResourceObjectFactory.h"
#include"../../../../Object/Resource/Texture/JTexture.h"
#include"../../../../Utility/JCommonUtility.h"
#include"../../../../Utility/JMacroUtility.h"
#include"../../../../Window/JWindows.h"

namespace JinEngine
{
	namespace Editor
	{
		namespace Constants
		{
			static constexpr ImGuiWindowFlags wndFlag = ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_NoDocking;

			static constexpr float paddingRate = 0.025f;

			//text and icon rate
			static constexpr float iconSizeRate = 0.75f;
			static constexpr float iconSpacingRate = 0.02f;
			static constexpr uint titleBarIconCount = 1;

			//pList
			static const JVector2<float> pListCanvasRate = JVector2<float>(0.7f, 0.8f);
			static constexpr float pListSpacingRate = 0.02f;
			static constexpr float pListPaddingRate = 0.04f;
			static constexpr float pListPerColumn = 4;
			static constexpr float pListPerRow = 3;
			static constexpr float pListImageRate = 0.7f;
			static constexpr float pListInnerPaddingRate = 0.025f;

			//pDetail
			static const JVector2<float> pDetailCanvaseRate = JVector2<float>(0.3f, 0.8f);
			static const JVector2<float> pDetailImageSizeRate = JVector2<float>(1.0f, 0.3f);
			static const JVector2<float> pDetailImagePaddingRate = JVector2<float>(0.2f, 0.075f);
			static const JVector2<float> pDetailTextPaddingRate = JVector2<float>(0.025f, 0.075f);

			//guide button
			//wnd : button
			static const float buttonWidthRate = 0.1f;
			//alphasize : buttoon
			static const float buttonHeightRate = 2.0f;
			static const float buttonSpacingRate = 0.0125f;

			static constexpr uint pListButtonCount = 2;
			static constexpr uint pDetailButtonCount = 1;

			//Texture
			static const float searchIconWidthRate = 0.025f;

			static JVector2<float> GetCanvasPos(const JVector2<float>& paddingRate, const JVector2<float>& offset = JVector2<float>(0, 0))
			{
				const JVector2<float> nowCursorPos = ImGui::GetCursorPos();
				const JVector2<float> wndSize = JImGuiImpl::GetGuiWidnowContentsSize();
				return wndSize * paddingRate + offset + nowCursorPos;
			}

			static JVector2<float> GetCanvasSize(const JVector2<float>& canvasRate)
			{
				const JVector2<float> wndSize = JImGuiImpl::GetGuiWidnowContentsSize();
				const JVector2<float> canvasPadding = wndSize * paddingRate;
				const JVector2<float> totalCanvasSize = wndSize - canvasPadding * 2;
				return totalCanvasSize * canvasRate;
			}
		}

		struct SelectorValues
		{
		private:
			std::unique_ptr<JEditorInputBuffHelper> nameHelper;
			std::unique_ptr<JEditorInputBuffHelper> pathHelper;
			int selectProjectIndex = -1;
			bool createProjectTrigger = false;
			bool loadProjectTrigger = false;
		public:
			static constexpr uint maxNameRange = 50;
			static constexpr uint maxPathRange = 260;
		public:
			SelectorValues()
			{
				nameHelper = std::make_unique<JEditorInputBuffHelper>(maxNameRange);
				pathHelper = std::make_unique<JEditorInputBuffHelper>(maxPathRange);
			}
		public:
			void ClearInputHelperBuff()noexcept
			{
				nameHelper->Clear();
				pathHelper->Clear();
			}
			void OffButtonTrigger()noexcept
			{
				createProjectTrigger = loadProjectTrigger = false;
			}
		public:
			bool* GetCreateProjectTriggerPtr()noexcept
			{
				return &createProjectTrigger;
			}
			bool* GetLoadProjectTriggerPtr()noexcept
			{
				return &loadProjectTrigger;
			}
		public:
			J_SIMPLE_GET_SET(int, selectProjectIndex, SelectedIndex);
			J_SIMPLE_GET_SET(bool, createProjectTrigger, CreateProjectTrigger);
			J_SIMPLE_GET_SET(bool, loadProjectTrigger, LoadProjectTrigger);
			J_SIMPLE_UNIQUE_P_GET(JEditorInputBuffHelper, nameHelper, NameHelper);
			J_SIMPLE_UNIQUE_P_GET(JEditorInputBuffHelper, pathHelper, PathHelper);
		};

		JProjectSelectorHub::JProjectSelectorHub(const std::string& name, std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE ownerPageType)
			:JEditorWindow(name, std::move(attribute), ownerPageType)
		{
			selectorValues = std::make_unique<SelectorValues>();
			JResourceUserInterface::AddEventListener(*JResourceManager::Instance().EvInterface(), GetGuid(), J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE);
			JApplicationProject::LoadProjectList();
		}
		JProjectSelectorHub::~JProjectSelectorHub()
		{
			JResourceUserInterface::RemoveListener(*JResourceManager::Instance().EvInterface(), GetGuid());
			JApplicationProject::StoreProjectList();
		}
		J_EDITOR_WINDOW_TYPE JProjectSelectorHub::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::PROJECT_SELECTOR_HUB;
		}
		void JProjectSelectorHub::UpdateWindow()
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowPos(viewport->WorkPos);

			int flag = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize;
			EnterWindow(flag);
			if (IsActivated())
			{
				//JImGuiImpl::AddImage(*backgroundTexture.Get(), viewport->WorkPos, viewport->WorkSize, false, IM_COL32(255, 255, 255, 50));
				JImGuiImpl::PushFont();
				TitleOnScreen();
				ProjectListOnScreen();
				ProjectDetailOnScreen();
				GuideButtonOnScreen();

				if (selectorValues->GetCreateProjectTrigger())
					CreateNewProjectOnScreen();
				if (selectorValues->GetLoadProjectTrigger())
					LoadProjectOnScreen();

				JImGuiImpl::PopFont();
			}
			CloseWindow();
		}
		void JProjectSelectorHub::TitleOnScreen()
		{
			ImGui::SetWindowFontScale(1);
			const JVector2<float> wndSize = JImGuiImpl::GetGuiWidnowContentsSize();
			const JVector2<float> padding = wndSize * Constants::paddingRate;
			ImGui::SetCursorPos(ImGui::GetCursorPos() + padding);
			JImGuiImpl::Text("JinEngine");

			using MenuIconPtr = void(*)(SelectorValues& value);
			using GetTexturePtr = JTexture * (*)();

			static std::string uniqueLabel[Constants::titleBarIconCount]
			{
				"##" + std::to_string(Core::MakeGuid()),
				//"##" + std::to_string(Core::MakeGuid()),
				//"##" + std::to_string(Core::MakeGuid())
			};

			GetTexturePtr getTexturePtr[Constants::titleBarIconCount]
			{
				[]() {return JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::OPTION_SETTING); },
				//[]() {return JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::OPTION_SETTING); },
				//[]() {return JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::OPTION_SETTING); }
			};
			MenuIconPtr iconPtr[Constants::titleBarIconCount]
			{
				[](SelectorValues& value) {/*¹Ì±¸Çö*/},
				//[](SelectorValues& value) {MessageBox(0, L"Click2", 0, 0); },
				//[](SelectorValues& value) {MessageBox(0, L"Click3", 0, 0); }
			};

			const JVector2<float> textSize = ImGui::CalcTextSize("JinEngine");
			const JVector2<float> iconSize = JVector2<float>(ImGui::GetCurrentContext()->FontSize * 0.75f, ImGui::GetCurrentContext()->FontSize * 0.75f);
			const JVector2<float> iconSpacing = wndSize * Constants::iconSpacingRate;

			float canvasWidth = (iconSize.x * Constants::titleBarIconCount) + (iconSpacing.x * (Constants::titleBarIconCount - 1));
			float canvasHeight = iconSize.y;
			JVector2<float>canvasSize = JVector2<float>(canvasWidth, canvasHeight);
			JVector2<float>canvasPos = JVector2<float>(wndSize.x - canvasSize.x - padding.x, ImGui::GetCursorPosY() - iconSize.y);
			JVector2<float> iconEnd(wndSize.x, ImGui::GetCursorPosY());
			JVector2<float> iconStart(0, 0);
			iconStart.x = iconEnd.x - (iconSize.x * Constants::titleBarIconCount) - (iconSpacing.x * (Constants::titleBarIconCount - 1));
			iconStart.y = iconEnd.y - iconSize.y;

			JEditorDynamicAlignCalculator<1> iconAlignCal;
			iconAlignCal.Update(canvasSize,
				iconSize,
				JVector2<float>(0, 0),
				iconSpacing,
				{ iconSize.y },
				{ JVector2<float>(0,0) },
				canvasPos);

			for (uint i = 0; i < Constants::titleBarIconCount; ++i)
			{
				iconAlignCal.SetNextContentsPosition();
				bool isOpen = true;
				if (JImGuiImpl::ImageSwitch(uniqueLabel[i],
					*(*getTexturePtr[i])(),
					isOpen,
					iconSize,
					JImGuiImpl::GetUColor(ImGuiCol_ChildBg),
					IM_COL32(0, 0, 0, 0)))
					(*iconPtr[i])(*selectorValues);
			}
			ImGui::SetWindowFontScale(1);
		}
		void JProjectSelectorHub::ProjectListOnScreen()
		{
			ImGui::SetWindowFontScale(0.45);
			ImGui::Separator();

			const JVector2<float> nowCursorPos = ImGui::GetCursorPos();
			const JVector2<float> wndSize = JImGuiImpl::GetGuiWidnowContentsSize();
			const JVector2<float> listPos = Constants::GetCanvasPos(Constants::paddingRate);
			const JVector2<float> listCanvasSize = Constants::GetCanvasSize(Constants::pListCanvasRate);

			const JVector2<float> contentsPadding = wndSize * Constants::pListPaddingRate;
			const JVector2<float> contentsSpacing = wndSize * Constants::pListSpacingRate;

			ImGui::SetCursorPos(listPos);
			JImGuiImpl::DrawRectFrame(ImGui::GetCursorScreenPos(), listCanvasSize, 5, IM_COL32(25, 25, 25, 255), true);
			JImGuiImpl::BeginChildWindow("##ProjectList", listCanvasSize);
			JImGuiImpl::DrawRectFilledMultiColor(ImGui::GetCursorScreenPos(), listCanvasSize, IM_COL32(85, 85, 85, 200), IM_COL32(25, 25, 25, 0), true);
			JEditorDynamicAlignCalculator<2> widgetAlignCal;
			widgetAlignCal.Update(listCanvasSize, contentsPadding, contentsSpacing,
				Constants::pListPerColumn,
				Constants::pListPerRow,
				{ Constants::pListImageRate, 1 - Constants::pListImageRate },
				{ JVector2<float>(0, 0), JVector2<float>(Constants::pListInnerPaddingRate, Constants::pListInnerPaddingRate) },
				JVector2<float>(0, 0));

			JEditorTextAlignCalculator txtAlignCal;
			const uint projectListCount = JApplicationProject::GetProjectInfoCount();
			for (uint i = 0; i < projectListCount; ++i)
			{
				JApplicationProject::JProjectInfo* info = JApplicationProject::GetProjectInfo(i);
				JVector2<float> nowSize = widgetAlignCal.GetNowContentsSize();

				widgetAlignCal.SetNextContentsPosition();
				if (JImGuiImpl::Button(JCUtil::WstrToU8Str(L"##" + info->GetName()).c_str(), nowSize))
					selectorValues->SetSelectedIndex(i);

				nowSize = widgetAlignCal.GetNowContentsSize();
				widgetAlignCal.SetNextContentsPosition();

				txtAlignCal.Update(JCUtil::WstrToU8Str(info->GetName()), nowSize, true);
				ImGui::Text(txtAlignCal.MiddleAligned().c_str());
			}
			JImGuiImpl::EndChildWindow();
			ImGui::SameLine();
			ImGui::SetCursorPosY(nowCursorPos.y);
			ImGui::SetWindowFontScale(1);
			//const JVector2<float>
		}
		void JProjectSelectorHub::ProjectDetailOnScreen()
		{
			ImGui::SetWindowFontScale(0.45);
			const JVector2<float> detailCanvasPos = Constants::GetCanvasPos(JVector2<float>(0, Constants::paddingRate));
			const JVector2<float> detailCanvasSize = Constants::GetCanvasSize(Constants::pDetailCanvaseRate);

			ImGui::SetCursorPos(detailCanvasPos);
			JImGuiImpl::DrawRectFrame(ImGui::GetCursorScreenPos(), detailCanvasSize, 5, IM_COL32(25, 25, 25, 255), true);
			JImGuiImpl::BeginChildWindow("##ProjectDetail", detailCanvasSize);
			JImGuiImpl::DrawRectFilledMultiColor(ImGui::GetCursorScreenPos(), detailCanvasSize, IM_COL32(85, 85, 85, 200), IM_COL32(25, 25, 25, 0), true);
			const JVector2<float> imageSize = (detailCanvasSize - detailCanvasSize * Constants::pDetailImagePaddingRate * 2) * Constants::pDetailImageSizeRate;
			const JVector2<float> imagePos = detailCanvasSize * Constants::pDetailImagePaddingRate;

			JApplicationProject::JProjectInfo* info = nullptr;
			ImGui::SetCursorPos(imagePos);
			if (selectorValues->GetSelectedIndex() != -1)
			{
				info = JApplicationProject::GetProjectInfo(selectorValues->GetSelectedIndex());
				JImGuiImpl::DrawRectFilledColor(ImGui::GetCursorScreenPos(), imageSize, JImGuiImpl::GetUColor(ImGuiCol_Button), false);
			}
			else
			{
				JImGuiImpl::DrawRectFilledColor(ImGui::GetCursorScreenPos(), imageSize, JImGuiImpl::GetUColor(ImGuiCol_Button), false);
				//Load last actImage
			}

			const JVector2<float> textPadding = detailCanvasSize * Constants::pDetailTextPaddingRate;
			const JVector2<float> textPos = textPadding;

			auto pInfoOnScreen = [](const JVector2<float> padding, const std::string& guide, const std::string& info)
			{
				ImGui::SetCursorPosX(padding.x);
				ImGui::SetCursorPosY(ImGui::GetCursorPos().y + padding.y);
				JImGuiImpl::Text(guide);
				ImGui::SameLine();
				if (!info.empty())
					JImGuiImpl::Text(info);
			};

			ImGui::SetCursorPosX(0);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + imageSize.y);
			const std::string path = info ? JCUtil::WstrToU8Str(info->GetPath()) : "";

			JEditorTextAlignCalculator txtAlignCal;
			txtAlignCal.Update(path, JVector2<float>(JImGuiImpl::GetGuiWidnowContentsSize().x - textPadding.x * 2, 0), false);

			pInfoOnScreen(textPadding, "Name: ", info ? JCUtil::WstrToU8Str(info->GetName()) : "");
			pInfoOnScreen(textPadding, "Path: ", info ? txtAlignCal.LeftAligned() : "");
			pInfoOnScreen(textPadding, "Version: ", info ? JCUtil::WstrToU8Str(info->GetVersion()) : "");
			pInfoOnScreen(textPadding, "Created date: ", info ? info->GetCreatedTime().ToString() : "");
			pInfoOnScreen(textPadding, "Last updated date: ", info ? info->GetLastUpdateTime().ToString() : "");

			JImGuiImpl::EndChildWindow();
			ImGui::SetWindowFontScale(1);
		}
		void JProjectSelectorHub::GuideButtonOnScreen()
		{
			ImGui::SetWindowFontScale(0.45);
			const JVector2<float> wndSize = JImGuiImpl::GetGuiWidnowContentsSize();
			const JVector2<float> alphabetSize = ImGui::GetCurrentContext()->FontSize;
			const JVector2<float> btnSize = JVector2<float>(wndSize.x * Constants::buttonWidthRate, alphabetSize.y * Constants::buttonHeightRate);
			const JVector2<float> btnSpacing = wndSize * Constants::buttonSpacingRate;

			const JVector2<float> posOffset = JVector2<float>(Constants::GetCanvasPos(Constants::paddingRate).x, Constants::GetCanvasPos(0).y);

			const float listCanvasEndPosX = posOffset.x + Constants::GetCanvasSize(Constants::pListCanvasRate).x;
			const float detailCanvasEndPosX = listCanvasEndPosX + Constants::GetCanvasSize(Constants::pDetailCanvaseRate).x;

			const float listBtnPosX = listCanvasEndPosX - Constants::pListButtonCount * (btnSize.x + btnSpacing.x) + btnSpacing.x;
			const JVector2<float> listBtnPos = JVector2<float>(listBtnPosX, posOffset.y);
			const JVector2<float> listBtnSize = JVector2<float>(listCanvasEndPosX - listBtnPos.x, btnSize.y);

			const float detailBtnPosX = detailCanvasEndPosX - Constants::pDetailButtonCount * (btnSize.x + btnSpacing.x) + btnSpacing.x;
			const JVector2<float> detailBtnPos = JVector2<float>(detailBtnPosX, posOffset.y);
			const JVector2<float> detailBtnSize = JVector2<float>(detailCanvasEndPosX - detailBtnPos.x, btnSize.y);

			JEditorDynamicAlignCalculator<1> btnAlignCal;
			btnAlignCal.Update(listBtnSize,
				btnSize,
				JVector2<float>(0, 0),
				btnSpacing,
				{ btnSize.y },
				{ JVector2<float>(0,0) },
				listBtnPos);

			using GuidButtonPtr = void(*)(SelectorValues& value);
			static std::string pListBtnName[Constants::pListButtonCount]
			{
				"Create##ProjectSelector",
				"Load##ProjectSelector"
			};
			static GuidButtonPtr pListPtr[Constants::pListButtonCount]
			{
				[](SelectorValues& value)
				{
					value.ClearInputHelperBuff();
					value.OffButtonTrigger();
					value.SetCreateProjectTrigger(true);
				},
				[](SelectorValues& value)
				{
					value.ClearInputHelperBuff();
					value.OffButtonTrigger();
					value.SetLoadProjectTrigger(true);
				},
			};

			for (uint i = 0; i < Constants::pListButtonCount; ++i)
			{
				btnAlignCal.SetNextContentsPosition();
				if (JImGuiImpl::Button(pListBtnName[i], btnSize))
					(*pListPtr[i])(*selectorValues);
			}

			btnAlignCal.Update(detailBtnSize,
				btnSize,
				JVector2<float>(0, 0),
				btnSpacing,
				{ btnSize.y },
				{ JVector2<float>(0,0) },
				detailBtnPos);

			static std::string pDetailBtnName[Constants::pDetailButtonCount]
			{
				"Start##ProjectSelector",
			};
			static GuidButtonPtr pDetailPtr[Constants::pDetailButtonCount]
			{
				[](SelectorValues& value)
				{
					value.ClearInputHelperBuff();
					value.OffButtonTrigger();
					JApplicationProject::SetNextProjectInfo(JApplicationProject::GetProjectInfo(value.GetSelectedIndex())->GetUnique());
					if (!JApplicationProject::SetStartNewProjectTrigger())
						MessageBox(0, L"SetStartNewProjectTrigger Fail", 0, 0);
				},
			};

			for (uint i = 0; i < Constants::pDetailButtonCount; ++i)
			{
				btnAlignCal.SetNextContentsPosition();
				if (JImGuiImpl::Button(pDetailBtnName[i], btnSize))
					(*pDetailPtr[i])(*selectorValues);
			}
			ImGui::SetWindowFontScale(1);
		}
		void JProjectSelectorHub::CreateNewProjectOnScreen()
		{
			ImGui::SetNextWindowFocus();
			ImGui::SetNextWindowSize(ImVec2(JWindow::Instance().GetClientWidth() * 0.7f, JWindow::Instance().GetClientHeight() * 0.6f));
			if (JImGuiImpl::BeginWindow("Create New Project##Create Project Window" + GetName(), selectorValues->GetCreateProjectTriggerPtr(), Constants::wndFlag))
			{
				ImGui::SetWindowFontScale(0.6f);
				//ImGui::SetCursorPosY(JWindow::Instance().GetClientHeight() * 0.4f);
				JImGuiImpl::Text("Project Name: ");
				JImGuiImpl::InputText("##NameInput_ProjectSelector", selectorValues->GetNameHelper()->buff, selectorValues->GetNameHelper()->result, "Name...");

				JImGuiImpl::Text("Folder: ");
				JImGuiImpl::InputText("##PathInput_ProjectSelector", selectorValues->GetPathHelper()->buff, selectorValues->GetPathHelper()->result, "Path...", ImGuiInputTextFlags_ReadOnly);

				const float iconSize = ImGui::GetWindowSize().x * Constants::searchIconWidthRate;
				if (JImGuiImpl::ImageButton("##SearchDir_ProjectSelector", *serachIconTexture.Get(), iconSize))
				{
					std::wstring dirPath;
					if (JWindow::Instance().SelectDirectory(dirPath, L"please, select project location") && JWindow::Instance().HasStorageSpace(dirPath, necessaryCapacityMB))
					{
						selectorValues->GetPathHelper()->result = JCUtil::WstrToU8Str(dirPath);
						selectorValues->GetPathHelper()->buff = selectorValues->GetPathHelper()->result;
						selectorValues->GetPathHelper()->buff.resize(selectorValues->maxPathRange);
					}
				}
				 
				JImGuiImpl::Text("Version: ");
				JImGuiImpl::ComboSet("##Create Project Window Version Combo", versionIndex, JApplicationVariable::GetAppVersion());
				if (JImGuiImpl::Button("Create Proejct##Create Project Window"))
				{
					if (!selectorValues->GetNameHelper()->result.empty() && !selectorValues->GetPathHelper()->result.empty())
					{
						if (JCUtil::IsOverlappedDirectoryPath(JCUtil::U8StrToWstr(selectorValues->GetNameHelper()->result),
							JCUtil::U8StrToWstr(selectorValues->GetPathHelper()->result)))
							MessageBox(0, L"Overlapped Project Name", 0, 0);
						else
						{
							SetStartProjectProccess();
							selectorValues->ClearInputHelperBuff();
							selectorValues->SetCreateProjectTrigger(false); 
						}
					}
				}
				JImGuiImpl::EndWindow();
			}
		}
		void JProjectSelectorHub::LoadProjectOnScreen()
		{
			std::wstring dirPath;
			if (JWindow::Instance().SelectDirectory(dirPath, L"please, select project directory") && JWindow::Instance().HasStorageSpace(dirPath, necessaryCapacityMB))
			{
				if (JApplicationProject::IsValidPath(dirPath))
				{
					std::unique_ptr<JApplicationProject::JProjectInfo> pInfo;
					JApplicationProject::JProjectInfo* existingInfo = JApplicationProject::GetProjectInfo(dirPath);
					if (existingInfo != nullptr)
						pInfo = existingInfo->GetUnique();
					else
						pInfo = JApplicationProject::MakeProjectInfo(dirPath);

					JApplicationProject::SetNextProjectInfo(std::move(pInfo));
					if (!JApplicationProject::SetStartNewProjectTrigger())
						MessageBox(0, L"Load Project Fail", 0, 0);
					selectorValues->ClearInputHelperBuff();
				}
				selectorValues->SetLoadProjectTrigger(false);
			}
		}
		void JProjectSelectorHub::SetStartProjectProccess()
		{
			const std::wstring newProejctName = JCUtil::U8StrToWstr(JCUtil::EraseSideChar(selectorValues->GetNameHelper()->result, ' '));
			const std::wstring newProejctPath = JCUtil::U8StrToWstr(JCUtil::EraseSideChar(selectorValues->GetPathHelper()->result, ' ')) + newProejctName;
			std::vector<std::string> version = JApplicationVariable::GetAppVersion();

			JApplicationProject::SetNextProjectInfo(JApplicationProject::MakeProjectInfo(newProejctPath, version[versionIndex]));
			if (!JApplicationProject::SetStartNewProjectTrigger())
				MessageBox(0, L"Start Project Fail", 0, 0);
		}
		void JProjectSelectorHub::DoActivate()noexcept
		{
			JEditor::DoActivate();
			serachIconTexture = Core::GetUserPtr(JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::SEARCH_FOLDER_ICON));
			CallOnResourceReference(serachIconTexture.Get());

			backgroundTexture = Core::GetUserPtr(JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::PROJECT_SELECTOR_BACKGROUND));
			CallOnResourceReference(backgroundTexture.Get());

			JDirectory* engineProjectRsFolder = JResourceManager::Instance().GetDirectory(JApplicationVariable::GetEngineProjectLastRsPath());
			const uint projectListCount = JApplicationProject::GetProjectInfoCount();
			lastRSVec.resize(projectListCount);
			for (uint i = 0; i < projectListCount; ++i)
			{
				JApplicationProject::JProjectInfo* info = JApplicationProject::GetProjectInfo(i);
				const std::wstring path = info->lastRsPath();
				std::wstring folderPath;
				std::wstring name;
				std::wstring format;
				JCUtil::DecomposeFilePath(path, folderPath, name, format);
				/*
						JTextureInitData(const std::wstring& name,
				const size_t guid,
				const J_OBJECT_FLAG flag,
				JDirectory* directory,
				const std::wstring oridataPath);
	 
			JTextureInitData(const std::wstring& name,
				JDirectory* directory,
				const std::wstring oridataPath);
				*/
				JTexture* newTexture = JRFI<JTexture>::Create(Core::JPtrUtil::MakeOwnerPtr<JTexture::InitData>(name,
					Core::MakeGuid(), 
					OBJECT_FLAG_EDITOR_OBJECT,
					engineProjectRsFolder, 
					path));

				if (newTexture == nullptr)
					MessageBox(0, L"Load last project rs error", info->GetPath().c_str(), 0);

				CallOnResourceReference(newTexture);
				lastRSVec[i] = Core::GetUserPtr(newTexture);		 
			}			
		}
		void JProjectSelectorHub::DoDeActivate()noexcept
		{
			JEditor::DoDeActivate();
			CallOffResourceReference(serachIconTexture.Release());
			CallOffResourceReference(backgroundTexture.Release());
 
			const uint lastRsVecCout = (uint)lastRSVec.size();
			for (uint i = 0; i < lastRsVecCout; ++i)
				CallOffResourceReference(lastRSVec[i].Release());
			lastRSVec.clear();
		}
		void JProjectSelectorHub::OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)
		{
			if (iden == GetGuid())
				return;

			if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
			{
				if (serachIconTexture.IsValid() && jRobj->GetGuid() == serachIconTexture->GetGuid())
				{
					CallOffResourceReference(serachIconTexture.Get());
					serachIconTexture.Clear();
				}
				if (backgroundTexture.IsValid() && jRobj->GetGuid() == backgroundTexture->GetGuid())
				{
					CallOffResourceReference(backgroundTexture.Get());
					backgroundTexture.Clear();
				}
			}
		}
	}
}