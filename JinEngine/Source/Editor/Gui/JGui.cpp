#include"JGui.h" 
#include"JGuiOption.h"
#include"Adapter/JGuiBehaviorAdapter.h"
#include"Adapter/JGuiBehaviorAdaptee.h"
#include"JGuiImageInfo.h"
#include"../Page/Docking/JDockUpdateHelper.h"
#include"../../Core/Math/JVectorExtend.h"
#include"../../Object/Resource/Texture/JTexture.h"
#include"../../Object/Resource/JResourceManager.h"
#include"../../Core/File/JFileIOHelper.h"
#include"../../Graphic/Gui/JGuiBackendInterface.h"
#include"../../Graphic/JGraphic.h" 
#include"../../Graphic/JGraphicPrivate.h" 
#include"../../Graphic/GraphicResource/JGraphicResourceInterface.h"
#include"../../Graphic/GraphicResource/JGraphicResourceUserAccess.h"
#include"../../Window/JWindow.h" 
#include"../../Application/Engine/JApplicationEngine.h"
#include"../../Application/Project/JApplicationProject.h"
#include<bitset>
#include <sstream>
#include <iomanip>

//#include"../../Develop/Debug/JDevelopDebug.h"
namespace JinEngine::Editor
{
	namespace Private
	{	
		class JGuiImpl final : public Graphic::JGuiBackendInterface
		{
		private:
			using IntersectCondPtr = bool(*)(const JGuiWindowInfo&, std::string);
		private:
			const size_t guid = JCUtil::CalculateGuid(typeid(JGuiImpl).name());
		private:
			std::unique_ptr<JGuiBehaviorAdapter> adapter;
			JGuiBehaviorAdaptee* adaptee = nullptr;		//cash
		private:
			std::vector<JGuiWindowInfo> displayedWndInfo;	//sorted back to front
			std::vector<JGuiWindowInfo> lastDisplayedWndInfo;	//sorted displayed back to front
		private:
			uint actWidgetCount[(uint)Core::J_GUI_WIDGET_TYPE::COUNT];
		private:
			bool lastMouseClicked[(uint)Core::J_MOUSE_BUTTON::COUNT]; 
		public:
			JGuiOption option; 
		public:
			void Initialize(std::unique_ptr<Graphic::JGuiInitData>&& initData)
			{ 
				assert(adaptee != nullptr);
				adaptee->Initialize(std::move(initData));
			}
			void Clear()
			{
				adaptee->Clear();
			}
		public:
			void ReBuildGraphicBackend(std::unique_ptr<Graphic::JGuiInitData>&& initData)
			{
				assert(adaptee != nullptr);
				adaptee->ReBuildGraphicBackend(std::move(initData));
			}
		public:
			Graphic::GuiIdentification GetGuiIdentification()const noexcept final
			{
				return (Graphic::GuiIdentification)option.guiType;
			}
			JGuiBehaviorAdaptee* GetAdapter()
			{
				return adaptee;
			}
			JGuiBehaviorAdaptee* GetAdaptee()
			{
				return adaptee;
			} 
		private:
			std::wstring GetConfigFilePath()const noexcept
			{
				std::wstring path;
				if (JApplicationEngine::GetApplicationState() == J_APPLICATION_STATE::PROJECT_SELECT)
					path = JApplicationEngine::ConfigPath() + L"\\Gui.txt";
				else
					path = JApplicationProject::ConfigPath() + L"\\Gui.txt";

				return path;
			}
		public:
			void SetAdapter(std::unique_ptr<JGuiBehaviorAdapter>&& newAdapter)
			{
				adaptee = nullptr;
				adapter = std::move(newAdapter);
				if (adapter != nullptr)
				{
					auto gOption = JGraphic::Instance().GetGraphicOption();
					adaptee = adapter->GetAdaptee(option.guiType, gOption.deviceType);
				}
			} 
			void SetGui(const J_GUI_TYPE guiType)
			{
				if (option.guiType == guiType)
					return;

				auto gOption = JGraphic::Instance().GetGraphicOption();
				auto newAdaptee = adapter->GetAdaptee(guiType, gOption.deviceType);
				if (newAdaptee == nullptr)
					return;

				adaptee = newAdaptee;
				option.guiType = guiType;
			}
		public:
			bool IsLastMouseClicked(const Core::J_MOUSE_BUTTON btn)
			{
				return lastMouseClicked[(uint)btn];
			}
		public:
			void AddActWidgetCount(const Core::J_GUI_WIDGET_TYPE type)
			{
				++actWidgetCount[(uint)type];
			}
			void ClearActWidgetCount()
			{
				for (uint i = 0; i < (uint)Core::J_GUI_WIDGET_TYPE::COUNT; ++i)
					actWidgetCount[i] = 0;
			}
			uint TotalActWidgetCount()
			{
				uint sum = 0;
				for (uint i = 0; i < (uint)Core::J_GUI_WIDGET_TYPE::COUNT; ++i)
					sum += actWidgetCount[i];
				return sum;
			}
		public:
			void UpdateGuiBackend()
			{
				assert(adaptee != nullptr);
				adaptee->UpdateGuiBackend();
			} 
			void UpdateMouseEvent()
			{
				if (!adaptee->CanFocusByMouseRightClick() && lastMouseClicked[(uint)Core::J_MOUSE_BUTTON::RIGHT])
				{ 
					const JVector2F mousePos = adaptee->GetMousePos();
					JGuiWindowInfo* info = IntersectFirst(mousePos);
					if (info != nullptr && !info->isLastFrameFocused)
						adaptee->FocusWindow(info->windowID);
				}
			}
			void UpdateLastMouseState()
			{
				lastMouseClicked[(uint)Core::J_MOUSE_BUTTON::LEFT] = adaptee->IsMouseClicked(Core::J_MOUSE_BUTTON::LEFT);
				lastMouseClicked[(uint)Core::J_MOUSE_BUTTON::RIGHT] = adaptee->IsMouseClicked(Core::J_MOUSE_BUTTON::RIGHT);
				lastMouseClicked[(uint)Core::J_MOUSE_BUTTON::MIDDLE] = adaptee->IsMouseClicked(Core::J_MOUSE_BUTTON::MIDDLE);
			}
		public:
			JGuiWindowInfo* IntersectFirst(const JVector2F& pos)
			{
				const int count = (int)lastDisplayedWndInfo.size();
				if (count == 0)
					return nullptr;

				auto findHostWndPtr = GetIntersectCondPtr();
				JGuiWindowInfo* lastSelected = nullptr;
				int lastSelectedOrder = -1;
				 
				for (int i = count - 1; i >= 0; --i)
				{
					if (pos.Contained(lastDisplayedWndInfo[i].pos, lastDisplayedWndInfo[i].size))
					{
						if (lastDisplayedWndInfo[i].hasDockNode)
						{  
							JGuiWindowInfo hostWindowInfo;
							if (JGui::GetDockNodeHostWindowInfo(lastDisplayedWndInfo[i].dockID, hostWindowInfo))
							{
								std::string frontName = hostWindowInfo.windowName.substr(0, hostWindowInfo.windowName.find_first_of("##"));
								int hostWindowIndex = JCUtil::GetIndex(lastDisplayedWndInfo, findHostWndPtr, frontName);
								if (hostWindowIndex != invalidIndex && hostWindowIndex > lastSelectedOrder)
								{
									lastSelected = &lastDisplayedWndInfo[i];
									lastSelectedOrder = hostWindowIndex; 
								}
							}
						}
						else
						{
							if (i > lastSelectedOrder)
							{
								lastSelected = &lastDisplayedWndInfo[i];
								lastSelectedOrder = i; 
							} 
						} 
					}
				}
				return lastSelected;
			} 
		public:
			void AddDisplayedWindowInfo(JGuiWindowInfo&& lastDisplayedWindowInfo)
			{
				displayedWndInfo.push_back(std::move(lastDisplayedWindowInfo));
			}
			void ClearDisplayedWindowInfo()
			{
				lastDisplayedWndInfo = std::move(displayedWndInfo);  
				displayedWndInfo.clear();
			}
		public:
			void SettingGuiDrawing()final
			{
				assert(adaptee != nullptr);
				if (adaptee == nullptr)
					return;

				adaptee->SettingGuiDrawing();
			}
			void Draw(std::unique_ptr<Graphic::JGuiDrawData>&& drawData)final
			{
				assert(adaptee != nullptr);
				if (adaptee == nullptr)
					return;

				adaptee->Draw(std::move(drawData));
			}
		public:
			void LoadOption()
			{
				JFileIOTool tool;
				if (!tool.Begin(GetConfigFilePath(), JFileIOTool::TYPE::JSON, JFileIOTool::BEGIN_OPTION_JSON_TRY_LOAD_DATA))
					return;

				J_GUI_TYPE guiType;
				JFileIOHelper::LoadEnumData(tool, guiType, "GuiType");
				tool.Close();

				SetGui(guiType);
			}
			void StoreOption()
			{
				JFileIOTool tool;
				if (!tool.Begin(GetConfigFilePath(), JFileIOTool::TYPE::JSON))
					return;

				JFileIOHelper::StoreEnumData(tool, option.guiType, "GuiType");
				tool.Close(JFileIOTool::CLOSE_OPTION_JSON_STORE_DATA);
			}
		private:
			IntersectCondPtr GetIntersectCondPtr()
			{
				return [](const JGuiWindowInfo& a, std::string name){return JCUtil::Contain(a.windowName, name);}; 
			}
		};
		std::unique_ptr<Private::JGuiImpl> impl;
	}
	namespace Private
	{
		JGuiImpl* Impl()noexcept
		{
			if (impl == nullptr)
				impl = std::make_unique< JGuiImpl>();
			return impl.get();
		}
		JGuiBehaviorAdaptee* Adaptee()noexcept
		{
			return Impl()->GetAdaptee();
		}
	}

#pragma region JIdentifier  
	std::string JGui::CreateGuiLabel(Core::JIdentifier* iden) noexcept
	{
		return JCUtil::WstrToU8Str(iden->GetName()) + "##" + std::to_string(iden->GetGuid());
	}
	std::string JGui::CreateGuiLabel(Core::JIdentifier* iden, const std::string& text) noexcept
	{
		return JCUtil::WstrToU8Str(iden->GetName()) + "##" + std::to_string(iden->GetGuid());
	}
	std::string JGui::CreateGuiLabel(const JUserPtr<Core::JIdentifier>& iden) noexcept
	{
		return JCUtil::WstrToU8Str(iden->GetName()) + "##" + std::to_string(iden->GetGuid());
	}
	std::string JGui::CreateGuiLabel(const JUserPtr<Core::JIdentifier>& iden, const std::string& text) noexcept
	{
		return JCUtil::WstrToU8Str(iden->GetName()) + "##" + std::to_string(iden->GetGuid()) + text;
	}
	std::string JGui::CreateGuiLabel(const std::string& name, const std::string& uniqueLabel) noexcept
	{
		return  name + "##" + uniqueLabel;
	}
	std::string JGui::CreateGuiLabel(const std::string& name, const size_t guid) noexcept
	{
		return  name + "##" + std::to_string(guid);
	}
	std::string JGui::CreateGuiLabel(const std::string& name, const size_t guid, const std::string& uniqueLabel) noexcept
	{
		return  name + "##" + std::to_string(guid) + uniqueLabel;
	}
	std::string JGui::CreateGuiLabel(const size_t guid, const std::string& uniqueLabel) noexcept
	{
		return "##" + std::to_string(guid) + uniqueLabel;
	}
	std::string JGui::CreatePreviewName(Core::JIdentifier* iden) noexcept
	{
		return iden != nullptr ? JCUtil::WstrToU8Str(iden->GetName()) : "";
	}
#pragma endregion

#pragma region Color
	JVector4<float> JGui::GetSelectedWidgetColorFactor()noexcept
	{
		return JVector4<float>(0.35f, 0.35f, 0.35f, 0.35f);
	} 
	JVector4<float> JGui::GetUnFocusedWidgetColorFactor()noexcept
	{
		return JVector4<float>(0.2f, 0.2f, 0.2f, 0.2f);
	}
	JVector4<float> JGui::GetDeActivatedTextColorFactor()noexcept
	{
		return JVector4<float>(-0.3f, -0.3f, -0.3f, -0.25f);
	}
	JVector4<float> JGui::GetColor(const J_GUI_COLOR flag)noexcept
	{
		return Private::Adaptee()->GetColor(flag);
	}
	uint JGui::GetUColor(const J_GUI_COLOR flag)noexcept
	{
		return Private::Adaptee()->GetUColor(flag);
	}
	JVector4<float> JGui::GetSelectableColorFactor(const bool isFocus, const bool isSelecetd, const bool isHovered) noexcept
	{
		if (isSelecetd)
		{
			if (isFocus)
				return JGui::GetSelectedWidgetColorFactor();
			else
				return  JGui::GetUnFocusedWidgetColorFactor();
		}
		else if (isHovered)
		{
			if (isFocus)
				return JGui::GetSelectedWidgetColorFactor() * 0.75f;
			else
				return JGui::GetUnFocusedWidgetColorFactor();
		}
		else
			return JVector4F::Zero();
	}
	void JGui::SetColor(const J_GUI_COLOR flag, const JVector4<float>& color)noexcept
	{
		Private::Adaptee()->SetColor(flag, color);
	}
	void JGui::SetColorToSoft(const J_GUI_COLOR flag, const JVector4<float>& factor)noexcept
	{
		Private::Adaptee()->SetColor(flag, GetColor(flag) + factor);
	}
	void JGui::SetColorToDefault(const J_GUI_COLOR flag)noexcept
	{
		Private::Adaptee()->SetColorToDefault(flag);
	}
	void JGui::SetAllColorToSoft(const JVector4<float>& factor)noexcept
	{
		Private::Adaptee()->SetAllColorToSoft(factor);
	}
	void JGui::SetAllColorToDefault()noexcept
	{
		Private::Adaptee()->SetAllColorToDefault();
	} 
	uint32 JGui::ConvertUColor(const JVector4<float>& color)noexcept
	{
		return Private::Adaptee()->ConvertUColor(color);
	}
	void JGui::PushColor(const J_GUI_COLOR colType, const JVector4<float>& color)
	{
		Private::Adaptee()->PushColor(colType, color);
	}
	void JGui::PushColorToSoft(const J_GUI_COLOR colType, const JVector4<float>& factor)
	{
		Private::Adaptee()->PushColor(colType, GetColor(colType) + factor);
	}
	void JGui::PushTreeNodeColorToSoft(const JVector4<float>& factor)noexcept
	{ 
		PushColorToSoft(J_GUI_COLOR::HEADER, factor);
		PushColorToSoft(J_GUI_COLOR::HEADER_HOVERED, factor);
		PushColorToSoft(J_GUI_COLOR::HEADER_ACTIVE, factor);
	}
	void JGui::PushButtonColorToSoftSet(const JVector4<float>& factor)noexcept
	{ 
		PushColorToSoft(J_GUI_COLOR::BUTTON, factor);
		PushColorToSoft(J_GUI_COLOR::BUTTON_HOVERED, factor);
		PushColorToSoft(J_GUI_COLOR::BUTTON_ACTIVE, factor);
	}
	void JGui::PushButtonColorDeActSet()noexcept
	{
		PushColorToSoft(J_GUI_COLOR::TEXT, GetDeActivatedTextColorFactor());
		PushColorToSoft(J_GUI_COLOR::BUTTON, CreateVec4(0.2f));
		PushColor(J_GUI_COLOR::BUTTON_HOVERED, JVector4F::Zero());
		PushColor(J_GUI_COLOR::BUTTON_ACTIVE, JVector4F::Zero());
	} 
	void JGui::PopColor(const uint count)
	{
		Private::Adaptee()->PopColor(count);
	}
	void JGui::PopTreeNodeColorToSoftSet()
	{
		Private::Adaptee()->PopColor(3);
	}
	void JGui::PopButtonColorToSoftSet()
	{
		Private::Adaptee()->PopColor(3);
	}
	void JGui::PopButtonColorDeActSet()
	{
		Private::Adaptee()->PopColor(4);
	}
	void JGui::PushTreeNodeColorSet(const bool isFocus, const bool isActivated, const bool isSelected)noexcept
	{
		if (!isActivated)
			JGui::PushColorToSoft(J_GUI_COLOR::TEXT, JGui::GetDeActivatedTextColorFactor());
		if (isSelected)
			JGui::PushTreeNodeColorToSoft(GetSelectableColorFactor(isFocus, true, true));
	}
	void JGui::PopTreeNodeColorSet(const bool isActivated, const bool isSelected)noexcept
	{
		if (isSelected)
			JGui::PopTreeNodeColorToSoftSet();
		if (!isActivated)
			JGui::PopColor();
	}
#pragma endregion

#pragma region Style
	JVector2<float> JGui::GetWindowPadding()noexcept
	{
		return Private::Adaptee()->GetWindowPadding();
	}
	float JGui::GetWindowBorderSize()noexcept
	{
		return Private::Adaptee()->GetWindowBorderSize();
	}
	float JGui::GetWindowRounding()noexcept
	{
		return Private::Adaptee()->GetWindowRounding();
	}
	JVector2<float> JGui::GetFramePadding()noexcept
	{
		return Private::Adaptee()->GetFramePadding();
	}
	float JGui::GetFrameBorderSize()noexcept
	{
		return Private::Adaptee()->GetFrameBorderSize();
	}
	JVector2<float> JGui::GetItemSpacing()noexcept
	{
		return Private::Adaptee()->GetItemSpacing();
	}
	JVector2<float> JGui::GetItemInnerSpacing()noexcept
	{
		return Private::Adaptee()->GetItemInnerSpacing();
	}
	float JGui::GetScrollBarSize()noexcept
	{
		return Private::Adaptee()->GetScrollBarSize();
	}
	float JGui::GetStyleValueF(const J_GUI_STYLE style)
	{
		return Private::Adaptee()->GetStyleValueF(style);
	}
	JVector2<float> JGui::GetStyleValueV2(const J_GUI_STYLE style)
	{
		return Private::Adaptee()->GetStyleValueV2(style);
	}
	void JGui::SetWindowPadding(const JVector2<float>& padding)noexcept
	{
		Private::Adaptee()->SetWindowPadding(padding);
	}
	void JGui::SetWindowBorderSize(const float size)noexcept
	{
		Private::Adaptee()->SetWindowBorderSize(size);
	}
	void JGui::SetWindowRounding(const float factor)noexcept
	{
		Private::Adaptee()->SetWindowRounding(factor);
	}
	void JGui::SetFramePadding(const JVector2<float>& padding)noexcept
	{
		Private::Adaptee()->SetFramePadding(padding);
	}
	void JGui::SetFrameBorderSize(const float size)noexcept
	{
		Private::Adaptee()->SetFrameBorderSize(size);
	}
	void JGui::SetItemSpacing(const JVector2<float>& spacing)noexcept
	{
		Private::Adaptee()->SetItemSpacing(spacing);
	}
	void JGui::SetItemInnerSpacing(const JVector2<float>& spacing)noexcept
	{
		Private::Adaptee()->SetItemInnerSpacing(spacing);
	}
	void JGui::SetScrollBarSize(const float size)noexcept
	{
		Private::Adaptee()->SetScrollBarSize(size);
	}
	void JGui::PushStyle(const J_GUI_STYLE style, const float value)
	{
		Private::Adaptee()->PushStyle(style, value);
	}
	void JGui::PushStyle(const J_GUI_STYLE style, const JVector2<float>& value)
	{
		Private::Adaptee()->PushStyle(style, value);
	}
	void JGui::PopStyle(const uint count)
	{
		Private::Adaptee()->PopStyle(count);
	}
#pragma endregion

#pragma region IO
	JVector2<float> JGui::GetAlphabetSize()noexcept
	{
		return Private::Adaptee()->GetAlphabetSize();
	}
	float JGui::GetFontSize()noexcept
	{
		return Private::Adaptee()->GetFontSize();
	}
	JVector2<float> JGui::CalTextSize(const std::string& str)noexcept
	{
		return Private::Adaptee()->CalTextSize(str);
	}
	uint JGui::GetTextBuffRange()noexcept
	{
		return Private::Impl()->option.textBufRange;
	}
	float JGui::GetGlobalFontScale()noexcept
	{
		return Private::Adaptee()->GetGlobalFontScale();
	}
	float JGui::GetCurrentWindowFontScale()noexcept
	{
		return Private::Adaptee()->GetCurrentWindowFontScale();
	}
	void JGui::SetGlobalFontScale(const float scale)noexcept
	{
		Private::Adaptee()->SetGlobalFontScale(scale);
	}
	void JGui::SetCurrentWindowFontScale(const float scale)noexcept
	{
		Private::Adaptee()->SetCurrentWindowFontScale(scale);
	}
	void JGui::SetFont(const J_GUI_FONT_TYPE fontType)
	{
		Private::Adaptee()->SetFont(fontType);
	}
	void JGui::PushFont()noexcept
	{
		Private::Adaptee()->PushFont();
	}
	void JGui::PopFont()noexcept
	{
		Private::Adaptee()->PopFont();
	}
	uint JGui::GetMouseClickedCount(const Core::J_MOUSE_BUTTON btn)noexcept
	{
		return Private::Adaptee()->GetMouseClickedCount(btn);
	}
	float JGui::GetMouseWheel()noexcept
	{
		return Private::Adaptee()->GetMouseWheel();
	}
	JVector2<float> JGui::GetMousePos() noexcept
	{
		return Private::Adaptee()->GetMousePos();
	}
	JVector2<float> JGui::GetMouseDragDelta() noexcept
	{
		return Private::Adaptee()->GetMouseDragDelta();
	}
	JVector2<float> JGui::GetCursorPos() noexcept
	{
		return Private::Adaptee()->GetCursorPos();
	}
	JVector2<float> JGui::GetCursorScreenPos() noexcept
	{
		return Private::Adaptee()->GetCursorScreenPos();
	}
	float JGui::GetCursorPosX()noexcept
	{
		return Private::Adaptee()->GetCursorPos().x;
	}
	float JGui::GetCursorPosY()noexcept
	{
		return Private::Adaptee()->GetCursorPos().y;
	}
	void JGui::SetCursorPos(const JVector2<float>& pos)noexcept
	{
		Private::Adaptee()->SetCursorPos(pos);
	}
	void JGui::SetCursorPosX(const float x)noexcept
	{
		auto cusor = Private::Adaptee()->GetCursorPos();
		cusor.x = x;
		Private::Adaptee()->SetCursorPos(cusor);
	}
	void JGui::SetCursorPosY(const float y)noexcept
	{
		auto cusor = Private::Adaptee()->GetCursorPos();
		cusor.y = y;
		Private::Adaptee()->SetCursorPos(cusor);
	}
	void JGui::SetCursorScreenPos(const JVector2<uint>& pos)noexcept
	{
		Private::Adaptee()->SetCursorScreenPos(pos);
	}
	bool JGui::IsMouseClicked(const Core::J_MOUSE_BUTTON btn)noexcept
	{
		return Private::Adaptee()->IsMouseClicked(btn);
	}
	bool JGui::IsLastMouseClicked(const Core::J_MOUSE_BUTTON btn)noexcept
	{
		return Private::Impl()->IsLastMouseClicked(btn);
	}
	bool JGui::IsMouseReleased(const Core::J_MOUSE_BUTTON btn)noexcept
	{
		return Private::Adaptee()->IsMouseReleased(btn);
	}
	bool JGui::IsMouseDown(const Core::J_MOUSE_BUTTON btn)noexcept
	{
		return Private::Adaptee()->IsMouseDown(btn);
	}
	bool JGui::IsMouseDragging(const Core::J_MOUSE_BUTTON btn)noexcept
	{
		return Private::Adaptee()->IsMouseDragging(btn);
	}
	bool JGui::AnyMouseDown(const bool containMiddle, const bool containRight, const bool containLeft)noexcept
	{
		bool isDown = false;
		if (!isDown && containLeft)
			isDown |= Private::Adaptee()->IsMouseDown(Core::J_MOUSE_BUTTON::LEFT);
		if (!isDown && containRight)
			isDown |= Private::Adaptee()->IsMouseDown(Core::J_MOUSE_BUTTON::RIGHT);
		if (!isDown && containMiddle)
			isDown |= Private::Adaptee()->IsMouseDown(Core::J_MOUSE_BUTTON::MIDDLE);
		return isDown;
	}
	bool JGui::AnyMouseClicked(const bool containMiddle, const bool containRight, const bool containLeft)noexcept
	{
		bool isClicked = false;
		if (!isClicked && containLeft)
			isClicked |= Private::Adaptee()->IsMouseClicked(Core::J_MOUSE_BUTTON::LEFT);
		if (!isClicked && containRight)
			isClicked |= Private::Adaptee()->IsMouseClicked(Core::J_MOUSE_BUTTON::RIGHT);
		if (!isClicked && containMiddle)
			isClicked |= Private::Adaptee()->IsMouseClicked(Core::J_MOUSE_BUTTON::MIDDLE);
		return isClicked;
	} 
	bool JGui::IsMouseInRect(const JVector2<float>& position, const JVector2<float>& size)noexcept
	{
		return Private::Adaptee()->IsMouseInRect(position, position + size);
	}
	bool JGui::IsMouseInCurrentWindow()noexcept
	{
		return IsMouseInRect(GetWindowPos(), GetWindowSize());
	}
	bool JGui::IsMouseInLastItem()noexcept
	{
		return IsMouseInRect(GetLastItemRectPos(), GetLastItemRectSize());
		//return IsMouseInRect(GetLastItemRectPos() - GetWindowScrollPos(), GetLastItemRectSize());
	}
	bool JGui::IsMouseInRectMM(const JVector2<float>& min, const JVector2<float>& max)noexcept
	{
		return Private::Adaptee()->IsMouseInRect(min, max);
	}
	bool JGui::IsMouseHoveringRect(const JVector2<float>& min, const JVector2<float>& max, const bool clip)noexcept
	{
		return Private::Adaptee()->IsMouseHoveringRect(min, max, clip);
	}
	bool JGui::IsMouseInLine(JVector2<float> st, JVector2<float> ed, const float thickness)noexcept
	{
		return Private::Adaptee()->IsMouseInLine(st, ed, thickness);
	}
	bool JGui::CanFocusByMouseRightClick()noexcept
	{
		return Private::Adaptee()->CanFocusByMouseRightClick();
	}
	bool JGui::IsKeyPressed(const Core::J_KEYCODE key)noexcept
	{
		return Private::Adaptee()->IsKeyPressed(key);
	}
	bool JGui::IsKeyReleased(const Core::J_KEYCODE key)noexcept
	{
		return Private::Adaptee()->IsKeyReleased(key);
	}
	bool JGui::IsKeyDown(const Core::J_KEYCODE key)noexcept
	{
		return Private::Adaptee()->IsKeyDown(key);
	}
	bool JGui::BeginDragDropSource(J_GUI_DRAG_DROP_FLAG_ flag)
	{
		return Private::Adaptee()->BeginDragDropSource(flag);
	}
	bool JGui::SetDragDropPayload(const std::string& typeName, JDragDropData* draggingHint, J_GUI_CONDIITON cond)
	{
		return Private::Adaptee()->SetDragDropPayload(typeName, draggingHint, cond);
	}
	void JGui::EndDragDropSource()
	{
		Private::Adaptee()->EndDragDropSource();
	}
	bool JGui::BeginDragDropTarget()
	{
		return Private::Adaptee()->BeginDragDropTarget();
	}
	JDragDropData* JGui::TryGetTypeHintDragDropPayload(const std::string& typeName, J_GUI_DRAG_DROP_FLAG_ flag)
	{
		return Private::Adaptee()->TryGetTypeHintDragDropPayload(typeName, flag);
	}
	void JGui::EndDragDropTarget()
	{
		Private::Adaptee()->EndDragDropTarget();
	}
	bool JGui::IsDragDropActivated()
	{
		return Private::Adaptee()->IsDragDropActivated();
	}
#pragma endregion
#pragma region JGui
	bool JGui::BeginWindow(const std::string& name, bool* p_open, J_GUI_WINDOW_FLAG_ flags)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::WINDOW);
		const bool isOpend = Private::Adaptee()->BeginWindow(name, p_open, (J_GUI_WINDOW_FLAG)flags);
		if (isOpend)
		{
			JGuiWindowInfo info;
			JGui::GetCurrentWindowInfo(info);
			Private::Impl()->AddDisplayedWindowInfo(std::move(info));
		}
		return isOpend;
	}
	void JGui::EndWindow()
	{
		Private::Adaptee()->EndWindow();
	}
	bool JGui::BeginChildWindow(const std::string& name, const JVector2<float>& windowSize, bool border, J_GUI_WINDOW_FLAG_ flags)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::CHILD_WINDW);
		return Private::Adaptee()->BeginChildWindow(name, windowSize, border, (J_GUI_WINDOW_FLAG)flags);
	}
	void JGui::EndChildWindow()
	{
		Private::Adaptee()->EndChildWindow();
	}
	void JGui::OpenPopup(const std::string& name, J_GUI_POPUP_FLAG_ flags)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::POPUP);
		Private::Adaptee()->OpenPopup(name, (J_GUI_POPUP_FLAG)flags);
	}
	void JGui::CloseCurrentPopup()
	{
		Private::Adaptee()->CloseCurrentPopup();
	}
	bool JGui::BeginPopup(const std::string& name, J_GUI_WINDOW_FLAG_ flags)
	{
		return Private::Adaptee()->BeginPopup(name, (J_GUI_WINDOW_FLAG)flags);
	}
	void JGui::EndPopup()
	{
		Private::Adaptee()->EndPopup();
	}
	void JGui::BeginGroup()
	{
		Private::Adaptee()->BeginGroup();
	}
	void JGui::EndGroup()
	{
		Private::Adaptee()->EndGroup();
	}
	void JGui::Text(const std::wstring& text)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::TEXT);
		Private::Adaptee()->Text(JCUtil::WstrToU8Str(text));
	}
	void JGui::Text(const std::string& text)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::TEXT);
		Private::Adaptee()->Text(text);
	}
	void JGui::Text(const char* text)
	{
		Text(std::string(text));
	}
	void JGui::Text(const wchar_t* text)
	{
		Text(std::wstring(text));
	}
	void JGui::Text(const std::string& text, const float fontScale)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::TEXT);

		const float preFontScale = Private::Adaptee()->GetCurrentWindowFontScale();
		Private::Adaptee()->SetCurrentWindowFontScale(fontScale);
		Private::Adaptee()->Text(text);
		Private::Adaptee()->SetCurrentWindowFontScale(preFontScale);
	}
	void JGui::Text(const std::wstring& text, const float fontScale)
	{
		Text(JCUtil::WstrToU8Str(text), fontScale);
	}
	bool JGui::CheckBox(const std::string& name, bool& v)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::CHECKBOX);
		return Private::Adaptee()->CheckBox(name, v);
	}
	bool JGui::Button(const std::string& name, const JVector2<float>& jVec2, J_GUI_BUTTON_FLAG_ flag)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::BUTTON);
		return Private::Adaptee()->Button(name, jVec2, (J_GUI_BUTTON_FLAG)flag);
	}
	bool JGui::ArrowButton(const std::string& name, const JVector2<float>& jVec2, const float arrowScale, J_GUI_BUTTON_FLAG_ flag, J_GUI_CARDINAL_DIR dir)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::BUTTON);
		return Private::Adaptee()->ArrowButton(name, jVec2, arrowScale, (J_GUI_BUTTON_FLAG)flag, dir);
	}
	bool JGui::IsTreeNodeOpend(const std::string& name, J_GUI_TREE_NODE_FLAG_ flags)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::BUTTON);
		return Private::Adaptee()->IsTreeNodeOpend(name, flags);
	}
	bool JGui::TreeNodeEx(const std::string& name, J_GUI_TREE_NODE_FLAG_ flags)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::TREE_NODE);
		return Private::Adaptee()->TreeNodeEx(name, (J_GUI_TREE_NODE_FLAG)flags);
	}
	void JGui::TreePop()
	{
		Private::Adaptee()->TreePop();
	}
	bool JGui::Selectable(const std::string& name, bool* pSelected, J_GUI_SELECTABLE_FLAG_ flags, const JVector2F sizeArg)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::SELECTALBE);
		return Private::Adaptee()->Selectable(name, pSelected, (J_GUI_SELECTABLE_FLAG)flags, sizeArg);
	}
	bool JGui::Selectable(const std::string& name, bool selected, J_GUI_SELECTABLE_FLAG_ flags, const JVector2F sizeArg)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::SELECTALBE);
		return Private::Adaptee()->Selectable(name, selected, (J_GUI_SELECTABLE_FLAG)flags, sizeArg);
	}
	bool JGui::InputText(const std::string& name, std::string& buff, J_GUI_INPUT_TEXT_FLAG_ flags)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::INPUT);
		return Private::Adaptee()->InputText(name, buff, (J_GUI_INPUT_TEXT_FLAG)flags);
	}
	bool JGui::InputText(const std::string& name, std::string& buff, const size_t size, J_GUI_INPUT_TEXT_FLAG_ flags)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::INPUT);
		return Private::Adaptee()->InputText(name, buff, size, (J_GUI_INPUT_TEXT_FLAG)flags);
	}
	bool JGui::InputText(const std::string& name, std::string& buff, std::string& result, const std::string& hint, J_GUI_INPUT_TEXT_FLAG_ flags)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::INPUT);
		return Private::Adaptee()->InputText(name, buff, result, hint, (J_GUI_INPUT_TEXT_FLAG)flags);
	}
	bool JGui::InputMultilineText(const std::string& name, std::string& buff, std::string& result, const JVector2<float>& size, J_GUI_INPUT_TEXT_FLAG_ flags)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::INPUT);
		return Private::Adaptee()->InputMultilineText(name, buff, result, size, (J_GUI_INPUT_TEXT_FLAG)flags);
	}
	bool JGui::InputInt(const std::string& name, int* value, J_GUI_INPUT_TEXT_FLAG_ flags, int step, uint inputWidthRate)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::INPUT);
		SetNextItemWidth(GetAlphabetSize().x * (std::numeric_limits<float>::digits * 0.375f * inputWidthRate));
		return Private::Adaptee()->InputInt(name, value, (J_GUI_INPUT_TEXT_FLAG)flags, step);
	}
	bool JGui::InputInt(const std::string& name, uint* value, J_GUI_INPUT_TEXT_FLAG_ flags, uint step, uint inputWidthRate)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::INPUT);
		SetNextItemWidth(GetAlphabetSize().x * (std::numeric_limits<float>::digits * 0.375f * inputWidthRate));
		return Private::Adaptee()->InputInt(name, value, (J_GUI_INPUT_TEXT_FLAG)flags, step);
	}
	bool JGui::InputIntClamp(const std::string& name, int* value, const int minV, const int maxV, J_GUI_INPUT_TEXT_FLAG_ flags, int step, uint inputWidthRate)
	{
		bool isInput = InputInt(name, value, (J_GUI_INPUT_TEXT_FLAG)flags, step, inputWidthRate);
		if (isInput)
			*value = std::clamp(*value, minV, maxV);
		return isInput;
	}
	bool JGui::InputIntClamp(const std::string& name, uint* value, const uint minV, const uint maxV, J_GUI_INPUT_TEXT_FLAG_ flags, uint step, uint inputWidthRate)
	{
		bool isInput = InputInt(name, value, (J_GUI_INPUT_TEXT_FLAG)flags, step, inputWidthRate);
		if (isInput)
			*value = std::clamp(*value, minV, maxV);
		return isInput;
	}
	bool JGui::InputIntClamp(const std::string& name, Core::JRestrictedRangeVar<int>& value, J_GUI_INPUT_TEXT_FLAG_ flags , int step, uint inputWidthRate)
	{
		bool isInput = InputInt(name, value.GetPtr(), (J_GUI_INPUT_TEXT_FLAG)flags, step, inputWidthRate);
		if (isInput)
			value.Set(value.Get());
		return isInput; 
	}
	bool JGui::InputIntClamp(const std::string& name, Core::JRestrictedRangeVar<uint>& value, J_GUI_INPUT_TEXT_FLAG_ flags, uint step, uint inputWidthRate)
	{
		bool isInput = InputInt(name, value.GetPtr(), (J_GUI_INPUT_TEXT_FLAG)flags, step, inputWidthRate);
		if (isInput)
			value.Set(value.Get());
		return isInput;
	}
	bool JGui::InputFloat(const std::string& name, float* value, J_GUI_INPUT_TEXT_FLAG_ flags, const float formatDigit, float step, uint inputWidthRate)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::INPUT);
		SetNextItemWidth(GetAlphabetSize().x * (std::numeric_limits<float>::digits * 0.375f * inputWidthRate));
		return Private::Adaptee()->InputFloat(name, value, (J_GUI_INPUT_TEXT_FLAG)flags, formatDigit, step);
	}
	bool JGui::InputFloatClamp(const std::string& name, float* value, const float minV, const float maxV, J_GUI_INPUT_TEXT_FLAG_ flags, const float formatDigit, float step, uint inputWidthRate)
	{  
		bool isInput = InputFloat(name, value, (J_GUI_INPUT_TEXT_FLAG)flags, formatDigit, step, inputWidthRate);
		if (isInput)
			*value = std::clamp(*value, minV, maxV);
		return isInput;
	}
	bool JGui::InputFloatClamp(const std::string& name, Core::JRestrictedRangeVar<float>& value, J_GUI_INPUT_TEXT_FLAG_ flags, const float formatDigit, float step, uint inputWidthRate)
	{
		bool isInput = InputFloat(name, value.GetPtr(), (J_GUI_INPUT_TEXT_FLAG)flags, formatDigit, step, inputWidthRate);
		if (isInput)
			value.Set(value.Get());
		return isInput;	 
	}
	bool JGui::SliderInt(const std::string& name, int* value, int vMin, int vMax, J_GUI_SLIDER_FLAG_ flags)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::SLIDER);
		return Private::Adaptee()->SliderInt(name, value, vMin, vMax, (J_GUI_SLIDER_FLAG)flags);
	}
	bool JGui::SliderInt(const std::string& name, uint* value, uint vMin, uint vMax, J_GUI_SLIDER_FLAG_ flags)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::SLIDER);
		return Private::Adaptee()->SliderInt(name, value, vMin, vMax, (J_GUI_SLIDER_FLAG)flags);
	}
	bool JGui::SliderFloat(const std::string& name, float* value, float vMin, float vMax, const float formatDigit, J_GUI_SLIDER_FLAG_ flags)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::SLIDER);
		return Private::Adaptee()->SliderFloat(name, value, vMin, vMax, formatDigit, (J_GUI_SLIDER_FLAG)flags);
	}
	bool JGui::VSliderInt(const std::string& name, JVector2<float> size, int* value, int vMin, int vMax, J_GUI_SLIDER_FLAG_ flags)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::SLIDER);
		return Private::Adaptee()->VSliderInt(name, size, value, vMin, vMax, (J_GUI_SLIDER_FLAG)flags);
	}
	bool JGui::VSliderInt(const std::string& name, JVector2<float> size, uint* value, uint vMin, uint vMax, J_GUI_SLIDER_FLAG_ flags)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::SLIDER);
		return Private::Adaptee()->VSliderInt(name, size, value, vMin, vMax, (J_GUI_SLIDER_FLAG)flags);
	}
	bool JGui::VSliderFloat(const std::string& name, JVector2<float> size, float* value, float vMin, float vMax, const float formatDigit, J_GUI_SLIDER_FLAG_ flags)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::SLIDER);
		return Private::Adaptee()->VSliderFloat(name, size, value, vMin, vMax, formatDigit, (J_GUI_SLIDER_FLAG)flags);
	}
	bool JGui::BeginTabBar(const std::string& name, J_GUI_TAB_BAR_FLAG_ flags)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::TAB_BAR);
		return Private::Adaptee()->BeginTabBar(name, (J_GUI_TAB_BAR_FLAG)flags);
	}
	void JGui::EndTabBar()
	{
		Private::Adaptee()->EndTabBar();
	}
	bool JGui::BeginTabItem(const std::string& name, bool* p_open, J_GUI_TAB_ITEM_FLAG_ flags)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::TAB_ITEM);
		return Private::Adaptee()->BeginTabItem(name, p_open, (J_GUI_TAB_ITEM_FLAG)flags);
	}
	void JGui::EndTabItem()
	{
		Private::Adaptee()->EndTabItem();
	}
	bool JGui::TabItemButton(const std::string& name, J_GUI_TAB_ITEM_FLAG_ flags)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::TAB_ITEM);
		return Private::Adaptee()->TabItemButton(name, (J_GUI_TAB_ITEM_FLAG)flags);
	}
	bool JGui::BeginTable(const std::string& name, const int columnCount, J_GUI_TABLE_FLAG_ flags, const JVector2<float> outerSize, const float innerWidth)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::TABLE);
		return Private::Adaptee()->BeginTable(name, columnCount, (J_GUI_TABLE_FLAG)flags, outerSize, innerWidth);
	}
	void JGui::EndTable()
	{
		Private::Adaptee()->EndTable();
	}
	void JGui::TableSetupColumn(const std::string& name, J_GUI_TABLE_COLUMN_FLAG_ flags, float initWeight)
	{
		Private::Adaptee()->TableSetupColumn(name, (J_GUI_TABLE_COLUMN_FLAG)flags, initWeight);
	}
	void JGui::TableHeadersRow()
	{
		Private::Adaptee()->TableHeadersRow();
	}
	void JGui::TableNextRow()
	{
		Private::Adaptee()->TableNextRow();
	}
	bool JGui::TableSetColumnIndex(const int index)
	{
		return Private::Adaptee()->TableSetColumnIndex(index);
	}
	bool JGui::BeginMainMenuBar()
	{
		return Private::Adaptee()->BeginMainMenuBar();
	}
	void JGui::EndMainMenuBar()
	{
		Private::Adaptee()->EndMainMenuBar();
	}
	bool JGui::BeginMenuBar()
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::MENU_BAR);
		return Private::Adaptee()->BeginMenuBar();
	}
	void JGui::EndMenuBar()
	{
		Private::Adaptee()->EndMenuBar();
	}
	bool JGui::BeginMenu(const std::string& name, bool enable)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::MENU);
		return Private::Adaptee()->BeginMenu(name, enable);
	}
	void JGui::EndMenu()
	{
		Private::Adaptee()->EndMenu();
	}
	bool JGui::MenuItem(const std::string& name, bool selected, bool enabled)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::MENU_ITEM);
		return Private::Adaptee()->MenuItem(name, selected, enabled);
	}
	bool JGui::MenuItem(const std::string& name, const std::string& shortcut, bool selected, bool enabled)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::MENU_ITEM);
		return Private::Adaptee()->MenuItem(name, shortcut, selected, enabled);
	}
	bool JGui::BeginCombo(const std::string& name, const std::string& preview, J_GUI_COMBO_FLAG_ flags)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::COMBO);
		return Private::Adaptee()->BeginCombo(name, preview, (J_GUI_COMBO_FLAG)flags);
	}
	bool JGui::BeginComboEx(const std::string& name,
		const std::string& preview,
		J_GUI_COMBO_FLAG_ flags,
		J_GUI_CARDINAL_DIR initDir,
		J_GUI_CARDINAL_DIR activateDir)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::COMBO);
		return Private::Adaptee()->BeginComboEx(name, preview, (J_GUI_COMBO_FLAG)flags, initDir, activateDir);
	}
	void JGui::EndCombo()
	{
		Private::Adaptee()->EndCombo();
	}
	bool JGui::BeginListBox(const std::string& name, const JVector2<float> size)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::LIST);
		return Private::Adaptee()->BeginListBox(name, size);
	}
	void JGui::EndListBox()
	{
		Private::Adaptee()->EndListBox();
	}
	bool JGui::ColorPicker(const std::string& name, JVector3<float>& color, J_GUI_COLOR_EDIT_FALG_ flags)
	{
		return Private::Adaptee()->ColorPicker(name, color, flags);
	}
	bool JGui::ColorPicker(const std::string& name, JVector4<float>& color, J_GUI_COLOR_EDIT_FALG_ flags)
	{
		return Private::Adaptee()->ColorPicker(name, color, flags);
	}
	void JGui::Tooltip(const std::string& message, const float fontScale)noexcept
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::WINDOW);
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::TEXT);
		const float preFontScale = JGui::GetCurrentWindowFontScale();
		JGui::SetCurrentWindowFontScale(fontScale);
		Private::Adaptee()->Tooltip(message);
		JGui::SetCurrentWindowFontScale(preFontScale);
	} 
	void JGui::Tooltip(const float value, const int range, const float fontScale)noexcept
	{
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(range);
		oss << value;
		Tooltip(oss.str(), fontScale);
	}
	void JGui::InvalidImage(const JVector2<float>& size,
		const JVector2<float>& uv0,
		const JVector2<float>& uv1,
		const JVector4<float>& tintCol,
		const JVector4<float>& borderCol)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::IMAGE);
		Private::Adaptee()->InvalidImage(size, uv0, uv1, tintCol, borderCol);
	}
	void JGui::Image(JGuiImageInfo info,
		const JVector2<float>& size,
		const JVector2<float>& uv0,
		const JVector2<float>& uv1,
		const JVector4<float>& tintCol,
		const JVector4<float>& borderCol)
	{
		if (!info.IsValid())
			InvalidImage(size);
		else if (info.useFirstHandle)
		{
			Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::IMAGE);
			Private::Adaptee()->Image(info, size, uv0, uv1, tintCol, borderCol);
			if (info.extraPerImagePtr != nullptr)
				info.extraPerImagePtr(0);
		}
		else if (info.displayAllType)
		{ 
			uint imageCount = 0;
			auto gRInterface = info.gUserAccess->GraphicResourceUserInterface();
			const uint dataCount = info.gUserAccess->GraphicResourceUserInterface().GetDataCount(info.rType);
			for (uint i = 0; i < dataCount; ++i)
			{
				info.dataIndex = i;
				const uint viewCount = gRInterface.GetViewCount(info.rType, info.bType, i);
				for (uint j = 0; j < viewCount; ++j)
				{ 
					info.bIndex = j;
					Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::IMAGE);
					Private::Adaptee()->Image(info, size, uv0, uv1, tintCol, borderCol);
					if (info.extraPerImagePtr != nullptr)
						info.extraPerImagePtr(imageCount);
					++imageCount;
				}
			}
		}
		else
		{
			auto gRInterface = info.gUserAccess->GraphicResourceUserInterface();
			const uint viewCount = gRInterface.GetViewCount(info.rType, info.bType, info.dataIndex);
			for (uint i = 0; i < viewCount; ++i)
			{
				info.bIndex = i;
				Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::IMAGE);
				Private::Adaptee()->Image(info, size, uv0, uv1, tintCol, borderCol);
				if (info.extraPerImagePtr != nullptr)
					info.extraPerImagePtr(i);
			}
		}
	}
	bool JGui::ImageButton(const std::string name,
		JGuiImageInfo info,
		const JVector2<float>& size,
		const JVector2<float>& uv0,
		const JVector2<float>& uv1,
		float framePadding,
		const JVector4<float>& bgCol,
		const JVector4<float>& tintCol)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::IMAGE);
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::BUTTON);

		info.displayAllType = false;
		//if invalue set missing texture
		if (!info.IsValid())
			info.gUserAccess = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::MISSING).Get();

		return Private::Adaptee()->ImageButton(name,
			info,
			size,
			uv0, uv1,
			framePadding,
			bgCol, tintCol);
	}
	void JGui::AddInvalidImage(const JVector2<float>& pMin,
		const JVector2<float>& pMax,
		const bool isFront, 
		const JVector4<float>& color,
		const JVector2<float>& uvMin,
		const JVector2<float>& uvMax)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::IMAGE);

		auto missing = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::MISSING).Get();
		JGuiImageInfo info(missing);
		Private::Adaptee()->AddImage(info, pMin, pMax, isFront, color, uvMin, uvMax);
	}
	void JGui::AddRoundedInvalidImage(const JVector2<float>& pMin,
		const JVector2<float>& pMax,
		const float rounding,
		J_GUI_DRAW_FLAG_ flag,
		const bool isFront,
		const JVector4<float>& color,
		const JVector2<float>& uvMin,
		const JVector2<float>& uvMax)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::IMAGE);

		auto missing = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::MISSING).Get();
		JGuiImageInfo info(missing);
		Private::Adaptee()->AddRoundedImage(info, pMin, pMax, rounding, flag, isFront, color, uvMin, uvMax);
	}
	void JGui::AddImage(JGuiImageInfo info,
		const JVector2<float>& pMin,
		const JVector2<float>& pMax,
		const bool isFront,
		const JVector4<float>& color,
		const JVector2<float>& uvMin,
		const JVector2<float>& uvMax)
	{
		if (!info.IsValid())
			AddInvalidImage(pMin, pMax, isFront, color, uvMin, uvMax);
		else if (info.useFirstHandle)
		{
			Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::IMAGE);
			Private::Adaptee()->AddImage(info, pMin, pMax, isFront, color, uvMin, uvMax);
			if (info.extraPerImagePtr != nullptr)
				info.extraPerImagePtr(0);
		}
		else if (info.displayAllType)
		{
			uint imageCount = 0;
			auto gRInterface = info.gUserAccess->GraphicResourceUserInterface();
			const uint dataCount = info.gUserAccess->GraphicResourceUserInterface().GetDataCount(info.rType);
			for (uint i = 0; i < dataCount; ++i)
			{
				info.dataIndex = i;
				const uint viewCount = gRInterface.GetViewCount(info.rType, info.bType, i);
				for (uint j = 0; j < viewCount; ++j)
				{
					info.bIndex = j;
					Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::IMAGE);
					Private::Adaptee()->AddImage(info, pMin, pMax, isFront, color, uvMin, uvMax);
					if (info.extraPerImagePtr != nullptr)
						info.extraPerImagePtr(imageCount);
					++imageCount;
				}
			}

		}
		else
		{
			auto gRInterface = info.gUserAccess->GraphicResourceUserInterface();
			const uint viewCount = gRInterface.GetViewCount(info.rType, info.bType, info.dataIndex);
			for (uint i = 0; i < viewCount; ++i)
			{
				info.bIndex = i;
				Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::IMAGE);
				Private::Adaptee()->AddImage(info, pMin, pMax, isFront, color, uvMin, uvMax);
				if (info.extraPerImagePtr != nullptr)
					info.extraPerImagePtr(0);
			}
		}
	}
	void JGui::AddRoundedImage(JGuiImageInfo info,
		const JVector2<float>& pMin,
		const JVector2<float>& pMax,
		const bool isFront,
		const float rounding,
		J_GUI_DRAW_FLAG_ flag,
		const JVector4<float>& color,
		const JVector2<float>& uvMin,
		const JVector2<float>& uvMax)
	{
		if (!info.IsValid())
			AddRoundedInvalidImage(pMin, pMax, isFront, rounding, flag, color, uvMin, uvMax);
		else if (info.useFirstHandle)
		{
			Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::IMAGE);
			Private::Adaptee()->AddRoundedImage(info, pMin, pMax, isFront, rounding, flag, color, uvMin, uvMax);
			if (info.extraPerImagePtr != nullptr)
				info.extraPerImagePtr(0);
		}
		else if (info.displayAllType)
		{
			uint imageCount = 0;
			auto gRInterface = info.gUserAccess->GraphicResourceUserInterface();
			const uint dataCount = info.gUserAccess->GraphicResourceUserInterface().GetDataCount(info.rType);
			for (uint i = 0; i < dataCount; ++i)
			{
				info.dataIndex = i;
				const uint viewCount = gRInterface.GetViewCount(info.rType, info.bType, i);
				for (uint j = 0; j < viewCount; ++j)
				{
					info.bIndex = j;
					Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::IMAGE);
					Private::Adaptee()->AddRoundedImage(info, pMin, pMax, isFront, rounding, flag, color, uvMin, uvMax);
					if (info.extraPerImagePtr != nullptr)
						info.extraPerImagePtr(imageCount);
					++imageCount;
				}
			}

		}
		else
		{
			auto gRInterface = info.gUserAccess->GraphicResourceUserInterface();
			const uint viewCount = gRInterface.GetViewCount(info.rType, info.bType, info.dataIndex);
			for (uint i = 0; i < viewCount; ++i)
			{
				info.bIndex = i;
				Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::IMAGE);
				Private::Adaptee()->AddRoundedImage(info, pMin, pMax, isFront, rounding, flag, color, uvMin, uvMax);
				if (info.extraPerImagePtr != nullptr)
					info.extraPerImagePtr(0);
			}
		}
	}
	bool JGui::ImageSelectable(const std::string name,
		JGuiImageInfo info, 
		const JVector2<float>& size,
		const bool useRestoreCursorPos)
	{
		bool temp = false;
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::SELECTALBE);
		return Private::Adaptee()->ImageSelectable(name, info, temp, false, size, useRestoreCursorPos);
	}
	bool JGui::ImageSelectable(const std::string name,
		JGuiImageInfo info,
		bool& pressed,
		bool changeValueIfPreesd,
		const JVector2<float>& size,
		const bool useRestoreCursorPos)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::SELECTALBE);
		return Private::Adaptee()->ImageSelectable(name, info, pressed, changeValueIfPreesd, size, useRestoreCursorPos);
	}
	bool JGui::Switch(const std::string& name,
		bool& pressed,
		bool changeValueIfPreesd,
		const JVector2<float>& size)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::SWITCH);
		return Private::Adaptee()->Switch(name, pressed, changeValueIfPreesd, size);
	}
	bool JGui::ImageSwitch(const std::string name,
		JGuiImageInfo info,
		bool& pressed,
		bool changeValueIfPreesd,
		const JVector2<float>& size,
		const JVector4<float>& bgColor, 
		const JVector4<float>& frameColor,
		const float frameThickness)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::SWITCH);
		return Private::Adaptee()->ImageSwitch(name, info, pressed, changeValueIfPreesd, size, bgColor, frameColor, frameThickness);
	}
	bool JGui::ImageButton(const std::string name,
		JGuiImageInfo info,
		const JVector2<float>& size,
		const JVector4<float>& bgColor, 
		const JVector4<float>& frameColor,
		const float frameThickness)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::BUTTON);
		return Private::Adaptee()->ImageButton(name, info, size, bgColor, frameColor, frameThickness);
	}
	bool JGui::MaximizeButton(const bool isLocatedCloseBtnLeftSide)
	{
		return Private::Adaptee()->MaximizeButton(isLocatedCloseBtnLeftSide);
	}
	bool JGui::MinimizeButton(const bool isLocatedCloseBtnLeftSide, const bool isLocatedMaximizeBtnLeftSize)
	{
		return Private::Adaptee()->MinimizeButton(isLocatedCloseBtnLeftSide, isLocatedMaximizeBtnLeftSize);
	}
	bool JGui::PreviousSizeButton(const bool isLocatedCloseBtnLeftSide, const bool isLocatedMaximizeBtnLeftSize)
	{
		return Private::Adaptee()->PreviousSizeButton(isLocatedCloseBtnLeftSide, isLocatedMaximizeBtnLeftSize);
	}
	void JGui::DrawRectFilledMultiColor(const JVector2<float>& pos,
		const JVector2<float>& size,
		const JVector4<float>& color,
		const JVector4<float>& colorDelta,
		const bool useRestoreCursorPos,
		const bool useFrameRounding)noexcept
	{
		const JVector4F upLeft = color + colorDelta * 0.5f;
		const JVector4F upRight = color + colorDelta;
		const JVector4F downLeft = color + colorDelta * -1.0f;
		const JVector4F downRight = color + colorDelta * -0.5f;
		DrawRectFilledMultiColor(pos, size, upLeft, upRight, downLeft, downRight, useRestoreCursorPos, useFrameRounding);
	}
	void JGui::DrawRectFilledMultiColor(const JVector2<float>& pos,
		const JVector2<float>& size,
		const JVector4<float>& color,
		const JVector4<float>& colorDelta,
		const bool useRestoreCursorPos,
		const J_GUI_ORDINAL_DIR_FLAG_ addedDeltaDir,
		const bool useFrameRounding)noexcept
	{
		const J_GUI_ORDINAL_DIR_FLAG dirFlag = (J_GUI_ORDINAL_DIR_FLAG)addedDeltaDir;
		const JVector4F addCol = color + colorDelta;
		const JVector4F minCol = color - colorDelta;
		const JVector4F upLeft = Core::HasSQValueEnum(dirFlag, J_GUI_ORDINAL_DIR_UP_LEFT) ? addCol : minCol;
		const JVector4F upRight = Core::HasSQValueEnum(dirFlag, J_GUI_ORDINAL_DIR_UP_RIGHT) ? addCol : minCol;
		const JVector4F downLeft = Core::HasSQValueEnum(dirFlag, J_GUI_ORDINAL_DIR_DOWN_LEFT) ? addCol : minCol;
		const JVector4F downRight = Core::HasSQValueEnum(dirFlag, J_GUI_ORDINAL_DIR_DOWN_RIGHT) ? addCol : minCol;
		DrawRectFilledMultiColor(pos, size, upLeft, upRight, downLeft, downRight, useRestoreCursorPos, useFrameRounding);
	}
	void JGui::DrawRectFilledMultiColor(const JVector2<float>& pos,
		const JVector2<float>& size,
		const JVector4<float>& upLeftCol,
		const JVector4<float>& upRightCol,
		const JVector4<float>& downLeftCol,
		const JVector4<float>& downRightCol,
		const bool useRestoreCursorPos,
		const bool useFrameRounding)noexcept
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::DRAW);
		if (useFrameRounding)
		{
			Private::Adaptee()->DrawRectFilledMultiColor(pos, size,
				upLeftCol, upRightCol, downLeftCol, downRightCol,
				Private::Adaptee()->GetStyleValueF(J_GUI_STYLE::FRAME_ROUNDING),
				J_GUI_DRAW_FLAG_ROUND_ALL,
				useRestoreCursorPos);
		}
		else
		{
			Private::Adaptee()->DrawRectFilledMultiColor(pos, size,
				upLeftCol, upRightCol, downLeftCol, downRightCol,
				0,
				J_GUI_DRAW_FLAG_NONE,
				useRestoreCursorPos);
		}
	}
	void JGui::DrawRectFilledMultiColor(const JVector2<float>& pos,
		const JVector2<float>& size,
		const JVector4<float>& upLeftCol,
		const JVector4<float>& upRightCol,
		const JVector4<float>& downLeftCol,
		const JVector4<float>& downRightCol,
		const float rounding,
		const J_GUI_DRAW_FLAG_ drawFlag,
		const bool useRestoreCursorPos)noexcept
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::DRAW);
		Private::Adaptee()->DrawRectFilledMultiColor(pos, size,
			upLeftCol, upRightCol, downLeftCol, downRightCol,
			rounding,
			drawFlag,
			useRestoreCursorPos);
	}
	void JGui::DrawRectFilledColor(const JVector2<float>& pos,
		const JVector2<float>& size,
		const JVector4<float>& color,
		const bool useRestoreCursorPos,
		const bool useFrameRounding)noexcept
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::DRAW);
		if (useFrameRounding)
		{
			const float rounding = Private::Adaptee()->GetStyleValueF(J_GUI_STYLE::FRAME_ROUNDING);
			Private::Adaptee()->DrawRectFilledColor(pos, size, color, rounding, J_GUI_DRAW_FLAG_ROUND_ALL, useRestoreCursorPos);
		}
		else
			Private::Adaptee()->DrawRectFilledColor(pos, size, color, 0, 0, useRestoreCursorPos);
	}
	void JGui::DrawRectFilledColor(const JVector2<float>& pos,
		const JVector2<float>& size,
		const JVector4<float>& color,
		const float rounding,
		const J_GUI_DRAW_FLAG_ drawFlag,
		const bool useRestoreCursorPos)noexcept
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::DRAW);
		Private::Adaptee()->DrawRectFilledColor(pos, size, color, rounding, drawFlag, useRestoreCursorPos);
	}
	void JGui::DrawRectFrame(const JVector2<float>& pos,
		const JVector2<float>& size,
		const JVector4<float>& color,
		const float thickness,
		const bool useRestoreCursorPos,
		const bool useFrameRounding)noexcept
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::DRAW);
		if (useFrameRounding)
		{
			const float rounding = Private::Adaptee()->GetStyleValueF(J_GUI_STYLE::FRAME_ROUNDING);
			Private::Adaptee()->DrawRectFrame(pos, size, color, thickness, rounding, J_GUI_DRAW_FLAG_ROUND_ALL, useRestoreCursorPos);
		}
		else
			Private::Adaptee()->DrawRectFrame(pos, size, color, thickness, 0, 0, useRestoreCursorPos);
	}
	void JGui::DrawRectFrame(const JVector2<float>& pos,
		const JVector2<float>& size,
		const JVector4<float>& color,
		const float thickness,
		const float rounding,
		const J_GUI_DRAW_FLAG_ drawFlag,
		const bool useRestoreCursorPos)noexcept
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::DRAW);
		Private::Adaptee()->DrawRectFrame(pos, size, color, thickness, rounding, drawFlag, useRestoreCursorPos);
	}
	void JGui::DrawToolTipBox(const std::string& uniqueLabel,
		const std::string& tooltip,
		const JVector2<float>& pos,
		const JVector2<float>& padding,
		const bool useRestoreCursorPos)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::DRAW);
		Private::Adaptee()->DrawToolTipBox(uniqueLabel, tooltip, pos, padding, useRestoreCursorPos);
	}
	void JGui::DrawToolTipBox(const std::string& uniqueLabel,
		const std::string& tooltip,
		const JVector2<float>& pos,
		const float maxWidth,
		const JVector2<float>& padding,
		const J_EDITOR_ALIGN_TYPE alignType,
		const bool useRestoreCursorPos)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::DRAW);
		Private::Adaptee()->DrawToolTipBox(uniqueLabel, tooltip, pos, maxWidth, padding, alignType, useRestoreCursorPos);
	}
	void JGui::DrawCircleFilledColor(const JVector2<float>& centerPos,
		const float radius,
		const JVector4<float>& color,
		const bool useRestoreCursorPos)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::DRAW);
		Private::Adaptee()->DrawCircleFilledColor(centerPos, radius, color, useRestoreCursorPos);
	}
	void JGui::DrawCircle(const JVector2<float>& centerPos,
		const float radius,
		const JVector4<float>& color,
		const bool useRestoreCursorPos,
		const float thickness)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::DRAW);
		Private::Adaptee()->DrawCircle(centerPos, radius, color, useRestoreCursorPos, thickness);
	}
	void JGui::PushClipRect(const JVector2<float>& rectMinP, const JVector2<float>& rectMaxP, const bool intersectWithCurrentClipRect)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::DRAW);
		Private::Adaptee()->PushClipRect(rectMinP, rectMaxP, intersectWithCurrentClipRect);
	}
	void JGui::AddLine(const JVector2<float>& p1, const JVector2<float>& p2, const JVector4<float>& color, const float thickness)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::DRAW);
		Private::Adaptee()->AddLine(p1, p2, color, thickness);
	}
	void JGui::AddTriangle(const JVector2<float>& p1, const JVector2<float>& p2, const JVector2<float>& p3, const JVector4<float>& color, const float thickness)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::DRAW);
		Private::Adaptee()->AddTriangle(p1, p2, p3, color, thickness);
	}
	void JGui::AddTriangleFilled(const JVector2<float>& p1, const JVector2<float>& p2, const JVector2<float>& p3, const JVector4<float>& color)
	{
		Private::Impl()->AddActWidgetCount(Core::J_GUI_WIDGET_TYPE::DRAW);
		Private::Adaptee()->AddTriangleFilled(p1, p2, p3, color);
	}
	void JGui::NewLine()noexcept
	{
		Private::Adaptee()->NewLine();
	}
	void JGui::SameLine()noexcept
	{
		Private::Adaptee()->SameLine();
	}
	void JGui::Separator()noexcept
	{
		Private::Adaptee()->Separator();
	}
	void JGui::Indent(const float width)noexcept
	{
		Private::Adaptee()->Indent(width);
	}
	void JGui::UnIndent(const float width)noexcept
	{
		Private::Adaptee()->UnIndent(width);
	}
	float JGui::IndentMovementPixel(const float width)noexcept
	{
		return Private::Adaptee()->IndentMovementPixel(width);
	}
	JVector2<float> JGui::GetMainWorkPos()noexcept
	{
		return Private::Adaptee()->GetMainWorkPos();
	}
	JVector2<float> JGui::GetMainWorkSize()noexcept
	{
		return Private::Adaptee()->GetMainWorkSize();
	}
	JVector2<float> JGui::GetDisplaySize()noexcept
	{
		return JWindow::GetDisplaySize();
	}
	JVector2<float> JGui::GetAppWindowSize()noexcept
	{
		return JWindow::GetWindowSize();
	}
	JVector2<float> JGui::GetClientWindowPos()noexcept
	{
		return JWindow::GetClientPosition();
	}
	JVector2<float> JGui::GetClientWindowSize()noexcept
	{
		return JWindow::GetClientSize();
	}
	JVector2<float> JGui::GetDefaultClientWindowMinSize()noexcept
	{
		return JWindow::GetClientSize() * 0.075f;
	}
	JVector2<float> JGui::GetWindowPos()noexcept
	{
		return Private::Adaptee()->GetWindowPos();
	}
	JVector2<float> JGui::GetWindowSize()noexcept
	{
		return Private::Adaptee()->GetWindowSize();
	}
	JVector2<float> JGui::GetRestWindowSpace()noexcept
	{
		return Private::Adaptee()->GetRestWindowSpace();
	}
	JVector2<float> JGui::GetWindowContentsSize()noexcept
	{
		return Private::Adaptee()->GetWindowContentsSize();
	}
	JVector2<float> JGui::GetRestWindowContentsSize()noexcept
	{
		return Private::Adaptee()->GetRestWindowContentsSize();
	}
	JVector2<float> JGui::GetWindowMenuBarSize()noexcept
	{
		return Private::Adaptee()->GetWindowMenuBarSize();
	}
	JVector2<float> JGui::GetWindowTitleBarSize()noexcept
	{
		return Private::Adaptee()->GetWindowTitleBarSize();
	} 
	JVector2<float> JGui::GetWindowScrollPos()noexcept
	{
		return Private::Adaptee()->GetWindowScrollPos();
	}
	int JGui::GetWindowOrder(const GuiID windowID)noexcept
	{
		return Private::Adaptee()->GetWindowOrder(windowID);
	}
	bool JGui::GetWindowInfo(const std::string& wndName, _Inout_ JGuiWindowInfo& info)noexcept
	{
		return Private::Adaptee()->GetWindowInfo(wndName, info);
	}
	bool JGui::GetWindowInfo(const GuiID windowID, _Inout_ JGuiWindowInfo& info)noexcept
	{
		return Private::Adaptee()->GetWindowInfo(windowID, info);
	}
	bool JGui::GetCurrentWindowInfo(_Inout_ JGuiWindowInfo& info)noexcept
	{
		return Private::Adaptee()->GetCurrentWindowInfo(info);
	} 
	std::vector<JGuiWindowInfo> JGui::GetDisplayedWindowInfo()noexcept
	{
		return Private::Adaptee()->GetDisplayedWindowInfo(true);
	}   
	bool JGui::GetFirstIntersectWindowInfo(const JVector2F pos, _Inout_ JGuiWindowInfo& info)noexcept
	{
		auto result = Private::impl->IntersectFirst(pos);
		if (result == nullptr)
			return false;

		info = *result;
		return true;
	}
	std::set<GuiID> JGui::GetWindowOpendTreeNodeID(const GuiID windowID)noexcept
	{ 
		return Private::Adaptee()->GetWindowOpendTreeNodeID(windowID);
	}
	void JGui::SetNextWindowPos(const JVector2<float>& pos, J_GUI_CONDIITON flag)noexcept
	{
		Private::Adaptee()->SetNextWindowPos(pos, flag);
	}
	void JGui::SetNextWindowSize(const JVector2<float>& size, J_GUI_CONDIITON flag)noexcept
	{ 
		if (size <= JVector2F::Zero())
			return;

		Private::Adaptee()->SetNextWindowSize(size, flag);
	}
	void JGui::SetNextWindowFocus()noexcept
	{
		Private::Adaptee()->SetNextWindowFocus();
	}
	bool JGui::IsCurrentWindowFocused(J_GUI_FOCUS_FLAG_ flag)noexcept
	{
		return Private::Adaptee()->IsCurrentWindowFocused((J_GUI_FOCUS_FLAG)flag);
	} 
	void JGui::FocusWindow(const GuiID windowID)
	{
		Private::Adaptee()->FocusWindow(windowID);
	}
	void JGui::FocusCurrentWindow()
	{
		Private::Adaptee()->FocusCurrentWindow();
	}
	void JGui::RestoreFromMaximize(const GuiID windowID, const std::vector<GuiID>& preTabItemID)
	{
		Private::Adaptee()->RestoreFromMaximize(windowID, preTabItemID);
	} 
	GuiID JGui::CalCurrentWindowItemID(const std::string& label)noexcept
	{
		return Private::Adaptee()->CalCurrentWindowItemID(label);
	}
	JVector2<float> JGui::GetLastItemRectMin()noexcept
	{
		return Private::Adaptee()->GetLastItemRectMin();
	}
	JVector2<float> JGui::GetLastItemRectMax()noexcept
	{
		return Private::Adaptee()->GetLastItemRectMax();
	}
	JVector2<float> JGui::GetLastItemRectPos()noexcept
	{
		return Private::Adaptee()->GetLastItemRectMin();
	}
	JVector2<float> JGui::GetLastItemRectSize()noexcept
	{
		return Private::Adaptee()->GetLastItemRectSize();
	}
	void JGui::SetNextItemWidth(const float width)noexcept
	{
		Private::Adaptee()->SetNextItemWidth(width);
	}
	void JGui::SetNextItemOpen(const bool value, J_GUI_CONDIITON flag)noexcept
	{
		Private::Adaptee()->SetNextItemOpen(value, flag);
	}
	void JGui::SetLastItemDefaultFocus()
	{
		Private::Adaptee()->SetLastItemDefaultFocus();
	}
	bool JGui::IsLastItemActivated()noexcept
	{
		return Private::Adaptee()->IsLastItemActivated();
	}
	bool JGui::IsLastItemHovered(J_GUI_HOVERED_FLAG_ flag)noexcept
	{
		return Private::Adaptee()->IsLastItemHovered((J_GUI_HOVERED_FLAG)flag);
	}
	bool JGui::IsLastItemClicked(const Core::J_MOUSE_BUTTON btn)noexcept
	{
		return Private::Adaptee()->IsLastItemClicked(btn);
	}
	bool JGui::IsLastItemAnyClicked(const bool containMiddle, const bool containRight, const bool containLeft)noexcept
	{
		bool res = false;
		if (!res && containMiddle)
			res |= Private::Adaptee()->IsLastItemClicked(Core::J_MOUSE_BUTTON::MIDDLE);
		if (!res && containRight)
			res |= Private::Adaptee()->IsLastItemClicked(Core::J_MOUSE_BUTTON::RIGHT);
		if (!res && containLeft)
			res |= Private::Adaptee()->IsLastItemClicked(Core::J_MOUSE_BUTTON::LEFT);
		return res;
	}
	bool JGui::IsTreeNodeArrowHovered(const std::string& nodeName)
	{
		return Private::Adaptee()->IsTreeNodeArrowHovered(nodeName);
	}
	bool JGui::CanUseWidget(const Core::J_GUI_WIDGET_TYPE type)
	{
		return Private::Adaptee()->CanUseWidget(type);
	}
	JVector2<float> JGui::CalDefaultButtonSize(const std::string& label)noexcept
	{
		return Private::Adaptee()->CalDefaultButtonSize(label);
	}
	JVector2<float> JGui::CalButtionSize(const std::string& label, const JVector2<float>& size)noexcept
	{
		return Private::Adaptee()->CalButtionSize(label, size);
	} 
	float JGui::CalCloseButtionSize()noexcept
	{
		return Private::Adaptee()->CalCloseButtionSize();
	}
	void JGui::PushItemFlag(J_GUI_ITEM_FLAG_ flag, const bool value)noexcept
	{
		Private::Adaptee()->PushItemFlag(flag, value);
	}
	void JGui::PopItemFlag()
	{
		Private::Adaptee()->PopItemFlag();
	}
	void JGui::PushItemWidth(const float width)noexcept
	{
		Private::Adaptee()->PushItemWidth(width);
	}
	void JGui::PopItemWidth()noexcept
	{
		Private::Adaptee()->PopItemWidth();
	}
	float JGui::GetSliderRightAlignPosX(float width , bool hasScrollbar)noexcept
	{ 
		const float posX = GetWindowSize().x - width - GetFramePadding().x - GetItemSpacing().x - JGui::GetScrollBarSize();	 
		return posX < 0 ? GetWindowSize().x : (hasScrollbar ? posX - JGui::GetScrollBarSize() : posX);
	}
	float JGui::GetSliderWidth()noexcept
	{
		return 	GetClientWindowSize().x * 0.05f;
	}
	std::vector<GuiID> GetWindowOpendTreeNodeID(const GuiID windowID)
	{

	}
	bool JGui::IsFullScreen()noexcept
	{
		return JWindow::IsFullScreen();
	}
	bool JGui::IsEnablePopup()noexcept
	{
		return Private::Impl()->option.enablePopup;
	}
	bool JGui::IsEnableSelector()noexcept
	{
		return Private::Impl()->option.enableSelector;
	}
	bool JGui::ComboSet(const std::string& uniqueLabel, int& selectedIndex, const std::vector<std::string>& strVec)
	{
		bool isOpen = false;
		if (BeginCombo(uniqueLabel, strVec[selectedIndex].c_str(), J_GUI_COMBO_FLAG_HEIGHT_LARGE))
		{
			const uint count = (uint)strVec.size();
			for (uint i = 0; i < count; i++)
			{
				bool isSelected = (selectedIndex == i);
				if (Selectable(strVec[i], &isSelected))
					selectedIndex = i;

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (isSelected)
					SetLastItemDefaultFocus();
			}
			EndCombo();
			isOpen = true;
		}
		return isOpen;
	}

#pragma endregion

#pragma region Docking
	bool JGui::GetDockNodeInfoByWindowName(const std::string& windowName, _Inout_ JGuiDockNodeInfo& info)noexcept
	{
		return Private::Adaptee()->GetDockNodeInfoByWindowName(windowName, info);
	};
	bool JGui::GetDockNodeInfo(const std::string& dockNodeName, _Inout_ JGuiDockNodeInfo& info)noexcept
	{
		return Private::Adaptee()->GetDockNodeInfo(dockNodeName, info);
	}
	bool JGui::GetDockNodeInfo(const GuiID dockID, _Inout_ JGuiDockNodeInfo& info)noexcept
	{
		return Private::Adaptee()->GetDockNodeInfo(dockID, info);
	}
	bool JGui::GetDockNodeHostWindowInfo(const GuiID childDockID, _Inout_ JGuiWindowInfo& info)noexcept
	{
		return Private::Adaptee()->GetDockNodeHostWindowInfo(childDockID, info);
	}
	bool JGui::GetCurrentDockNodeInfo(_Inout_ JGuiDockNodeInfo& info)noexcept
	{
		return Private::Adaptee()->GetCurrentDockNodeInfo(info);
	}
	bool JGui::HasDockNode(const std::string& dockNodeName)noexcept
	{
		return Private::Adaptee()->HasDockNode(dockNodeName);
	}
	bool JGui::CanUseDockHirechary()noexcept
	{
		return Private::Adaptee()->CanUseDockHirechary();
	}
	void JGui::BuildDockHirechary(const std::vector<std::unique_ptr<JGuiDockBuildNode>>& nodeVec)
	{
		Private::Adaptee()->BuildDockHirechary(nodeVec);
	}
	void JGui::UpdateDockSpace(const std::string& name, const JVector2<float>& size, J_GUI_DOCK_NODE_FLAG_ dockspaceFlag)
	{
		Private::Adaptee()->UpdateDockSpace(name, size, dockspaceFlag);
	}
	void JGui::OverrideNextDockNodeFlag(J_GUI_DOCK_NODE_FLAG_ flag)
	{
		Private::Adaptee()->OverrideNextDockNodeFlag(flag);
	}
	void JGui::CloseTabItem(const GuiID windowID)
	{
		Private::Adaptee()->CloseTabItem(windowID);
	}
	std::unique_ptr<JDockUpdateHelper> JGui::CreateDockUpdateHelper(const J_EDITOR_PAGE_TYPE pageType)
	{
		return Private::Adaptee()->CreateDockUpdateHelper(pageType);
	}
#pragma endregion



	bool JGuiPrivate::StartGuiUpdate()
	{
		if (Private::Impl()->GetAdaptee() == nullptr)
			return false;

		Private::Adaptee()->SetFont(J_GUI_FONT_TYPE::MEDIUM);
		Private::Impl()->UpdateGuiBackend();
		Private::Impl()->UpdateMouseEvent();
		Private::Impl()->ClearActWidgetCount(); 
		Private::Impl()->ClearDisplayedWindowInfo();
		Private::Adaptee()->PushStyle(J_GUI_STYLE::WINDOW_MIN_SIZE, JGui::GetDefaultClientWindowMinSize());
		return true;
	}
	void JGuiPrivate::EndGuiUpdate()
	{
		Private::Adaptee()->PopStyle(1);  
		Private::Impl()->UpdateLastMouseState();
	}
	void JGuiPrivate::Initialize(std::unique_ptr<Graphic::JGuiInitData>&& initData)
	{
		Private::Impl()->Initialize(std::move(initData));
	}
	void JGuiPrivate::Clear()
	{
		Private::Impl()->Clear();
	}
	Graphic::JGuiBackendInterface* JGuiPrivate::GetBackendInterface()
	{
		return Private::Impl();
	}
	void JGuiPrivate::SetAdapter(std::unique_ptr<JGuiBehaviorAdapter>&& adapter)
	{
		Private::Impl()->SetAdapter(std::move(adapter));
	}
	void JGuiPrivate::LoadOption()
	{
		Private::Impl()->LoadOption();
	}
	void JGuiPrivate::StoreOption()
	{
		Private::Impl()->StoreOption();
	}
	void JGuiPrivate::LoadGuiData()
	{
		Private::Adaptee()->LoadGuiData();
	}
	void JGuiPrivate::StoreGuiData()
	{
		Private::Adaptee()->StoreGuiData();
	}
}