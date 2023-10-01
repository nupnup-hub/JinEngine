#include"JProjectSelectorHub.h"
#include"../../JEditorAttribute.h"
#include"../../../Gui/JGui.h" 
#include"../../../Gui/JGuiImageInfo.h"
#include"../../../Align/JEditorAlignCalculator.h"
#include"../../../EditTool/JEditorSearchBarHelper.h"
#include"../../../EditTool/JEditorInputBuffHelper.h"
#include"../../../EditTool/JEditorDynamicColor.h"
#include"../../../Event/JEditorEvent.h"
#include"../../../../Core/Identity/JIdenCreator.h"
#include"../../../../Core/Utility/JMacroUtility.h"
#include"../../../../Core/Utility/JCommonUtility.h"
#include"../../../../Core/Math/JVectorExtend.h"
#include"../../../../Application/JApplicationEngine.h" 
#include"../../../../Application/JApplicationProject.h" 
#include"../../../../Application/JApplicationProjectPrivate.h" 
#include"../../../../Object/Resource/JResourceObjectHint.h" 
#include"../../../../Object/Resource/JResourceManager.h" 
#include"../../../../Object/Resource/JResourceObject.h"
#include"../../../../Object/Resource/Texture/JTexture.h"
#include"../../../../Object/Directory/JDirectory.h"
#include"../../../../Object/Directory/JFile.h"
#include"../../../../Graphic/GraphicResource/JGraphicResourceInterface.h"
#include"../../../../Window/JWindow.h" 

namespace JinEngine
{
	namespace Editor
	{
		namespace
		{
			using AppIOInterface = JApplicationProjectPrivate::IOInterface;
			using AppLifeInterface = JApplicationProjectPrivate::LifeInterface;
		}
		namespace Private
		{
			static constexpr J_GUI_WINDOW_FLAG_ guiWindowFlag = J_GUI_WINDOW_FLAG_NO_RESIZE |
				J_GUI_WINDOW_FLAG_NO_MOVE |
				J_GUI_WINDOW_FLAG_NO_COLLAPSE |
				J_GUI_WINDOW_FLAG_NO_DOCKING;

			static constexpr float paddingRate = 0.025f;
			static constexpr float frameThickness = 7.5f;

			//text and icon rate
			static constexpr float iconSizeRate = 0.75f;
			static constexpr float iconSpacingRate = 0.01f;

			//pList 
			static constexpr float pListSpacingRate = 0.02f;
			static constexpr float pListPaddingRate = 0.04f;
			static constexpr float pListPerColumn = 4;
			static constexpr float pListPerRow = 3;
			static constexpr float pListImageRate = 0.7f;
			static constexpr float pListInnerPaddingRate = 0.025f;

			//guide button
			//wnd : button
			static constexpr float buttonWidthRate = 0.1f;
			//alphasize : buttoon
			static constexpr float buttonHeightRate = 2.0f;
			static constexpr float buttonSpacingRate = 0.0125f;

			static constexpr uint pTotalButtonCount = 4;
			static constexpr uint pListButtonCount = 3;
			static constexpr uint pDetailButtonCount = 1;
			static constexpr uint pDetailButtonStartIndex = pListButtonCount;

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
		enum class J_PS_TRIGGER
		{
			CREATE_PROJECT,
			LOAD_PROEJCT,
			DESTROY_PROJECT,
			OPTION,
			OPTION_CREDIT,
			COUNT
		};
		enum class J_TITLE_ICON
		{
			OPTION_SETTING,
			//TEST,
			COUNT
		};

		struct SelectorValues
		{
		private:
			std::unique_ptr<JEditorInputBuffHelper> nameHelper;
			std::unique_ptr<JEditorInputBuffHelper> pathHelper;
			int selectProjectIndex = -1;
			bool triggers[(int)J_PS_TRIGGER::COUNT];
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
				for (uint i = 0; i < (uint)J_PS_TRIGGER::COUNT; ++i)
					triggers[i] = false;
			}
		public:
			void ClearInputHelperBuff()noexcept
			{
				nameHelper->Clear();
				pathHelper->Clear();
			}
			void OffButtonTrigger()noexcept
			{
				SetTrigger(J_PS_TRIGGER::CREATE_PROJECT, false);
				SetTrigger(J_PS_TRIGGER::LOAD_PROEJCT, false);
			}
		public:
			bool* GetTriggerPtr(const J_PS_TRIGGER type) noexcept
			{
				return &triggers[(int)type];
			}
			bool GetTrigger(const J_PS_TRIGGER type)const noexcept
			{
				return triggers[(int)type];
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
			void SetTrigger(const J_PS_TRIGGER type, const bool value)noexcept
			{
				triggers[(int)type] = value;
			}
			void SetNotTrigger(const J_PS_TRIGGER type)noexcept
			{
				triggers[(int)type] = !triggers[(int)type];
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
			JVector4F GetCanvasColor()const noexcept
			{
				//GetCanvasHigtLightColor 큰 값을 가져야한다.
				return JGui::GetColor(J_GUI_COLOR::HEADER) + JVector4F(0.2f, 0.2f, 0.2f, 0.2f);
			}
			JVector4F GetCanvasDeltaColor()const noexcept
			{
				return JVector4F(JVector3F((Graphic::Constants::backBufferClearColor * 0.325f).xyz), 0);
				//return JVector4F(0.2125f, 0.098f, 0.098f, 0);
				//return JVector4F(0.25f, 0.098f, 0.125f, 0.05f);
				//return JVector4F(0.098f, 0.098f, 0.098f, 0);
			}
			JVector4F GetCanvasHigtLightColor()const noexcept
			{
				return JVector4F(0.1f, 0.1f, 0.1f, 0.15f);
				//return JVector4F(0.25f, 0.098f, 0.125f, 0.05f);
				//return JVector4F(0.098f, 0.098f, 0.098f, 0);
			}
			JVector4F GetFrameColor()const noexcept
			{
				return JGui::GetColor(J_GUI_COLOR::FRAME_BG);
			}
		public:
			J_SIMPLE_GET_SET(JVector2<float>, totalCanvasSize, TotalCanvasSize);
			J_SIMPLE_GET_SET(JVector2<float>, canvasPadding, CanvasPadding);
			J_SIMPLE_GET_SET(JVector2<float>, canvasSpacing, CanvasSpacing);
			J_SIMPLE_GET_SET(int, selectProjectIndex, SelectedIndex);
			J_SIMPLE_UNIQUE_P_GET(JEditorInputBuffHelper, nameHelper, NameHelper);
			J_SIMPLE_UNIQUE_P_GET(JEditorInputBuffHelper, pathHelper, PathHelper);
		};
		class JProjectSelectorHubCreationImpl
		{
		public:
			using DestroyProjectF = Core::JSFunctorType<void, JProjectSelectorHub*>;
		public:
			std::unique_ptr<DestroyProjectF::Functor> destroyF; 
		};


		JProjectSelectorHub::JProjectSelectorHub(const std::string& name,
			std::unique_ptr<JEditorAttribute> attribute,
			const J_EDITOR_PAGE_TYPE ownerPageType,
			const J_EDITOR_WINDOW_FLAG windowFlag)
			:JEditorWindow(name, std::move(attribute), ownerPageType, windowFlag)
		{
			creationImpl = std::make_unique<JProjectSelectorHubCreationImpl>();
			searchHelper = std::make_unique<JEditorSearchBarHelper>(false);
			dynamicCol = std::make_unique<JEditorDynamicSpotColor>();
			values = std::make_unique<SelectorValues>();
			InitializeCreationImpl();

			ResourceEvListener::AddEventListener(*JResourceObject::EvInterface(), GetGuid(), J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE);
			AppIOInterface::LoadProjectList();
		}
		JProjectSelectorHub::~JProjectSelectorHub()
		{
			ResourceEvListener::RemoveListener(*JResourceObject::EvInterface(), GetGuid());
			AppIOInterface::StoreProjectList();
		}
		void JProjectSelectorHub::InitializeCreationImpl()
		{
			auto destoryProjLam = [](JProjectSelectorHub* hub)
			{
				if (hub->values->GetSelectedIndex() == invalidIndex)
					return;

				const int selectedIndex = hub->values->GetSelectedIndex();
				Core::JIdentifier::BeginDestroy(hub->lastRSVec[selectedIndex].Release()); 
				hub->lastRSVec.erase(hub->lastRSVec.begin() + selectedIndex);
				AppLifeInterface::DestroyProject(selectedIndex);
			};
			using DestroyProjectF = JProjectSelectorHubCreationImpl::DestroyProjectF;
			creationImpl->destroyF = std::make_unique<DestroyProjectF::Functor>(destoryProjLam);
		}
		J_EDITOR_WINDOW_TYPE JProjectSelectorHub::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::PROJECT_SELECTOR_HUB;
		}
		void JProjectSelectorHub::UpdateWindow()
		{
			//ImGuiCond_FirstUseEver
			JGui::SetNextWindowSize(JWindow::GetClientSize(), J_GUI_CONDIITON_ONCE);
			JGui::SetNextWindowPos(JWindow::GetClientPosition(), J_GUI_CONDIITON_ONCE);

			J_GUI_WINDOW_FLAG_ flag = J_GUI_WINDOW_FLAG_NO_SCROLL_BAR |
				J_GUI_WINDOW_FLAG_NO_COLLAPSE |
				J_GUI_WINDOW_FLAG_NO_MOVE |
				J_GUI_WINDOW_FLAG_NO_TITLE_BAR;

			JGui::PushStyle(J_GUI_STYLE::WINDOW_ROUNDING, 0.0f);
			JGui::PushStyle(J_GUI_STYLE::WINDOW_PADDING, JVector2F(0.0f, 0.0f));
			EnterWindow(flag);
			JGui::PopStyle(2);
			if (IsActivated())
			{
				//JGui::AddImage(*backgroundTexture.Get(), viewport->WorkPos, viewport->WorkSize, false, IM_COL32(255, 255, 255, 50));
				UpdateMouseClick();
				UpdateCanvasSize();
				TitleOnScreen();
				ProjectListOnScreen();
				ProjectDetailOnScreen();
				GuideButtonOnScreen();

				if (values->GetTrigger(J_PS_TRIGGER::CREATE_PROJECT))
					CreateNewProjectOnScreen();
				if (values->GetTrigger(J_PS_TRIGGER::LOAD_PROEJCT))
					LoadProjectOnScreen();
				if (values->GetTrigger(J_PS_TRIGGER::DESTROY_PROJECT))
					RequestDestroyProject();
				if (values->GetTrigger(J_PS_TRIGGER::OPTION))
					OptionOnScreen();
			}
			CloseWindow();
		}
		void JProjectSelectorHub::UpdateCanvasSize()
		{
			JGui::SetCurrentWindowFontScale(2);
			const JVector2<float> titleTextSize = JGui::CalTextSize(ENGINE_NAME);

			JGui::SetCurrentWindowFontScale(0.9f);
			const JVector2<float> otherTextSize = JGui::CalTextSize(ENGINE_NAME);
			JGui::SetCurrentWindowFontScale(2);

			JVector2<float> canvasPos[(int)J_PS_CANVAS::COUNT];
			JVector2<float> canvasSize[(int)J_PS_CANVAS::COUNT];

			const JVector2<float> wndSize = JGui::GetWindowContentsSize();
			const JVector2<float> padding = wndSize * Private::paddingRate;
			const JVector2<float> canvasSpacing = wndSize * JVector2<float>(0.0125f, 0.025f);

			const JVector2<float> pListCanvasRate = JVector2<float>(0.7f, 0.8f);
			const JVector2<float> pDetailCanvasRate = JVector2<float>(0.3f, 0.8f);

			canvasPos[(int)J_PS_CANVAS::TITLE] = JGui::GetCursorPos() + padding;
			canvasSize[(int)J_PS_CANVAS::TITLE] = JVector2<float>(wndSize.x - padding.x * 2, titleTextSize.y * 1.25f);

			canvasPos[(int)J_PS_CANVAS::PROJECT_LIST].x = canvasPos[(int)J_PS_CANVAS::TITLE].x;
			canvasPos[(int)J_PS_CANVAS::PROJECT_LIST].y = canvasPos[(int)J_PS_CANVAS::TITLE].y + canvasSize[(int)J_PS_CANVAS::TITLE].y + canvasSpacing.y;
			canvasSize[(int)J_PS_CANVAS::PROJECT_LIST] = (wndSize - padding * 2) * pListCanvasRate;
			canvasSize[(int)J_PS_CANVAS::PROJECT_LIST].x -= canvasSpacing.x / 2;

			canvasPos[(int)J_PS_CANVAS::PROJECT_DETAIL].x = canvasPos[(int)J_PS_CANVAS::PROJECT_LIST].x + canvasSize[(int)J_PS_CANVAS::PROJECT_LIST].x + canvasSpacing.x;
			canvasPos[(int)J_PS_CANVAS::PROJECT_DETAIL].y = canvasPos[(int)J_PS_CANVAS::PROJECT_LIST].y;
			canvasSize[(int)J_PS_CANVAS::PROJECT_DETAIL] = (wndSize - padding * 2) * pDetailCanvasRate;
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
			JGui::SetCurrentWindowFontScale(1);

			dynamicCol->Clear();
			JVector2F titleMin = canvasPos[(int)J_PS_CANVAS::TITLE];
			JVector2F titleMax = canvasPos[(int)J_PS_CANVAS::TITLE] + canvasSize[(int)J_PS_CANVAS::TITLE];
			JVector2F pListMin = canvasPos[(int)J_PS_CANVAS::PROJECT_LIST];
			JVector2F pListMax = canvasPos[(int)J_PS_CANVAS::PROJECT_LIST] + canvasSize[(int)J_PS_CANVAS::PROJECT_LIST];
			JVector2F pDetailMin = canvasPos[(int)J_PS_CANVAS::PROJECT_DETAIL];
			JVector2F pDetailMax = canvasPos[(int)J_PS_CANVAS::PROJECT_DETAIL] + canvasSize[(int)J_PS_CANVAS::PROJECT_DETAIL];

			JVector2F maxP = JVector2F::Max(JVector2F::Max(titleMax, pListMax), pDetailMax);
			//up left, up right, down left, down right
			dynamicCol->PushPoint(titleMin, maxP);
			dynamicCol->PushPoint(JVector2F(titleMax.x, titleMin.y), maxP);
			dynamicCol->PushPoint(JVector2F(titleMin.x, titleMax.y), maxP);
			dynamicCol->PushPoint(titleMax, maxP);

			dynamicCol->PushPoint(pListMin, maxP);
			dynamicCol->PushPoint(JVector2F(pListMax.x, pListMin.y), maxP);
			dynamicCol->PushPoint(JVector2F(pListMin.x, pListMax.y), maxP);
			dynamicCol->PushPoint(pListMax, maxP);

			dynamicCol->PushPoint(pDetailMin, maxP);
			dynamicCol->PushPoint(JVector2F(pDetailMax.x, pDetailMin.y), maxP);
			dynamicCol->PushPoint(JVector2F(pDetailMin.x, pDetailMax.y), maxP);
			dynamicCol->PushPoint(pDetailMax, maxP);

			dynamicCol->SetUpdateSpeed(0.00001f);
			dynamicCol->SetColor(values->GetCanvasColor());
			dynamicCol->SetDeltaColor(values->GetCanvasDeltaColor());
			dynamicCol->SetHighLight(values->GetCanvasHigtLightColor(), -0.9f, 0.45f);
			dynamicCol->Update();
		}
		void JProjectSelectorHub::TitleOnScreen()
		{
			JGui::SetCurrentWindowFontScale(1.5f);
			const JVector2<float> textSize = JGui::CalTextSize("JinEngine");
			const JVector2<float> iconSize = CreateVec2(JGui::GetFontSize() * 0.7f);
			const JVector2<float> iconSpacing = values->GetTotalCanvasSize() * Private::iconSpacingRate;

			JGui::SetCursorPos(values->GetCanvasPos(J_PS_CANVAS::TITLE));
			JGui::DrawRectFilledMultiColor(JGui::GetCursorScreenPos(),
				values->GetCanvasSize(J_PS_CANVAS::TITLE),
				dynamicCol->GetColor(0),
				dynamicCol->GetColor(1),
				dynamicCol->GetColor(2),
				dynamicCol->GetColor(3),
				true);
			JGui::DrawRectFrame(JGui::GetCursorScreenPos(),
				values->GetCanvasSize(J_PS_CANVAS::TITLE),
				values->GetFrameColor(),
				Private::frameThickness,
				true);

			JGui::SetCursorPosX(JGui::GetCursorPosX() + JGui::CalTextSize("J").x * 0.5f);
			JGui::SetCursorPosY(JGui::GetCursorPosY() + (values->GetCanvasSize(J_PS_CANVAS::TITLE).y - textSize.y) * 0.5f);
			JEditorTextAlignCalculator textAlign;
			textAlign.Update("JinEngine", values->GetCanvasSize(J_PS_CANVAS::TITLE), false);
			JGui::Text(textAlign.LeftAligned());

			float iconCanvasWidth = (iconSize.x * (int)J_TITLE_ICON::COUNT) +
				(iconSpacing.x * ((int)J_TITLE_ICON::COUNT - 1)) +
				(values->GetCanvasPadding().x) +
				(iconSize.x * 0.3f);		//icon selectable영역이 canavas를 초과할수있으므로 offset값이 필요
			float iconCanvasHeight = iconSize.y;

			const JVector2<float>canvasSize = JVector2<float>(iconCanvasWidth, iconCanvasHeight);
			JVector2<float>canvasPos;
			canvasPos.x = values->GetTotalCanvasSize().x - canvasSize.x;
			canvasPos.y = JGui::GetCursorPosY() - iconSize.y;

			JEditorStaticAlignCalculator iconAlignCal;
			iconAlignCal.Update(canvasSize, iconSize, CreateVec2(0), iconSpacing, canvasPos);

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
				[]() {return _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::OPTION_SETTING).Get(); },
				//[]() {return _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::OPTION_SETTING).Get(); }
				//[]() {return _JResourceManager::Instance().Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::OPTION_SETTING); },
				//[]() {return _JResourceManager::Instance().Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::OPTION_SETTING); }
			};
			MenuIconPtr iconPtr[(int)J_TITLE_ICON::COUNT]
			{
				[](SelectorValues& value) {value.SetNotTrigger(J_PS_TRIGGER::OPTION); },
				//[](SelectorValues& value) {value.SetNotTrigger(J_PS_TRIGGER::OPTION); }
				//[](SelectorValues& value) {MessageBox(0, L"Click2", 0, 0); },
				//[](SelectorValues& value) {MessageBox(0, L"Click3", 0, 0); }
			};

			for (uint i = 0; i < (int)J_TITLE_ICON::COUNT; ++i)
			{
				iconAlignCal.SetNextContentsPosition();
				values->SetTitleIconPos((J_TITLE_ICON)i, JGui::GetCursorScreenPos());
				bool isOpen = true;

				JGuiImageInfo imageInfo((*getTexturePtr[i])());
				if (JGui::ImageSwitch(uniqueLabel[i],
					imageInfo,
					isOpen,
					false,
					iconSize,
					JGui::GetColor(J_GUI_COLOR::CHILD_BG),
					JVector4F::Zero()))
					(*iconPtr[i])(*values);
			}
			JGui::SetCurrentWindowFontScale(1);
		}
		void JProjectSelectorHub::ProjectListOnScreen()
		{
			JGui::SetCurrentWindowFontScale(0.9f);
			JGui::SetCursorPos(values->GetCanvasPos(J_PS_CANVAS::PROJECT_LIST));
			/*
					JGui::DrawRectFilledMultiColor(JGui::GetCursorScreenPos(),
				values->GetCanvasSize(J_PS_CANVAS::PROJECT_LIST),
				values->GetCanvasColor(),
				values->GetCanvasDeltaColor(),
				true,
				J_GUI_ORDINAL_DIR_UP_LEFT_DOWN_LEFT);
			*/
			JGui::DrawRectFilledMultiColor(JGui::GetCursorScreenPos(),
				values->GetCanvasSize(J_PS_CANVAS::PROJECT_LIST),
				dynamicCol->GetColor(4),
				dynamicCol->GetColor(5),
				dynamicCol->GetColor(6),
				dynamicCol->GetColor(7),
				true);

			//has order dependency
			//child window내부에서는 window pos + size를 벗어날수없으므로
			//frame 외부가 잘린다
			JGui::DrawRectFrame(JGui::GetCursorScreenPos(),
				values->GetCanvasSize(J_PS_CANVAS::PROJECT_LIST),
				values->GetFrameColor(),
				Private::frameThickness,
				true);

			JGui::SetCursorPos(JGui::GetCursorPos() + Private::frameThickness);
			JGui::BeginChildWindow("##ProjectList", values->GetCanvasSize(J_PS_CANVAS::PROJECT_LIST));
			searchHelper->UpdateSearchBar();

			const JVector2<float> contentsPadding = values->GetTotalCanvasSize() * JVector2<float>(Private::pListPaddingRate, Private::pListPaddingRate * 0.5f);
			const JVector2<float> contentsSpacing = values->GetTotalCanvasSize() * Private::pListSpacingRate;
			float hoveredFrameThickness = (contentsSpacing.x > contentsSpacing.y ? contentsSpacing.x : contentsSpacing.y);//* 2

			JEditorDynamicAlignCalculator<2> widgetAlignCal;
			widgetAlignCal.Update(values->GetCanvasSize(J_PS_CANVAS::PROJECT_LIST),
				contentsPadding + (contentsSpacing * 0.5f),
				contentsSpacing,
				Private::pListPerColumn,
				Private::pListPerRow,
				{ CreateVec2(Private::pListImageRate), JVector2<float>(Private::pListImageRate, 1 - Private::pListImageRate) },
				{ CreateVec2(0), CreateVec2(0) },
				J_EDITOR_INNER_ALGIN_TYPE::COLUMN,
				JVector2<float>(0, JGui::GetCursorPosY()));

			JGui::PushColor(J_GUI_COLOR::HEADER, JVector4F::Zero());
			JGui::PushColor(J_GUI_COLOR::HEADER_HOVERED, JVector4F::Zero());
			JGui::PushColor(J_GUI_COLOR::HEADER_ACTIVE, JVector4F::Zero());

			JEditorTextAlignCalculator txtAlignCal;
			const uint projectListCount = JApplicationProject::GetProjectInfoCount();
			for (uint i = 0; i < projectListCount; ++i)
			{
				JApplicationProjectInfo* info = JApplicationProject::GetProjectInfo(i);
				if (!searchHelper->CanSrcNameOnScreen(info->GetName()))
					continue;

				JVector2<float> nowSize = widgetAlignCal.GetInnerContentsSize();
				widgetAlignCal.SetNextContentsPosition();
				bool isSelected = i == values->GetSelectedIndex();
				bool isHovered = JGui::IsMouseInRect(JGui::GetCursorScreenPos(), nowSize);

				JVector4<float> addedFrameColor = GetSelectableColorFactor(isSelected, isHovered);
				if (lastRSVec[i].IsValid())
				{
					const float contetentsFrameThickness = (isSelected || isHovered) ? hoveredFrameThickness : Private::frameThickness;
					JGui::DrawRectFrame(JGui::GetCursorScreenPos(),
						nowSize,
						values->GetFrameColor() + addedFrameColor,
						contetentsFrameThickness,
						true);

					JGuiImageInfo imageInfo(lastRSVec[i].Get());
					if (JGui::ImageSelectable(JCUtil::WstrToU8Str(info->GetName()), imageInfo,
						isSelected, false,
						nowSize,
						true))
						values->SetSelectedIndex(i);
				}
				else
				{
					JGui::DrawRectFrame(JGui::GetCursorScreenPos(), nowSize, values->GetFrameColor() + addedFrameColor, Private::frameThickness, true);
					if (JGui::Switch("##" + JCUtil::WstrToU8Str(info->GetName()) + "ProjectList", isSelected, false, nowSize))
						values->SetSelectedIndex(i);
				}

				nowSize = widgetAlignCal.GetInnerContentsSize();
				widgetAlignCal.SetNextContentsPosition();

				txtAlignCal.Update(JCUtil::WstrToU8Str(info->GetName()), nowSize, true);
				JGui::Text(txtAlignCal.MiddleAligned());
			}
			JGui::EndChildWindow();
			JGui::SetCurrentWindowFontScale(1);

			JGui::PopColor(3);
			//const JVector2<float>
		}
		void JProjectSelectorHub::ProjectDetailOnScreen()
		{
			static const JVector2<float> pDetailImageSizeRate = JVector2<float>(1.0f, 0.3f);
			static const JVector2<float> pDetailImagePaddingRate = JVector2<float>(0.2f, 0.075f);
			static const JVector2<float> pDetailTextPaddingRate = JVector2<float>(0.025f, 0.075f);

			JGui::SetCurrentWindowFontScale(0.9f);
			JGui::SetCursorPos(values->GetCanvasPos(J_PS_CANVAS::PROJECT_DETAIL));

			/*JGui::DrawRectFilledMultiColor(JGui::GetCursorScreenPos(),
				values->GetCanvasSize(J_PS_CANVAS::PROJECT_DETAIL),
				values->GetCanvasColor(),
				values->GetCanvasDeltaColor(),
				true,
				J_GUI_ORDINAL_DIR_UP_RIGHT_DOWN_RIGHT);*/

			JGui::DrawRectFilledMultiColor(JGui::GetCursorScreenPos(),
				values->GetCanvasSize(J_PS_CANVAS::PROJECT_DETAIL),
				dynamicCol->GetColor(8),
				dynamicCol->GetColor(9),
				dynamicCol->GetColor(10),
				dynamicCol->GetColor(11),
				true);

			//has order dependency
			//child window내부에서는 window pos + size를 벗어날수없으므로
			//frame 외부가 잘린다
			JGui::DrawRectFrame(JGui::GetCursorScreenPos(),
				values->GetCanvasSize(J_PS_CANVAS::PROJECT_DETAIL),
				values->GetFrameColor(),
				Private::frameThickness,
				true);
			JGui::BeginChildWindow("##ProjectDetail", values->GetCanvasSize(J_PS_CANVAS::PROJECT_DETAIL));

			const JVector2<float> canvasSize = values->GetCanvasSize(J_PS_CANVAS::PROJECT_DETAIL);
			const JVector2<float> imageSize = (canvasSize - canvasSize * pDetailImagePaddingRate * 2) * pDetailImageSizeRate;
			const JVector2<float> imagePos = canvasSize * pDetailImagePaddingRate;

			const JVector2<float> textPadding = canvasSize * pDetailTextPaddingRate;
			const JVector2<float> textPos = textPadding;
			JApplicationProjectInfo* info = nullptr;
			JGui::SetCursorPos(imagePos);

			if (values->GetSelectedIndex() != -1)
			{
				info = JApplicationProject::GetProjectInfo(values->GetSelectedIndex());
				if (lastRSVec[values->GetSelectedIndex()].IsValid())
				{
					JGuiImageInfo imageInfo(lastRSVec[values->GetSelectedIndex()].Get());
					JGui::AddRoundedImage(imageInfo, JGui::GetCursorScreenPos(), JGui::GetCursorScreenPos() + imageSize);
					JGui::SetCursorPos(imagePos);
					JGui::DrawRectFrame(JGui::GetCursorScreenPos(), imageSize, values->GetFrameColor(), Private::frameThickness, true);
					JGui::SetCursorPos(imagePos + JVector2<float>(0, imageSize.y + textPadding.y));
				}
				else
				{
					JGui::DrawRectFilledColor(JGui::GetCursorScreenPos(), imageSize, JGui::GetColor(J_GUI_COLOR::BUTTON), false);
					JGui::SetCursorPos(imagePos);
					JGui::DrawRectFrame(JGui::GetCursorScreenPos(), imageSize, values->GetFrameColor(), Private::frameThickness, true);
					JGui::SetCursorPos(imagePos + JVector2<float>(0, imageSize.y + textPadding.y));
				}
			}
			else
			{
				JGui::DrawRectFilledColor(JGui::GetCursorScreenPos(), imageSize, JGui::GetColor(J_GUI_COLOR::BUTTON), true);
				JGui::DrawRectFrame(JGui::GetCursorScreenPos(), imageSize, values->GetFrameColor(), Private::frameThickness, true);
			}

			auto pInfoOnScreen = [](const JVector2<float> padding, const std::string& guide, const std::string& info)
			{
				JGui::SetCursorPosX(padding.x);
				//JGui::SetCursorPosY(ImGui::GetCursorPos().y + padding.y);
				JGui::Text(guide);
				JGui::SameLine();
				JGui::Text(info);
			};

			JEditorTextAlignCalculator txtAlignCal;
			if (info != nullptr)
			{
				const std::string path = JCUtil::WstrToU8Str(info->GetPath());
				txtAlignCal.Update(path, JVector2<float>(JGui::GetWindowContentsSize().x - textPadding.x * 2, 0), false);

				pInfoOnScreen(textPadding, "Name: ", JCUtil::WstrToU8Str(info->GetName()));
				pInfoOnScreen(textPadding, "Path: ", txtAlignCal.LeftAligned());
				pInfoOnScreen(textPadding, "Version: ", JCUtil::WstrToU8Str(info->GetVersion()));
				pInfoOnScreen(textPadding, "Created date: ", info->GetCreatedTime().ToString());
				pInfoOnScreen(textPadding, "Last updated date: ", info->GetLastUpdateTime().ToString());
			}
			else
			{
				txtAlignCal.Update("Please, select project", JGui::GetWindowContentsSize(), false);
				const float cursorPosY = (canvasSize.y - JGui::GetCursorPosY()) / 2 + JGui::GetFontSize() * 0.5f;
				JGui::SetCursorPosX(textPadding.x);
				JGui::SetCursorPosY(cursorPosY);
				JGui::Text(txtAlignCal.MiddleAligned());
			}
			JGui::EndChildWindow();
			JGui::SetCurrentWindowFontScale(1);
		}
		void JProjectSelectorHub::GuideButtonOnScreen()
		{
			JGui::SetCurrentWindowFontScale(0.9f);
			struct LayoutData
			{
			public:
				const JVector2<float> wndSize = JGui::GetWindowContentsSize();
				const JVector2<float> alphabetSize = CreateVec2(JGui::GetFontSize());
				const JVector2<float> btnSize = JVector2<float>(wndSize.x * Private::buttonWidthRate, alphabetSize.y * Private::buttonHeightRate);
				const JVector2<float> btnSpacing = wndSize * Private::buttonSpacingRate;
			public:
				float listCanvasEndPosX;
				float detailCanvasEndPosX;
			public:
				float listBtnPosX;
				JVector2<float> listBtnPos;
				JVector2<float> listBtnSize;
			public:
				float detailBtnPosX;
				JVector2<float> detailBtnPos;
				JVector2<float> detailBtnSize;
			public:
				LayoutData(SelectorValues* values)
				{
					listCanvasEndPosX = values->GetCanvasPos(J_PS_CANVAS::PROJECT_LIST).x + values->GetCanvasSize(J_PS_CANVAS::PROJECT_LIST).x;
					detailCanvasEndPosX = values->GetCanvasPos(J_PS_CANVAS::PROJECT_DETAIL).x + values->GetCanvasSize(J_PS_CANVAS::PROJECT_DETAIL).x;

					listBtnPosX = listCanvasEndPosX - Private::pListButtonCount * (btnSize.x + btnSpacing.x) + btnSpacing.x;
					listBtnPos = JVector2<float>(listBtnPosX, values->GetCanvasPos(J_PS_CANVAS::GUIDE_BUTTON).y);
					listBtnSize = JVector2<float>(listCanvasEndPosX - listBtnPos.x, btnSize.y);

					detailBtnPosX = detailCanvasEndPosX - Private::pDetailButtonCount * (btnSize.x + btnSpacing.x) + btnSpacing.x;
					detailBtnPos = JVector2<float>(detailBtnPosX, values->GetCanvasPos(J_PS_CANVAS::GUIDE_BUTTON).y);
					detailBtnSize = JVector2<float>(detailCanvasEndPosX - detailBtnPos.x, btnSize.y);
				}
			};

			const LayoutData layOutData = LayoutData(values.get());

			JEditorStaticAlignCalculator btnAlignCal;
			btnAlignCal.Update(layOutData.listBtnSize,
				layOutData.btnSize,
				JVector2<float>(0, 0),
				layOutData.btnSpacing,
				layOutData.listBtnPos);

			using GuidButtonPtr = void(*)(SelectorValues& value);
			using CanSelectButtonPtr = bool(*)(const SelectorValues& value);
			using ControllLayoutPtr = void(*)(JEditorStaticAlignCalculator& cal, const LayoutData& data);
			//Left side buttion
			/**
			*	ordered by
			* 	Creaet Project,	---list btn start---
			*	Load Project,
			*	Destroy Project,
			*	Start Project	---detail btn start---
			*/
			std::string pListBtnName[Private::pTotalButtonCount]
			{
				"Create##ProjectSelector",
				"Load##ProjectSelector",
				"Destroy##ProjectSelector",
				"Start##ProjectSelector"
			};
			CanSelectButtonPtr pListSelectCondition[Private::pTotalButtonCount]
			{
				[](const SelectorValues& value) {return true; },
				[](const SelectorValues& value) {return true; },
				[](const SelectorValues& value) {return value.GetSelectedIndex() != invalidIndex; },
				[](const SelectorValues& value) {return value.GetSelectedIndex() != invalidIndex; },
			};
			GuidButtonPtr pListPtr[Private::pTotalButtonCount]
			{
				[](SelectorValues& value)
				{
					value.ClearInputHelperBuff();
					value.OffButtonTrigger();
					value.SetTrigger(J_PS_TRIGGER::CREATE_PROJECT, true);
				},
				[](SelectorValues& value)
				{
					value.ClearInputHelperBuff();
					value.OffButtonTrigger();
					value.SetTrigger(J_PS_TRIGGER::LOAD_PROEJCT, true);
				},
				[](SelectorValues& value)
				{
					value.ClearInputHelperBuff();
					value.OffButtonTrigger();
					value.SetTrigger(J_PS_TRIGGER::DESTROY_PROJECT, true);
				},
				[](SelectorValues& value)
				{
					value.ClearInputHelperBuff();
					value.OffButtonTrigger();
					auto projInfo = JApplicationProject::GetProjectInfo(value.GetSelectedIndex());
					if (projInfo == nullptr)
					{
						MessageBox(0, L"Fail get project info", 0, 0);
						return;
					}
					AppLifeInterface::SetNextProjectInfo(projInfo->CreateReplica());
					if (!AppLifeInterface::SetStartNewProjectTrigger())
						MessageBox(0, L"Fail start new project", 0, 0);
				}
			};
			ControllLayoutPtr pListLayoutCtrl[Private::pTotalButtonCount]
			{
				[](JEditorStaticAlignCalculator& cal, const LayoutData& data) {	cal.SetNextContentsPosition(); },
				[](JEditorStaticAlignCalculator& cal, const LayoutData& data) {	cal.SetNextContentsPosition(); },
				[](JEditorStaticAlignCalculator& cal, const LayoutData& data) {	cal.SetNextContentsPosition(); },
				[](JEditorStaticAlignCalculator& cal, const LayoutData& data)
				{
					cal.Update(data.detailBtnSize,
						data.btnSize,
						JVector2<float>(0, 0),
						data.btnSpacing,
						data.detailBtnPos);
					cal.SetNextContentsPosition();
				}
			};

			JGui::PushColor(J_GUI_COLOR::BUTTON, JVector4F::Zero());
			for (uint i = 0; i < Private::pTotalButtonCount; ++i)
			{
				pListLayoutCtrl[i](btnAlignCal, layOutData);
				JGui::DrawRectFilledColor(JGui::GetCursorScreenPos(),
					layOutData.btnSize,
					values->GetCanvasColor(),
					true);

				JGui::DrawRectFrame(JGui::GetCursorScreenPos(),
					layOutData.btnSize,
					values->GetFrameColor(),
					Private::frameThickness,
					true);

				const bool canSelect = (*pListSelectCondition[i])(*values);
				if (!canSelect)
					JGui::PushButtonColorDeActSet();
				if (JGui::Button(pListBtnName[i], layOutData.btnSize) && canSelect)
					(*pListPtr[i])(*values);
				if (!canSelect)
					JGui::PopButtonColorDeActSet();
			}
			JGui::PopColor(); 
			JGui::SetCurrentWindowFontScale(1);
		}
		void JProjectSelectorHub::CreateNewProjectOnScreen()
		{
			JGui::SetCurrentWindowFontScale(0.9f);
			JGui::SetNextWindowFocus();
			JGui::SetNextWindowSize(JVector2F(JWindow::GetClientSize().x * 0.7f, JWindow::GetClientSize().y * 0.6f));
			if (JGui::BeginWindow("Create New Project##Create Project Window" + GetName(), values->GetTriggerPtr(J_PS_TRIGGER::CREATE_PROJECT), Private::guiWindowFlag))
			{
				//JGui::SetCurrentWindowFontScale(0.6f);
				//JGui::SetCursorPosY(JWindow::Instance().GetClientHeight() * 0.4f);
				JGui::Text("Project Name: ");
				JGui::InputText("##NameInput_ProjectSelector", values->GetNameHelper()->buff, values->GetNameHelper()->result, "Name...");

				JGui::Text("Folder: ");
				JGui::InputText("##PathInput_ProjectSelector", values->GetPathHelper()->buff, values->GetPathHelper()->result, "Path...", J_GUI_INPUT_TEXT_FLAG_READ_ONLY);

				JGui::SameLine();
				const float iconSize = JGui::GetWindowSize().x * Private::searchIconWidthRate;

				JGuiImageInfo imageInfo(serachIconTexture.Get());
				if (JGui::ImageButton("##SearchDir_ProjectSelector", imageInfo, CreateVec2(iconSize)))
				{
					std::wstring dirPath;
					if (JWindow::SelectDirectory(dirPath, L"please, select project location") && JWindow::HasStorageSpace(dirPath, necessaryCapacityMB))
					{
						values->GetPathHelper()->result = JCUtil::WstrToU8Str(dirPath);
						values->GetPathHelper()->buff = values->GetPathHelper()->result;
						values->GetPathHelper()->buff.resize(values->maxPathRange);
					}
				}

				JGui::Text("Version: ");
				JGui::ComboSet("##Create Project Window Version Combo", versionIndex, JApplicationEngine::GetAppVersion());
				if (JGui::Button("Create Proejct##Create Project Window"))
				{
					if (!values->GetNameHelper()->result.empty() && !values->GetPathHelper()->result.empty())
					{
						if (JCUtil::IsOverlappedDirectoryPath(JCUtil::U8StrToWstr(values->GetNameHelper()->result),
							JCUtil::U8StrToWstr(values->GetPathHelper()->result)))
							MessageBox(0, L"Overlapped Project Name", 0, 0);
						else
						{
							CreateProjectProccess();
							values->ClearInputHelperBuff();
							values->SetTrigger(J_PS_TRIGGER::CREATE_PROJECT, false);
						}
					}
				}
				JGui::EndWindow();
			}
			JGui::SetCurrentWindowFontScale(1);
		}
		void JProjectSelectorHub::LoadProjectOnScreen()
		{
			std::wstring dirPath;
			if (JWindow::SelectDirectory(dirPath, L"please, select project directory") && JWindow::HasStorageSpace(dirPath, necessaryCapacityMB))
			{
				if (JApplicationProject::IsValidPath(dirPath))
				{
					std::unique_ptr<JApplicationProjectInfo> pInfo;
					JApplicationProjectInfo* existingInfo = JApplicationProject::GetProjectInfo(dirPath);
					if (existingInfo != nullptr)
						pInfo = existingInfo->CreateReplica();
					else
						pInfo = AppLifeInterface::MakeProjectInfo(dirPath);

					AppLifeInterface::SetNextProjectInfo(std::move(pInfo));
					if (!AppLifeInterface::SetStartNewProjectTrigger())
						MessageBox(0, L"Load Project Fail", 0, 0);
					values->ClearInputHelperBuff();
				}
				values->SetTrigger(J_PS_TRIGGER::LOAD_PROEJCT, false);
			}
			else
				values->SetTrigger(J_PS_TRIGGER::LOAD_PROEJCT, false);
		}
		void JProjectSelectorHub::OptionOnScreen()
		{
			auto setInnerTrigger = [](SelectorValues& values, const bool value)
			{
				constexpr uint innerTriggerCount = (int)J_PS_TRIGGER::COUNT - (int)J_PS_TRIGGER::OPTION - 1;
				J_PS_TRIGGER optionTriggers[innerTriggerCount]
				{
					J_PS_TRIGGER::OPTION_CREDIT
				};
				for (uint i = 0; i < innerTriggerCount; ++i)
					values.SetTrigger((J_PS_TRIGGER)((uint)J_PS_TRIGGER::OPTION + i), value);
			};

			const JVector2<float> optWndSize = JVector2F(JWindow::GetClientSize().x * 0.2f, JWindow::GetClientSize().y * 0.6f);
			const JVector2<float> optWndPos = values->GetTitleIconPos(J_TITLE_ICON::OPTION_SETTING) - JVector2<float>(optWndSize.x, 0);
			JGui::SetNextWindowFocus();
			JGui::SetNextWindowPos(optWndPos);
			JGui::SetNextWindowSize(optWndSize);
			if (JGui::BeginWindow("Opttion##Create Project Window" + GetName(), values->GetTriggerPtr(J_PS_TRIGGER::OPTION), Private::guiWindowFlag))
			{
				JGui::SetCurrentWindowFontScale(1.2f);
				JGui::SetNextItemWidth(JGui::GetWindowSize().x);
				JGui::Separator();
				if (JGui::Selectable("Credit##" + GetName()))
					values->SetNotTrigger(J_PS_TRIGGER::OPTION_CREDIT);

				const bool isMouseClick = JGui::AnyMouseClicked(false);
				if (isMouseClick && !JGui::IsMouseInRect(JGui::GetWindowPos(), JGui::GetWindowSize()))
				{
					values->SetTrigger(J_PS_TRIGGER::OPTION, false);
					setInnerTrigger(*values, false);
				}
				JGui::EndWindow();
			}
			if (!values->GetTrigger(J_PS_TRIGGER::OPTION))
				setInnerTrigger(*values, false);

			if (values->GetTrigger(J_PS_TRIGGER::OPTION_CREDIT))
			{
				const std::string longStr = "Developed by jung jin woo";
				const float blankWidth = JGui::CalTextSize(" ").x;
				const float creditWndWidth = JGui::CalTextSize(longStr.c_str()).x + blankWidth * 2;
				const JVector2<float> creditWndSize = JVector2<float>(creditWndWidth, optWndSize.y * 0.5f);
				const JVector2<float> creditWndPos = optWndPos - JVector2<float>(creditWndSize.x, 0);
				JGui::SetNextWindowPos(creditWndPos);
				JGui::SetNextWindowSize(creditWndSize);

				if (JGui::BeginWindow("Credit##Create Project Window" + GetName(),
					values->GetTriggerPtr(J_PS_TRIGGER::OPTION_CREDIT),
					Private::guiWindowFlag))
				{
					JGui::SetCurrentWindowFontScale(0.6f);
					JGui::SetCursorPosX(JGui::GetCursorPosX() + blankWidth);
					JGui::Text("Developed by jung jin woo");
					JGui::SetCursorPosX(JGui::GetCursorPosX() + blankWidth);
					JGui::Text("Years: 2021 - 2023");
					JGui::SetCursorPosX(JGui::GetCursorPosX() + blankWidth);
					JGui::Text("Email: flcl1501@naver.com");
					JGui::EndWindow();
				}
			}
			JGui::SetCurrentWindowFontScale(1);
		}
		void JProjectSelectorHub::RequestDestroyProject()
		{
			if (!values->GetTrigger(J_PS_TRIGGER::DESTROY_PROJECT) || values->GetSelectedIndex() == invalidIndex)
			{
				values->SetTrigger(J_PS_TRIGGER::DESTROY_PROJECT, false);
				return;
			}
			using DestroyProjectF = JProjectSelectorHubCreationImpl::DestroyProjectF;
			auto destroyB = std::make_unique<DestroyProjectF::CompletelyBind>(*creationImpl->destroyF, this);
			auto evStruct = std::make_unique<JEditorBindFuncEvStruct>(std::move(destroyB), GetOwnerPageType());
			JEditor::AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::BIND_FUNC, JEditorEvent::RegisterEvStruct(std::move(evStruct)));
			values->SetTrigger(J_PS_TRIGGER::DESTROY_PROJECT, false);
		}
		void JProjectSelectorHub::CreateProjectProccess()
		{
			const std::wstring newProejctName = JCUtil::U8StrToWstr(JCUtil::EraseSideChar(values->GetNameHelper()->result, ' '));
			const std::wstring newProejctPath = JCUtil::U8StrToWstr(JCUtil::EraseSideChar(values->GetPathHelper()->result, ' ')) + L"\\" + newProejctName;
			std::vector<std::string> version = JApplicationEngine::GetAppVersion();

			AppLifeInterface::SetNextProjectInfo(AppLifeInterface::MakeProjectInfo(newProejctPath, version[versionIndex]));
			if (!AppLifeInterface::SetStartNewProjectTrigger())
				MessageBox(0, L"Start Project Fail", 0, 0);
		}
		void JProjectSelectorHub::LoadLastRsTexture()
		{
			JUserPtr<JDirectory> engineProjectRsFolder = _JResourceManager::Instance().GetDirectory(JApplicationEngine::ProjectLastRsPath());
			const uint projectListCount = JApplicationProject::GetProjectInfoCount();
			lastRSVec.resize(projectListCount);
			for (uint i = 0; i < projectListCount; ++i)
			{
				JApplicationProjectInfo* info = JApplicationProject::GetProjectInfo(i);
				const std::wstring path = info->lastRsPath();
				if (_waccess(path.c_str(), 00) == -1)
					continue;

				std::wstring folderPath;
				std::wstring name;
				std::wstring format;
				JCUtil::DecomposeFilePath(path, folderPath, name, format);

				JUserPtr<JFile> file = engineProjectRsFolder->GetDirectoryFileByFullName(name, format);
				if (file != nullptr)
					lastRSVec[i] = JUserPtr<JTexture>::ConvertChild(file->TryGetResourceUser());
				else
				{
					J_OBJECT_FLAG flag = (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNCOPYABLE);
					JUserPtr <JTexture> rsTexture = JICI::Create<JTexture>(name,
						Core::MakeGuid(),
						flag,
						JResourceObject::GetFormatIndex<JTexture>(format),
						engineProjectRsFolder,
						path);
					if (rsTexture == nullptr)
						assert(L"Load last project rs error");
					lastRSVec[i] = rsTexture;
				}
			}
		}
		void JProjectSelectorHub::DoSetOpen()noexcept
		{
			JEditorWindow::DoSetOpen();
			serachIconTexture = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::SEARCH_FOLDER_ICON);
			optionSettingTexture = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::OPTION_SETTING);
			//backgroundTexture = _JResourceManager::Instance().Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::PROJECT_SELECTOR_BACKGROUND);
			LoadLastRsTexture();
		}
		void JProjectSelectorHub::DoSetClose()noexcept
		{
			serachIconTexture.Clear();
			optionSettingTexture.Clear();
			//backgroundTexture.Clear();
			lastRSVec.clear();
			searchHelper->ClearInputBuffer();
			JEditorWindow::DoSetClose();
		}
		void JProjectSelectorHub::OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)
		{
			if (iden == GetGuid())
				return;

			if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
			{
				if (serachIconTexture.IsValid() && jRobj->GetGuid() == serachIconTexture->GetGuid())
					serachIconTexture.Clear();
				if (optionSettingTexture.IsValid() && jRobj->GetGuid() == optionSettingTexture->GetGuid())
					optionSettingTexture.Clear();
				if (backgroundTexture.IsValid() && jRobj->GetGuid() == backgroundTexture->GetGuid())
					backgroundTexture.Clear();
			}
		}
	}
}