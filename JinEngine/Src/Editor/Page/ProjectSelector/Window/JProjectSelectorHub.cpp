#include"JProjectSelectorHub.h"
#include"../../JEditorAttribute.h"
#include"../../../Align/JEditorAlignCalculator.h"
#include"../../../Helpers/JEditorSearchBarHelper.h"
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

			static const std::string title = "JinEngine";
			static const JVector2<float> canvasSpacing = JVector2<float>(0.0125f, 0.025f);
			static constexpr float paddingRate = 0.025f;
			static constexpr float frameThickness = 4;

			//text and icon rate
			static constexpr float iconSizeRate = 0.75f;
			static constexpr float iconSpacingRate = 0.02f; 

			//pList
			static const JVector2<float> pListCanvasRate = JVector2<float>(0.7f, 0.8f);
			static constexpr float pListSpacingRate = 0.02f;
			static constexpr float pListPaddingRate = 0.04f;
			static constexpr float pListPerColumn = 4;
			static constexpr float pListPerRow = 3;
			static constexpr float pListImageRate = 0.7f;
			static constexpr float pListInnerPaddingRate = 0.025f;

			//pDetail
			static const JVector2<float> pDetailCanvasRate = JVector2<float>(0.3f, 0.8f);
			static const JVector2<float> pDetailImageSizeRate = JVector2<float>(1.0f, 0.3f);
			static const JVector2<float> pDetailImagePaddingRate = JVector2<float>(0.2f, 0.075f);
			static const JVector2<float> pDetailTextPaddingRate = JVector2<float>(0.025f, 0.075f);

			//guide button
			//wnd : button
			static constexpr float buttonWidthRate = 0.1f;
			//alphasize : buttoon
			static constexpr float buttonHeightRate = 2.0f;
			static constexpr float buttonSpacingRate = 0.0125f;

			static constexpr uint pListButtonCount = 2;
			static constexpr uint pDetailButtonCount = 1;

			//Texture
			static const float searchIconWidthRate = 0.025f;
		}

		enum class J_PS_CANVAS
		{
			TITLE = 0,
			PROJECT_LIST,
			PROJECT_DETAIL,
			GUIDE_BUTTON,
			COUNT
		};
		enum class J_TITLE_ICON
		{ 
			OPTION_SETTING,
			COUNT
		};

		struct SelectorValues
		{
		private:
			std::unique_ptr<JEditorInputBuffHelper> nameHelper;
			std::unique_ptr<JEditorInputBuffHelper> pathHelper;
			int selectProjectIndex = -1;
			bool createProjectTrigger = false;
			bool loadProjectTrigger = false;
			bool optionTrigger = false;
		private:
			JVector2<float> canvasPos[(int)J_PS_CANVAS::COUNT];
			JVector2<float> canvasSize[(int)J_PS_CANVAS::COUNT];
			JVector2<float> totalCanvasSize;
			JVector2<float> canvasPadding;
			JVector2<float> canvasSpacing;
		private:
			JVector2<float> titleIconPos[(int)J_TITLE_ICON::COUNT];
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
			JVector2<float> GetCanvasPos(const J_PS_CANVAS type)const noexcept
			{
				return canvasPos[(int)type];
			}
			JVector2<float> GetCanvasSize(const J_PS_CANVAS type)const noexcept
			{
				return canvasSize[(int)type];
			}
			JVector2<float> GetTitleIconPos(const J_TITLE_ICON type)const noexcept
			{
				return titleIconPos[(int)type];
			}
			void SetCanvasPos(const J_PS_CANVAS type, const JVector2<float>& value)noexcept
			{
				canvasPos[(int)type] = value;
			}
			void SetCanvasSize(const J_PS_CANVAS type, const JVector2<float>& value)noexcept
			{
				canvasSize[(int)type] = value;
			}
			void SetTitleIconPos(const J_TITLE_ICON type, const JVector2<float>& value)noexcept
			{
				titleIconPos[(int)type] = value;
			}
		public:
			ImU32 GetCanvasColor()const noexcept
			{
				return IM_COL32(85, 85, 85, 200);
			}
			ImU32 GetCanvasDeltaColor()const noexcept
			{
				return IM_COL32(25, 25, 25, 0);
			}
			ImU32 GetFrameColor()const noexcept
			{
				return JImGuiImpl::GetUColor(ImGuiCol_FrameBg);
			}
		public:
			J_SIMPLE_GET_SET(JVector2<float>, totalCanvasSize, TotalCanvasSize);
			J_SIMPLE_GET_SET(JVector2<float>, canvasPadding, CanvasPadding);
			J_SIMPLE_GET_SET(JVector2<float>, canvasSpacing, CanvasSpacing);
			J_SIMPLE_GET_SET(int, selectProjectIndex, SelectedIndex);
			J_SIMPLE_GET_SET(bool, createProjectTrigger, CreateProjectTrigger);
			J_SIMPLE_GET_SET(bool, loadProjectTrigger, LoadProjectTrigger);
			J_SIMPLE_GET_SET(bool, optionTrigger, OptionTrigger);
			J_SIMPLE_UNIQUE_P_GET(JEditorInputBuffHelper, nameHelper, NameHelper);
			J_SIMPLE_UNIQUE_P_GET(JEditorInputBuffHelper, pathHelper, PathHelper);
		};

		JProjectSelectorHub::JProjectSelectorHub(const std::string& name, std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE ownerPageType)
			:JEditorWindow(name, std::move(attribute), ownerPageType)
		{
			searchHelper = std::make_unique<JEditorSearchBarHelper>(false);
			values = std::make_unique<SelectorValues>();
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
				UpdateCanvasSize();
				TitleOnScreen();
				ProjectListOnScreen();
				ProjectDetailOnScreen();
				GuideButtonOnScreen();

				if (values->GetCreateProjectTrigger())
					CreateNewProjectOnScreen();
				if (values->GetLoadProjectTrigger())
					LoadProjectOnScreen();
				if (values->GetOptionTrigger())
					OptionOnScreen();

				JImGuiImpl::PopFont();
			}
			CloseWindow();
		}
		void JProjectSelectorHub::UpdateCanvasSize()
		{
			ImGui::SetWindowFontScale(1);
			const JVector2<float> titleTextSize = ImGui::CalcTextSize(Constants::title.c_str());

			ImGui::SetWindowFontScale(0.45f);
			const JVector2<float> otherTextSize = ImGui::CalcTextSize(Constants::title.c_str());

			JVector2<float> canvasPos[(int)J_PS_CANVAS::COUNT];
			JVector2<float> canvasSize[(int)J_PS_CANVAS::COUNT];

			const JVector2<float> wndSize = JImGuiImpl::GetGuiWidnowContentsSize();
			const JVector2<float> padding = wndSize * Constants::paddingRate;
			const JVector2<float> canvasSpacing = wndSize * Constants::canvasSpacing;

			canvasPos[(int)J_PS_CANVAS::TITLE] = ImGui::GetCursorPos() + padding;
			canvasSize[(int)J_PS_CANVAS::TITLE] = JVector2<float>(wndSize.x - padding.x * 2, titleTextSize.y * 1.25f);

			canvasPos[(int)J_PS_CANVAS::PROJECT_LIST].x = canvasPos[(int)J_PS_CANVAS::TITLE].x;
			canvasPos[(int)J_PS_CANVAS::PROJECT_LIST].y = canvasPos[(int)J_PS_CANVAS::TITLE].y + canvasSize[(int)J_PS_CANVAS::TITLE].y + canvasSpacing.y;
			canvasSize[(int)J_PS_CANVAS::PROJECT_LIST] = (wndSize - padding * 2) * Constants::pListCanvasRate;
			canvasSize[(int)J_PS_CANVAS::PROJECT_LIST].x -= canvasSpacing.x / 2;

			canvasPos[(int)J_PS_CANVAS::PROJECT_DETAIL].x = canvasPos[(int)J_PS_CANVAS::PROJECT_LIST].x + canvasSize[(int)J_PS_CANVAS::PROJECT_LIST].x + canvasSpacing.x;
			canvasPos[(int)J_PS_CANVAS::PROJECT_DETAIL].y = canvasPos[(int)J_PS_CANVAS::PROJECT_LIST].y;
			canvasSize[(int)J_PS_CANVAS::PROJECT_DETAIL] = (wndSize - padding * 2) * Constants::pDetailCanvasRate;
			canvasSize[(int)J_PS_CANVAS::PROJECT_DETAIL].x -= canvasSpacing.x / 2;

			canvasPos[(int)J_PS_CANVAS::GUIDE_BUTTON].x = canvasPos[(int)J_PS_CANVAS::TITLE].x;
			canvasPos[(int)J_PS_CANVAS::GUIDE_BUTTON].y = canvasPos[(int)J_PS_CANVAS::PROJECT_DETAIL].y +
				canvasSize[(int)J_PS_CANVAS::PROJECT_DETAIL].y +
				canvasSpacing.y * 0.75f;

			canvasSize[(int)J_PS_CANVAS::GUIDE_BUTTON] = (wndSize - padding * 2);
			canvasSize[(int)J_PS_CANVAS::GUIDE_BUTTON].y -= ((canvasSpacing.y * 2) + canvasSize[(int)J_PS_CANVAS::TITLE].y + canvasSize[(int)J_PS_CANVAS::PROJECT_DETAIL].y);

			for (uint i = 0; i < (int)J_PS_CANVAS::COUNT; ++i)
			{
				values->SetCanvasPos((J_PS_CANVAS)i, canvasPos[i]);
				values->SetCanvasSize((J_PS_CANVAS)i, canvasSize[i]);
			}
			values->SetTotalCanvasSize(wndSize);
			values->SetCanvasPadding(padding);
			values->SetCanvasSpacing(canvasSpacing);
			ImGui::SetWindowFontScale(1);
		}
		void JProjectSelectorHub::TitleOnScreen()
		{
			ImGui::SetWindowFontScale(1);
			const JVector2<float> textSize = ImGui::CalcTextSize("JinEngine");
			const JVector2<float> iconSize = JVector2<float>(ImGui::GetCurrentContext()->FontSize * 0.75f, ImGui::GetCurrentContext()->FontSize * 0.75f);
			const JVector2<float> iconSpacing = values->GetTotalCanvasSize() * Constants::iconSpacingRate;

			ImGui::SetCursorPos(values->GetCanvasPos(J_PS_CANVAS::TITLE));
			JImGuiImpl::DrawRectFilledColor(ImGui::GetCursorScreenPos(),
				values->GetCanvasSize(J_PS_CANVAS::TITLE),
				values->GetCanvasColor(),
				true);
			JImGuiImpl::DrawRectFrame(ImGui::GetCursorScreenPos(),
				values->GetCanvasSize(J_PS_CANVAS::TITLE),
				Constants::frameThickness,
				values->GetFrameColor(),
				true);

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::CalcTextSize("J").x * 0.5f));
			JEditorTextAlignCalculator textAlign;
			textAlign.Update("JinEngine", values->GetCanvasSize(J_PS_CANVAS::TITLE), false);
			JImGuiImpl::Text(textAlign.LeftAligned());

			float iconCanvasWidth = (iconSize.x * (int)J_TITLE_ICON::COUNT) + (iconSpacing.x * ((int)J_TITLE_ICON::COUNT - 1));
			float iconCanvasHeight = iconSize.y;

			const JVector2<float>canvasSize = JVector2<float>(iconCanvasWidth, iconCanvasHeight);
			JVector2<float>canvasPos;
			canvasPos.x = values->GetTotalCanvasSize().x - canvasSize.x - values->GetCanvasPadding().x;
			canvasPos.y = ImGui::GetCursorPosY() - iconSize.y;

			JEditorDynamicAlignCalculator<1> iconAlignCal;
			iconAlignCal.Update(canvasSize,
				iconSize,
				JVector2<float>(0, 0),
				iconSpacing,
				{ iconSize.y },
				{ JVector2<float>(0,0) },
				canvasPos);

			using MenuIconPtr = void(*)(SelectorValues& value);
			using GetTexturePtr = JTexture * (*)();

			static std::string uniqueLabel[(int)J_TITLE_ICON::COUNT]
			{
				"##" + std::to_string(Core::MakeGuid()),
				//"##" + std::to_string(Core::MakeGuid()),
				//"##" + std::to_string(Core::MakeGuid())
			};

			GetTexturePtr getTexturePtr[(int)J_TITLE_ICON::COUNT]
			{
				[]() {return JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::OPTION_SETTING); },
				//[]() {return JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::OPTION_SETTING); },
				//[]() {return JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::OPTION_SETTING); }
			};
			MenuIconPtr iconPtr[(int)J_TITLE_ICON::COUNT]
			{
				[](SelectorValues& value) {value.SetOptionTrigger(!value.GetOptionTrigger()); },
				//[](SelectorValues& value) {MessageBox(0, L"Click2", 0, 0); },
				//[](SelectorValues& value) {MessageBox(0, L"Click3", 0, 0); }
			};

			for (uint i = 0; i < (int)J_TITLE_ICON::COUNT; ++i)
			{
				iconAlignCal.SetNextContentsPosition();
				values->SetTitleIconPos((J_TITLE_ICON)i, ImGui::GetCursorScreenPos());
				bool isOpen = true;
				if (JImGuiImpl::ImageSwitch(uniqueLabel[i],
					*(*getTexturePtr[i])(),
					isOpen,
					false,
					iconSize,
					JImGuiImpl::GetUColor(ImGuiCol_ChildBg),
					IM_COL32(0, 0, 0, 0)))
					(*iconPtr[i])(*values);
			}
			
			ImGui::SetWindowFontScale(1);
		}
		void JProjectSelectorHub::ProjectListOnScreen()
		{
			ImGui::SetWindowFontScale(0.45);
			ImGui::SetCursorPos(values->GetCanvasPos(J_PS_CANVAS::PROJECT_LIST));
			JImGuiImpl::DrawRectFilledMultiColor(ImGui::GetCursorScreenPos(),
				values->GetCanvasSize(J_PS_CANVAS::PROJECT_LIST),
				values->GetCanvasColor(),
				values->GetCanvasDeltaColor(),
				true);
			//has order dependency
			//child window내부에서는 window pos + size를 벗어날수없으므로
			//frame 외부가 잘린다
			JImGuiImpl::DrawRectFrame(ImGui::GetCursorScreenPos(),
				values->GetCanvasSize(J_PS_CANVAS::PROJECT_LIST),
				Constants::frameThickness,
				values->GetFrameColor(),
				true);

			JImGuiImpl::BeginChildWindow("##ProjectList", values->GetCanvasSize(J_PS_CANVAS::PROJECT_LIST));
			searchHelper->UpdateSearchBar();
			
			const JVector2<float> contentsPadding = values->GetTotalCanvasSize() * Constants::pListPaddingRate;
			const JVector2<float> contentsSpacing = values->GetTotalCanvasSize() * Constants::pListSpacingRate;

			JEditorDynamicAlignCalculator<2> widgetAlignCal;
			widgetAlignCal.Update(values->GetCanvasSize(J_PS_CANVAS::PROJECT_LIST),
				contentsPadding,
				contentsSpacing,
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
				if(!searchHelper->CanSrcNameOnScreen(info->GetName()))
					continue;

				JVector2<float> nowSize = widgetAlignCal.GetNowContentsSize();
				widgetAlignCal.SetNextContentsPosition();
				bool isSelected = i == values->GetSelectedIndex();
				if (JImGuiImpl::ImageSelectable(JCUtil::WstrToU8Str(info->GetName()), *lastRSVec[i],
					isSelected, false,
					nowSize))
					values->SetSelectedIndex(i);

				nowSize = widgetAlignCal.GetNowContentsSize();
				widgetAlignCal.SetNextContentsPosition();

				txtAlignCal.Update(JCUtil::WstrToU8Str(info->GetName()), nowSize, true);
				ImGui::Text(txtAlignCal.MiddleAligned().c_str());
			}
			JImGuiImpl::EndChildWindow();
			ImGui::SetWindowFontScale(1);
			//const JVector2<float>
		}
		void JProjectSelectorHub::ProjectDetailOnScreen()
		{
			ImGui::SetWindowFontScale(0.45);
			ImGui::SetCursorPos(values->GetCanvasPos(J_PS_CANVAS::PROJECT_DETAIL));

			JImGuiImpl::DrawRectFilledMultiColor(ImGui::GetCursorScreenPos(),
				values->GetCanvasSize(J_PS_CANVAS::PROJECT_DETAIL),
				values->GetCanvasColor(),
				values->GetCanvasDeltaColor(),
				true);
			//has order dependency
			//child window내부에서는 window pos + size를 벗어날수없으므로
			//frame 외부가 잘린다
			JImGuiImpl::DrawRectFrame(ImGui::GetCursorScreenPos(),
				values->GetCanvasSize(J_PS_CANVAS::PROJECT_DETAIL),
				Constants::frameThickness,
				values->GetFrameColor(),
				true);
			JImGuiImpl::BeginChildWindow("##ProjectDetail", values->GetCanvasSize(J_PS_CANVAS::PROJECT_DETAIL));

			const JVector2<float> canvasSize = values->GetCanvasSize(J_PS_CANVAS::PROJECT_DETAIL);
			const JVector2<float> imageSize = (canvasSize - canvasSize * Constants::pDetailImagePaddingRate * 2) * Constants::pDetailImageSizeRate;
			const JVector2<float> imagePos = canvasSize * Constants::pDetailImagePaddingRate;

			const JVector2<float> textPadding = canvasSize * Constants::pDetailTextPaddingRate;
			const JVector2<float> textPos = textPadding;

			JApplicationProject::JProjectInfo* info = nullptr;
			ImGui::SetCursorPos(imagePos);
			if (values->GetSelectedIndex() != -1)
			{
				info = JApplicationProject::GetProjectInfo(values->GetSelectedIndex());
				JImGuiImpl::Image(*lastRSVec[values->GetSelectedIndex()], imageSize);
				ImGui::SetCursorPos(imagePos);
				JImGuiImpl::DrawRectFrame(ImGui::GetCursorScreenPos(), imageSize, Constants::frameThickness, values->GetFrameColor(), true);
				ImGui::SetCursorPos(imagePos + JVector2<float>(0, imageSize.y + textPadding.y));
			}
			else
			{
				JImGuiImpl::DrawRectFilledColor(ImGui::GetCursorScreenPos(), imageSize, JImGuiImpl::GetUColor(ImGuiCol_Button), true);
				JImGuiImpl::DrawRectFrame(ImGui::GetCursorScreenPos(), imageSize, Constants::frameThickness, values->GetFrameColor(), true);
				//Load last actImage
			}

			auto pInfoOnScreen = [](const JVector2<float> padding, const std::string& guide, const std::string& info)
			{
				ImGui::SetCursorPosX(padding.x);
				//ImGui::SetCursorPosY(ImGui::GetCursorPos().y + padding.y);
				JImGuiImpl::Text(guide);
				ImGui::SameLine();
				JImGuiImpl::Text(info);
			};

			JEditorTextAlignCalculator txtAlignCal;
			if (info != nullptr)
			{
				const std::string path = JCUtil::WstrToU8Str(info->GetPath());
				txtAlignCal.Update(path, JVector2<float>(JImGuiImpl::GetGuiWidnowContentsSize().x - textPadding.x * 2, 0), false);

				pInfoOnScreen(textPadding, "Name: ", JCUtil::WstrToU8Str(info->GetName()));
				pInfoOnScreen(textPadding, "Path: ", txtAlignCal.LeftAligned());
				pInfoOnScreen(textPadding, "Version: ", JCUtil::WstrToU8Str(info->GetVersion()));
				pInfoOnScreen(textPadding, "Created date: ", info->GetCreatedTime().ToString());
				pInfoOnScreen(textPadding, "Last updated date: ", info->GetLastUpdateTime().ToString());
			}
			else
			{
				txtAlignCal.Update("Please, select project", JImGuiImpl::GetGuiWidnowContentsSize(), false);
				const float cursorPosY = (canvasSize.y - ImGui::GetCursorPosY()) / 2 + ImGui::GetCurrentContext()->FontSize / 2;
				ImGui::SetCursorPosX(textPadding.x);
				ImGui::SetCursorPosY(cursorPosY);
				JImGuiImpl::Text(txtAlignCal.MiddleAligned());
			}
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

			const float listCanvasEndPosX = values->GetCanvasPos(J_PS_CANVAS::PROJECT_LIST).x + values->GetCanvasSize(J_PS_CANVAS::PROJECT_LIST).x;
			const float detailCanvasEndPosX = values->GetCanvasPos(J_PS_CANVAS::PROJECT_DETAIL).x + values->GetCanvasSize(J_PS_CANVAS::PROJECT_DETAIL).x;

			const float listBtnPosX = listCanvasEndPosX - Constants::pListButtonCount * (btnSize.x + btnSpacing.x) + btnSpacing.x;
			const JVector2<float> listBtnPos = JVector2<float>(listBtnPosX, values->GetCanvasPos(J_PS_CANVAS::GUIDE_BUTTON).y);
			const JVector2<float> listBtnSize = JVector2<float>(listCanvasEndPosX - listBtnPos.x, btnSize.y);

			const float detailBtnPosX = detailCanvasEndPosX - Constants::pDetailButtonCount * (btnSize.x + btnSpacing.x) + btnSpacing.x;
			const JVector2<float> detailBtnPos = JVector2<float>(detailBtnPosX, values->GetCanvasPos(J_PS_CANVAS::GUIDE_BUTTON).y);
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
			std::string pListBtnName[Constants::pListButtonCount]
			{
				"Create##ProjectSelector",
				"Load##ProjectSelector"
			};
			GuidButtonPtr pListPtr[Constants::pListButtonCount]
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

			JImGuiImpl::SetColor(JVector4<float>(0, 0, 0, 0), ImGuiCol_Button);
			for (uint i = 0; i < Constants::pListButtonCount; ++i)
			{
				btnAlignCal.SetNextContentsPosition();
				JImGuiImpl::DrawRectFilledMultiColor(ImGui::GetCursorScreenPos(),
					btnSize,
					values->GetCanvasColor(),
					values->GetCanvasDeltaColor(),
					true);
				JImGuiImpl::DrawRectFrame(ImGui::GetCursorScreenPos(),
					btnSize,
					Constants::frameThickness,
					values->GetFrameColor(),
					true);
				if (JImGuiImpl::Button(pListBtnName[i], btnSize))
					(*pListPtr[i])(*values);
			}
			JImGuiImpl::SetColorToDefault(ImGuiCol_Button);

			btnAlignCal.Update(detailBtnSize,
				btnSize,
				JVector2<float>(0, 0),
				btnSpacing,
				{ btnSize.y },
				{ JVector2<float>(0,0) },
				detailBtnPos);

			std::string startBtnName = "Start##ProjectSelector";
			GuidButtonPtr startPtr = [](SelectorValues& value)
			{
				value.ClearInputHelperBuff();
				value.OffButtonTrigger();
				JApplicationProject::SetNextProjectInfo(JApplicationProject::GetProjectInfo(value.GetSelectedIndex())->GetUnique());
				if (!JApplicationProject::SetStartNewProjectTrigger())
					MessageBox(0, L"SetStartNewProjectTrigger Fail", 0, 0);
			};
			 
			if (values->GetSelectedIndex() == -1)
				JImGuiImpl::DeActivateButtonColor();

			btnAlignCal.SetNextContentsPosition();
			if (values->GetSelectedIndex() == -1)
			{
				JImGuiImpl::DrawRectFilledColor(ImGui::GetCursorScreenPos(),
					btnSize,
					IM_COL32(42.5f, 42.5f, 42.5f, 200),
					true);
			}
			else
			{
				JImGuiImpl::DrawRectFilledMultiColor(ImGui::GetCursorScreenPos(),
					btnSize,
					values->GetCanvasColor(),
					values->GetCanvasDeltaColor(),
					true);
			}
			JImGuiImpl::DrawRectFrame(ImGui::GetCursorScreenPos(),
				btnSize,
				Constants::frameThickness,
				values->GetFrameColor(),
				true);

			JImGuiImpl::SetColor(JVector4<float>(0, 0, 0, 0), ImGuiCol_Button);
			if (JImGuiImpl::Button(startBtnName, btnSize))
				(*startPtr)(*values);
			JImGuiImpl::SetColorToDefault(ImGuiCol_Button);

			if (values->GetSelectedIndex() == -1)
				JImGuiImpl::ActivateButtonColor();

			ImGui::SetWindowFontScale(1);
		}
		void JProjectSelectorHub::CreateNewProjectOnScreen()
		{
			ImGui::SetNextWindowFocus();
			ImGui::SetNextWindowSize(ImVec2(JWindow::Instance().GetClientWidth() * 0.7f, JWindow::Instance().GetClientHeight() * 0.6f));
			if (JImGuiImpl::BeginWindow("Create New Project##Create Project Window" + GetName(), values->GetCreateProjectTriggerPtr(), Constants::wndFlag))
			{
				ImGui::SetWindowFontScale(0.6f);
				//ImGui::SetCursorPosY(JWindow::Instance().GetClientHeight() * 0.4f);
				JImGuiImpl::Text("Project Name: ");
				JImGuiImpl::InputText("##NameInput_ProjectSelector", values->GetNameHelper()->buff, values->GetNameHelper()->result, "Name...");

				JImGuiImpl::Text("Folder: ");
				JImGuiImpl::InputText("##PathInput_ProjectSelector", values->GetPathHelper()->buff, values->GetPathHelper()->result, "Path...", ImGuiInputTextFlags_ReadOnly);

				const float iconSize = ImGui::GetWindowSize().x * Constants::searchIconWidthRate;
				if (JImGuiImpl::ImageButton("##SearchDir_ProjectSelector", *serachIconTexture.Get(), iconSize))
				{
					std::wstring dirPath;
					if (JWindow::Instance().SelectDirectory(dirPath, L"please, select project location") && JWindow::Instance().HasStorageSpace(dirPath, necessaryCapacityMB))
					{
						values->GetPathHelper()->result = JCUtil::WstrToU8Str(dirPath);
						values->GetPathHelper()->buff = values->GetPathHelper()->result;
						values->GetPathHelper()->buff.resize(values->maxPathRange);
					}
				}

				JImGuiImpl::Text("Version: ");
				JImGuiImpl::ComboSet("##Create Project Window Version Combo", versionIndex, JApplicationVariable::GetAppVersion());
				if (JImGuiImpl::Button("Create Proejct##Create Project Window"))
				{
					if (!values->GetNameHelper()->result.empty() && !values->GetPathHelper()->result.empty())
					{
						if (JCUtil::IsOverlappedDirectoryPath(JCUtil::U8StrToWstr(values->GetNameHelper()->result),
							JCUtil::U8StrToWstr(values->GetPathHelper()->result)))
							MessageBox(0, L"Overlapped Project Name", 0, 0);
						else
						{
							SetStartProjectProccess();
							values->ClearInputHelperBuff();
							values->SetCreateProjectTrigger(false);
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
					values->ClearInputHelperBuff();
				}
				values->SetLoadProjectTrigger(false);
			}
		}
		void JProjectSelectorHub::OptionOnScreen()
		{
			ImGui::SetCursorScreenPos(values->GetTitleIconPos(J_TITLE_ICON::OPTION_SETTING));
			ImGui::BeginGroup();
			ImGui::BeginListBox("##ProjectSelector OptionList");
			JImGuiImpl::Selectable("Info", true);
			const bool isMouseClick = ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1);
			if (isMouseClick && !JImGuiImpl::IsMouseInRect(ImGui::GetWindowPos(), ImGui::GetWindowSize()))
				values->SetOptionTrigger(false);

			ImGui::EndListBox();
			ImGui::EndGroup();
		}
		void JProjectSelectorHub::SetStartProjectProccess()
		{
			const std::wstring newProejctName = JCUtil::U8StrToWstr(JCUtil::EraseSideChar(values->GetNameHelper()->result, ' '));
			const std::wstring newProejctPath = JCUtil::U8StrToWstr(JCUtil::EraseSideChar(values->GetPathHelper()->result, ' ')) + newProejctName;
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

			searchHelper->ClearInputBuffer();
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