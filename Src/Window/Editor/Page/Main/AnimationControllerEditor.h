#pragma once
#include"../EditorWindow.h" 
#include"../../Event/EditorEventStruct.h" 
#include"../../Event/EditorEventType.h"
#include"../../../../Core/Event/JEventListener.h"
#include"../../../../Utility/JDelegate.h" 
#include<unordered_map>
#include<vector>

namespace JinEngine
{
	class JAnimationController;
	class EditorString;
	class EditorPopup;
	namespace Core
	{
		class JFSMcondition;
		class JAnimationFSMstate;
	}

	class AnimationControllerEditor : public EditorWindow, public Core::JEventListener<size_t, EDITOR_EVENT, EditorEventStruct*>
	{
		struct DiagramViewFuncData
		{
		public:
			float windowPosX;
			float windowPosY;
			float windowWidth;
			float windowHeight;
		};
	private:
		JAnimationController* selectedController;
		int diagramIndex;
		Core::JAnimationFSMstate* selectedState;
		Core::JFSMcondition* selectedCondition;

		std::unique_ptr<EditorString> editorString;
		const std::string diagramListName;
		const std::string parameterListName;
		const std::string diagramViewName;
		std::unique_ptr<EditorPopup>diagramListPopup;
		std::unique_ptr<EditorPopup>parameterListPopup;
		std::unique_ptr<EditorPopup>diagramViewPopup;
		std::unordered_map<size_t, JDelegate<void(EditorUtility*)>> diagramListPopupFunc;
		std::unordered_map<size_t, JDelegate<void(EditorUtility*)>> parameterListPopupFunc;
		std::unordered_map<size_t, JDelegate<void(EditorUtility*, DiagramViewFuncData&)>> diagramViewPopupFunc;

		static constexpr uint stateShapeWidth = 100;
		static constexpr uint stateShapeHeight = 150;
		static constexpr uint nameMaxLength = 50;
		static constexpr uint selectableBufLength = 100;
		static constexpr uint frameThickness = 3;

		std::string nameBuf;
		bool diagramListSelectable[selectableBufLength];
		bool parameterListSelectable[selectableBufLength];
		bool diagramViewSelectable[selectableBufLength];

		float preMousePosX = 0;
		float preMousePosY = 0;

	public:
		AnimationControllerEditor(std::unique_ptr<EditorAttribute> attribute, const size_t ownerPageGuid);
		~AnimationControllerEditor();
		AnimationControllerEditor(const AnimationControllerEditor& rhs) = delete;
		AnimationControllerEditor& operator=(const AnimationControllerEditor& rhs) = delete;

		void Initialize(EditorUtility* editorUtility);
		void UpdateWindow(EditorUtility* editorUtility)override;

		bool Activate(EditorUtility* editorUtility) final;
		bool DeActivate(EditorUtility* editorUtility) final;
	private:
		void BuildDiagramListView(EditorUtility* editorUtility);
		void BuildParameterView(EditorUtility* editorUtility);
		void BuildAnimationDiagramView(EditorUtility* editorUtility, float cursorPosY);
		void CloseAllPopup()noexcept;
		void ClearSelectableBuff()noexcept;

		void CreateNewDiagram(EditorUtility* editorUtility)noexcept;
		void CreateNewParameter(EditorUtility* editorUtility)noexcept;
		void CreateNewState(EditorUtility* editorUtility, DiagramViewFuncData& funcData)noexcept;

		void EraseDiagram(EditorUtility* editorUtility)noexcept;
		void EraseParameter(EditorUtility* editorUtility)noexcept;
		void EraseState(EditorUtility* editorUtility, DiagramViewFuncData& funcData)noexcept;
	private:
		virtual void OnEvent(const size_t& senderGuid, const EDITOR_EVENT& eventType, EditorEventStruct* eventStruct) final;
	};
}
