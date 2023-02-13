#pragma once
#include"../../JEditorWindow.h"  
#include"../../../Interface/JEditorObjectInterface.h" 
#include"../../../../Core/FSM/JFSMparameterValueType.h"

namespace JinEngine
{
	class JAnimationController;
	namespace Core
	{
		class JAnimationFSMdiagram; 
	}
	namespace Editor
	{
		class JEditorPopupMenu;
		class JEditorStringMap;
		class JAnimationDiagramList final : public JEditorWindow,
			public JEditorObjectHandlerInterface
		{
		private:
			Core::JUserPtr<JAnimationController> aniCont;
			Core::JUserPtr<Core::JAnimationFSMdiagram> selectedDiagram; 
		private:
			std::unique_ptr<JEditorPopupMenu> diagramListPopup;
			std::unique_ptr<JEditorStringMap> editorString;
		private:
			using AniContUserPtr = Core::JUserPtr<JAnimationController>;
			using DataHandleStructure = Core::JDataHandleStructure<Core::JTransition::GetMaxTaskCapacity(), Core::JIdentifier>;
			using RegisterEvF = Core::JMFunctorType<JAnimationDiagramList, void>;
			using DiagramCreationFunctor = Core::JFunctor<void, DataHandleStructure&, Core::JDataHandle&, AniContUserPtr, const size_t>;
			using DiagramCreationBind = Core::JBindHandle<DiagramCreationFunctor, const Core::EmptyType&, const Core::EmptyType&, AniContUserPtr, const size_t>;
		private:
			DataHandleStructure fsmdata;
			std::unique_ptr<RegisterEvF::Functor> regCreateDiagramEvF;
			std::unique_ptr<RegisterEvF::Functor> regDestroyDiagramEvF;
			std::unique_ptr<DiagramCreationFunctor> createDiagramF;
			std::unique_ptr<DiagramCreationFunctor> destroyDiagramF;
		public:
			JAnimationDiagramList(const std::string& name,
				std::unique_ptr<JEditorAttribute> attribute,
				const J_EDITOR_PAGE_TYPE ownerPageType,
				const J_EDITOR_WINDOW_FLAG windowFlag);
			~JAnimationDiagramList() = default;
			JAnimationDiagramList(const JAnimationDiagramList& rhs) = delete;
			JAnimationDiagramList& operator=(const JAnimationDiagramList& rhs) = delete;
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		public:
			void Initialize(Core::JUserPtr<JAnimationController> newAniCont)noexcept;
			void UpdateWindow()final;
		private:
			void BuildDiagramList();
		private:
			void RegisterCreateDiagramEv();
			void RegisterDestroyDiagramEv();
		private:
			void CreateDiagram(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniCont, const size_t guid);
			void DestroyDiagram(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniCont, const size_t guid);
		protected:
			void DoSetClose()noexcept final;
		};
	}
}
