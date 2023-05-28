#pragma once
#include"JEditor.h"  
#include"JEditorPageEnum.h"
#include"JEditorWindowEnum.h" 
#include"JEditorWindowFontType.h" 
#include"../../Core/Func/Functor/JFunctor.h"
 
namespace JinEngine
{
	class JGameObject;
	namespace Editor
	{
		class JEditorPopupMenu;
		class JEditorStringMap;
		class JEditorAttribute;
		class JEditorWindowDockUpdateHelper;

		class JEditorWindow : public JEditor
		{ 
		protected:
			struct PopupSetting
			{
			public:
				JEditorPopupMenu* popupMenu;
				JEditorStringMap* stringMap;
				bool canOpenPopup; 
			public:
				PopupSetting(JEditorPopupMenu* popupMenu,
					JEditorStringMap* stringMap,
					bool canOpenPopup = true);
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
			};
		protected:			 
			using PassSelectedOneF = Core::JSFunctorType<bool, JEditorWindow*>;
			using PassSelectedAboveOneF = Core::JSFunctorType<bool, JEditorWindow*>;
		private: 
			const J_EDITOR_PAGE_TYPE ownerPageType; 
			bool isWindowOpen = false;
			J_EDITOR_WINDOW_FLAG windowFlag;
		private:
			std::unique_ptr<JEditorWindowDockUpdateHelper> dockUpdateHelper = nullptr;
		private:
			JUserPtr<Core::JIdentifier> hoveredObj;
			std::unordered_map<size_t, JUserPtr<Core::JIdentifier>> selectedObjMap;
			bool isContentsClick = false;
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
			void EnterWindow(int guiWindowFlag);
			void CloseWindow();
		protected:
			void UpdateMouseClick();
			void UpdateDocking(); 
			void UpdatePopup(const PopupSetting setting);
			void UpdatePopup(const PopupSetting setting, _Out_ PopupResult& result);
		protected:
			bool IsSelectedObject(const size_t guid)const noexcept;
			bool CanUseSelectedMap()const noexcept;
			bool CanUsePopup()const noexcept; 
		protected: 
			PassSelectedOneF::Functor* GetPassSelectedOneFunctor()noexcept;
			PassSelectedAboveOneF::Functor* GetPassSelectedAboveOneFunctor()noexcept;
			JUserPtr<Core::JIdentifier> GetHoveredObject()const noexcept;
			uint GetSelectedObjectCount()const noexcept;
			std::vector<JUserPtr<Core::JIdentifier>> GetSelectedObjectVec()const noexcept;
			JVector4<float> GetSelectedColorFactor()const noexcept;
		protected:
			void SetButtonColor(const JVector4<float>& factor)noexcept;
			void SetTreeNodeColor(const JVector4<float>& factor)noexcept; 
			void SetTreeNodeColorToDefault()noexcept;
			void SetHoveredObject(JUserPtr<Core::JIdentifier> obj)noexcept;
			void SetSelectedGameObjectTrigger(const JUserPtr<JGameObject>& gObj, const bool triggerValue)noexcept;
			void SetContentsClick(const bool value)noexcept;
		protected:
			void PushSelectedObject(JUserPtr<Core::JIdentifier> obj)noexcept;
			void ClearSelectedObject();
		protected:
			bool RegisterEventListener(const J_EDITOR_EVENT evType);
			bool RegisterEventListener(std::vector<J_EDITOR_EVENT>& evType);
			void DeRegisterEventListener(const J_EDITOR_EVENT evType);
			void DeRegisterListener();
		protected:
			//Support undo redo 
			void RequestPushSelectObject(const JUserPtr<Core::JIdentifier>& selectObj);
			void RequestPushSelectObject(const std::vector<JUserPtr<Core::JIdentifier>>& selectObjVec);
			void RequestPopSelectObject(const JEditorPopSelectObjectEvStruct& evStruct);
			void RequesBind(const std::string& desc,
				std::unique_ptr<Core::JBindHandleBase>&& doHandle, 
				std::unique_ptr<Core::JBindHandleBase>&& undoHandle); 
		protected:
			void TryBeginDragging(const JUserPtr<Core::JIdentifier> selectObj);
			JUserPtr<Core::JIdentifier> TryGetDraggingTarget();
		protected:
			void DoSetOpen()noexcept override;
			void DoSetClose()noexcept override;
			void DoActivate()noexcept override;
			void DoDeActivate()noexcept override;
		public:
			virtual void StoreEditorWindow(std::wofstream& stream);
			virtual void LoadEditorWindow(std::wifstream& stream);
		protected:
			void OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct) override;
		};
	}
}