#pragma once
#include"JEditor.h"  
#include"JEditorPageEnum.h"
#include"JEditorWindowEnum.h"  
#include"../Gui/JGuiType.h"
#include"../../Core/Func/Functor/JFunctor.h"
#include<set>
namespace JinEngine
{
	class JGameObject;
	namespace Editor
	{
		class JEditorPopupMenu;
		class JEditorStringMap;
		class JEditorAttribute;
		class JDockUpdateHelper;

		class JEditorWindow : public JEditor
		{
		private:
			enum class WINDOW_INNER_EVENT
			{
				CLEAR_SELECTED_OBJECT
			};
		protected: 
			struct State
			{
			public:
				static constexpr int nextFocusWattingFrame = 2;
			public:
				JUserPtr<Core::JIdentifier> hoveredObj;
			public:
				JVector2F nextPos;
				JVector2F nextSize;
			public:
				int nextFocusReqFrame = 0;	// set next window focus if nextFocusReqFrame n to 0
			public:
				bool hasSetNextPosReq = false;	//dock node가 아닌경우면 유효
				bool hasSetNextSizeReq = false;	//dock node가 아닌경우면 유효 
			public:
				bool isWindowOpen = false;
				bool isContentsClick = false; 
				bool isMaximize = false;
			};
			struct Option
			{
			public:
				//selected option
				bool canStaySelected = true;
			};
			struct PopupSetting
			{
			public:
				JEditorPopupMenu* popupMenu;
				JEditorStringMap* stringMap;
				bool canOpenPopup; 
				bool focusWindowIfCloseThisFrame;
			public:
				PopupSetting(JEditorPopupMenu* popupMenu,
					JEditorStringMap* stringMap,
					const bool canOpenPopup = true,
					const bool focusWindowIfCloseThisFrame = true);
			public:
				bool IsValid()const noexcept;
			};
			struct PopupResult
			{
			public:
				bool isOpen = false;
				bool isMouseInPopup = false;
				bool isPopupContentsClicked = false;
				bool isLeafPopupContentsClicked = false; 
				bool isCloseThisFrame = false;
			};
		protected:			 
			using PassSelectedOneF = Core::JSFunctorType<bool, JEditorWindow*>;
			using PassSelectedAboveOneF = Core::JSFunctorType<bool, JEditorWindow*>;
		private: 
			const J_EDITOR_PAGE_TYPE ownerPageType; 
			J_EDITOR_WINDOW_FLAG windowFlag;
		private:
			std::unique_ptr<JDockUpdateHelper> dockUpdateHelper = nullptr;
		private:
			State state; 
			Option option;
		private:
			std::unordered_map<size_t, JUserPtr<Core::JIdentifier>> selectedObjMap;
			std::set<size_t> listenOtherWindowGuidSet;
		public:
			JEditorWindow(const std::string name,
				std::unique_ptr<JEditorAttribute> attribute, 
				const J_EDITOR_PAGE_TYPE ownerPageType, 
				const J_EDITOR_WINDOW_FLAG windowFlag);
			~JEditorWindow();
			JEditorWindow(const JEditorWindow& rhs) = delete;
			JEditorWindow& operator=(const JEditorWindow& rhs) = delete;
		public:
			J_EDITOR_PAGE_TYPE GetOwnerPageType()const noexcept;
			virtual J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept = 0;
		public:
			virtual void UpdateWindow() = 0;
		protected:
			void EnterWindow(J_GUI_WINDOW_FLAG_ flag);
			void CloseWindow();
		protected:
			virtual void UpdateMouseClick();
			virtual void UpdateMouseWheel();
			void UpdateDocking(); 
			void UpdatePopup(const PopupSetting setting);
			void UpdatePopup(const PopupSetting setting, _Out_ PopupResult& result);
		protected: 
			PassSelectedOneF::Functor* GetPassSelectedOneFunctor()noexcept;
			PassSelectedAboveOneF::Functor* GetPassSelectedAboveOneFunctor()noexcept;
			JUserPtr<Core::JIdentifier> GetHoveredObject()const noexcept;
			uint GetSelectedObjectCount()const noexcept;  
			JUserPtr<Core::JIdentifier> GetFirstSelectedObject()const noexcept;
			std::vector<JUserPtr<Core::JIdentifier>> GetSelectedObjectVec()const noexcept;
			template<typename T>
			std::vector<JUserPtr<T>> GetSelectedObjectVec()const noexcept
			{
				std::vector<JUserPtr<T>> vec;
				vec.reserve(selectedObjMap.size());

				Core::JTypeInfo& info = T::StaticTypeInfo();
				for (const auto& data : selectedObjMap)
				{
					JUserPtr<T> ptr = Core::ConnectChildUserPtr<T>(data.second);
					if (ptr != nullptr)
						vec.push_back(ptr);
				}
				return vec;
			}
		public: 
			//dock node가 아닌경우만 유효
			void SetNextWindowPos(const JVector2F& pos)noexcept;
			//dock node가 아닌경우만 유효
			void SetNextWindowSize(const JVector2F& size)noexcept;
			void SetMaximize(const bool value)noexcept; 
		protected:
			void SetOption(const Option& newOption)noexcept; 
			void SetHoveredObject(JUserPtr<Core::JIdentifier> obj)noexcept;
			void SetSelectedGameObjectTrigger(const JUserPtr<JGameObject>& gObj, const bool triggerValue)noexcept;
			void SetContentsClick(const bool value)noexcept;
		protected:
			bool IsSelectedObject(const size_t guid)const noexcept;
			bool CanUseDock()const noexcept;
			bool CanUseSelectedMap()const noexcept;
			bool CanUsePopup()const noexcept;
			bool CanMaximize()const noexcept;
			bool IsContentsClicked()const noexcept;  
		protected:
			void PushSelectedObject(JUserPtr<Core::JIdentifier> obj)noexcept;
			void PopSelectedObject(JUserPtr<Core::JIdentifier> obj)noexcept;
			void ClearSelectedObject();
		protected:
			void PushOtherWindowGuidForListenEv(const size_t guid)noexcept;
			void PopOtherWindowGuidForListenEv(const size_t guid)noexcept;
		protected:
			bool RegisterEventListener(const J_EDITOR_EVENT evType);
			bool RegisterEventListener(std::vector<J_EDITOR_EVENT>& evType);
			void DeRegisterEventListener(const J_EDITOR_EVENT evType);
			void DeRegisterListener();
		protected:
			//Support undo redo 
			//delay call
			void RequestPushSelectObject(const JUserPtr<Core::JIdentifier>& selectObj);
			void RequestPushSelectObject(const std::vector<JUserPtr<Core::JIdentifier>>& selectObjVec);
			void RequestPopSelectObject(const JUserPtr<Core::JIdentifier>& selectObj);
			void RequestPopSelectObject(const std::vector<JUserPtr<Core::JIdentifier>>& selectObjVec);
			void RequestBind(const std::string& desc,
				std::unique_ptr<Core::JBindHandleBase>&& doHandle, 
				std::unique_ptr<Core::JBindHandleBase>&& undoHandle); 
		private:
			void ExecuteEv(const WINDOW_INNER_EVENT evType); 
			void ExecuteThisWindowNotifiedEv(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* ev);
			void ExecuteOtherWindowNotifiedEv(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* ev);
		protected:
			void TryBeginDragging(const JUserPtr<Core::JIdentifier> selectObj);
			JUserPtr<Core::JIdentifier> TryGetDraggingTarget();
		protected:
			void DoSetOpen()noexcept override;
			void DoSetClose()noexcept override;
			void DoActivate()noexcept override;
			void DoDeActivate()noexcept override;
		public:
			virtual void LoadEditorWindow(JFileIOTool& tool);
			virtual void StoreEditorWindow(JFileIOTool& tool);
		protected:
			void OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* ev) override;
		};
	}
}