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
		class JFSMparameter;
	}
	namespace Editor
	{ 
		class JEditorPopupMenu;
		class JEditorStringMap;
		class JAnimationParameterList final : public JEditorWindow,
			public JEditorObjectHandlerInterface
		{
		private:
			Core::JUserPtr<JAnimationController> aniCont;
			Core::JUserPtr<Core::JAnimationFSMdiagram> selectedDiagram;
			Core::JUserPtr<Core::JFSMparameter> selectedParameter;
		private: 
			std::unique_ptr<JEditorPopupMenu> parameterListPopup;
			std::unique_ptr<JEditorStringMap> editorString;
		private:
			using AniContUserPtr = Core::JUserPtr<JAnimationController>;
			using DataHandleStructure = Core::JDataHandleStructure<Core::JTransition::GetMaxTaskCapacity(), Core::JIdentifier>;
			using RegisterEvF = Core::JMFunctorType<JAnimationParameterList, void>;
			using ParameterCreationFunctor = Core::JFunctor<void, DataHandleStructure&, Core::JDataHandle&, AniContUserPtr, size_t>;
			using ParameterCreationBind = Core::JBindHandle<ParameterCreationFunctor, const Core::EmptyType&, const Core::EmptyType&, AniContUserPtr, const size_t>;
			
			using SetParameterTypeFunctor = Core::JFunctor<void, const Core::J_FSM_PARAMETER_VALUE_TYPE, AniContUserPtr, size_t>;
			using SetParameterNameFunctor = Core::JFunctor<void, const std::string, AniContUserPtr, size_t>;
			using SetParameterBooleanValueFunctor = Core::JFunctor<void, const bool, AniContUserPtr, size_t>;
			using SetParameterIntValueFunctor = Core::JFunctor<void, const int, AniContUserPtr, size_t>;
			using SetParameterFloatValueFunctor = Core::JFunctor<void, const float, AniContUserPtr, size_t>;
		private:
			DataHandleStructure fsmdata;
			std::unique_ptr<RegisterEvF::Functor> regCreateParameterEvF;
			std::unique_ptr<RegisterEvF::Functor> regDestroyParameterEvF;
			std::unique_ptr<ParameterCreationFunctor> createParameterF;
			std::unique_ptr<ParameterCreationFunctor> destroyParameterF;

			std::unique_ptr<SetParameterTypeFunctor> setParameterTypeF;
			std::unique_ptr<SetParameterNameFunctor> setParameterNameF;
			std::unique_ptr<SetParameterBooleanValueFunctor> setParameterBoolF;
			std::unique_ptr<SetParameterIntValueFunctor> setParameterIntF;
			std::unique_ptr<SetParameterFloatValueFunctor> setParameterFloatF;
		public:
			JAnimationParameterList(const std::string& name,
				std::unique_ptr<JEditorAttribute> attribute,
				const J_EDITOR_PAGE_TYPE ownerPageType,
				const J_EDITOR_WINDOW_FLAG windowFlag);
			~JAnimationParameterList() = default;
			JAnimationParameterList(const JAnimationParameterList& rhs) = delete;
			JAnimationParameterList& operator=(const JAnimationParameterList& rhs) = delete;
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		public:
			void Initialize(Core::JUserPtr<JAnimationController> newAniCont)noexcept;
			void UpdateWindow()final;
		private:
			void BuildParameterList();
		private:
			void RegisterCreateParameterEv();
			void RegisterDestroyParameterEv();
		private:
			void CreateParameter(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniCont, const size_t guid);
			void DestroyParameter(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniCont, const size_t guid);
		protected:
			void DoSetClose()noexcept final; 
		};
	}
}
