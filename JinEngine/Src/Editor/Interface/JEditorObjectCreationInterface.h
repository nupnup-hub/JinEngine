#pragma once
#include"JEditorObjectHandleInterface.h" 
#include"../Event/JEditorEvent.h"
#include"../Page/JEditorWindow.h"
#include"../../Object/GameObject/JGameObject.h"
#include"../../Core/Empty/EmptyType.h"
#include"../../Core/Identity/JIdentifier.h"
#include"../../Core/Handle/JDataHandleStructure.h"
#include"../Interface/JEditorTransitionInterface.h"

#include"../../Utility/JCommonUtility.h"
#include<memory>
#include<vector>

namespace JinEngine
{
	namespace Core
	{
		class JIdentifier;
	}
	namespace Editor
	{
		class JEditorWindow;

		namespace Constants
		{
			using EventInterface = Core::JEventInterface<size_t, J_EDITOR_EVENT, JEditorEvStruct*>;
			using JEventListener = typename EventInterface::Listener;		 
			using NotifyPtr = typename JEventListener::NotifyPtr;
			using AddEventPtr = typename JEventListener::AddEventNotificationPtr;

			using ClearTaskFunctor = typename Core::JSFunctorType<void, std::vector<size_t>>::Functor;
			//using NotifyPtr = void(JEditorWindow::*)(Core::JEventInterface<size_t, J_EDITOR_EVENT, JEditorEvStruct*>&, size_t, J_EDITOR_EVENT, JEditorEvStruct*);
			using DataHandleStructure = Core::JDataHandleStructure<Core::JTransition::GetMaxTaskCapacity(), Core::JIdentifier>;
		}

		struct JEditorCreationHint
		{
		private:
			using NotifyPtr = Constants::NotifyPtr;
		public:
			JEditorWindow* editorWnd;
		public:
			Core::JTypeInstanceSearchHint openDataHint;		// page open selected data
			Core::JTypeInstanceSearchHint ownerDataHint;	// object owner data ex) scene own gameObject
			bool isSetOpenDataBit = false;
			bool isSetOwnerDataBit = false;
			bool isSetTargetDataBit = false;
			bool canSetModBit = false; 
		public:
			const NotifyPtr notifyPtr;
		public:
			JEditorCreationHint(JEditorWindow* editorWnd,
				bool isSetOpenDataBit,
				bool isSetOwnerDataBit,
				bool isSetTargetDataBit,
				bool canSetModBit, 
				Core::JTypeInstanceSearchHint openDataHint = Core::JTypeInstanceSearchHint{},
				Core::JTypeInstanceSearchHint ownerDataHint = Core::JTypeInstanceSearchHint{},
				NotifyPtr notifyPtr = nullptr);
		public:
			bool IsValid()const noexcept;
		};

		struct JEditorRequestHint
		{
		public:
			using AddEventPtr = Constants::AddEventPtr;
			using ClearTaskFunctor = Constants::ClearTaskFunctor;
		public:
			AddEventPtr addEventPtr;
			ClearTaskFunctor* clearTaskFunctor;
		public:
			JEditorRequestHint(AddEventPtr addEventPtr, ClearTaskFunctor* clearTaskFunctor);
		public:
			bool IsValid()const noexcept;
		};

		template<typename ...Param> class JEditorObjectCreateInterface;
		class JEditorObjectDestroyInterface;
		class JEditorObjectUndoDestroyInterface;
		//passing class
		class JEditorObjectReleaseInterface
		{
		private:
			template<typename ...Param> friend class JEditorObjectCreateInterface;
			friend class JEditorObjectDestroyInterface;
			friend class JEditorObjectUndoDestroyInterface;
		private:
			static Core::JOwnerPtr<Core::JIdentifier> ReleaseInstance(Core::JIdentifier* ptr);
			static bool RestoreInstance(Core::JOwnerPtr<Core::JIdentifier>&& instance);
		};

		template<typename ...Param>
		class JEditorObjectCreateInterface : public JEditorObjectHandlerInterface
		{
		protected:
			using NotifyPtr = typename Constants::NotifyPtr;
			using DataHandleStructure = typename Constants::DataHandleStructure;
		public:
			using PreProcessF = Core::JSFunctorType<void, JEditorWindow*>;
			using PostProcessF = Core::JSFunctorType<void, JEditorWindow*, Core::JIdentifier*>;
			using CanCreateF = Core::JSFunctorType<bool, const size_t, const JEditorCreationHint&, Param...>;
			using ObjectCreateF = Core::JSFunctorType<void, const size_t, const JEditorCreationHint&, Param...>;
		public:
			using CreateUseTransitionF = typename Core::JTransitionCreationHandleType<DataHandleStructure&,
				const size_t,
				const JEditorCreationHint,
				Param...>;
			using CreateF = typename Core::JMFunctorType <JEditorObjectCreateInterface, void, const size_t,
				const JEditorCreationHint,
				Param...>;
		private:
			std::unique_ptr<PreProcessF::Functor> preProcessF;
			std::unique_ptr<PostProcessF::Functor> postProcessF;
			std::unique_ptr<typename CreateUseTransitionF::Functor> createUseTransitionF;
			std::unique_ptr<typename CreateF::Functor> createF;
			std::unique_ptr<typename ObjectCreateF::Functor> objectCreationF;
			std::unique_ptr<typename CanCreateF::Functor> canCreateF;
		public:
			JEditorObjectCreateInterface()
			{
				createUseTransitionF = std::make_unique<typename CreateUseTransitionF::Functor>(&JEditorObjectCreateInterface::CreateUseTransition, this);
				createF = std::make_unique<typename CreateF::Functor>(&JEditorObjectCreateInterface::Create, this);
			}
		public:
			typename CreateUseTransitionF::Functor* GetCreateUseTransitionFunctor()
			{
				return createUseTransitionF.get();
			}
			typename CreateF::Functor* GetCreateFunctor()
			{
				return createF.get();
			}
		public:
			void RegisterCreatePreProcess(PreProcessF::Ptr ptr)
			{
				preProcessF = std::make_unique<PreProcessF::Functor>(ptr);
			}
			void RegisterCreatePostProcess(PostProcessF::Ptr ptr)
			{
				postProcessF = std::make_unique<PostProcessF::Functor>(ptr);
			}
			void RegisterCanCreationF(typename CanCreateF::Ptr ptr)
			{
				canCreateF = std::make_unique<typename CanCreateF::Functor>(ptr);
			}
			void RegisterObjectCreationF(typename ObjectCreateF::Ptr ptr)
			{
				objectCreationF = std::make_unique<typename ObjectCreateF::Functor>(ptr);
			}
		public:
			void CreateUseTransition(DataHandleStructure& dS,
				Core::JDataHandle& dH,
				const size_t guid,
				const JEditorCreationHint creationHint,
				Param... var)
			{  
				bool isReleased = false;
				JEditorWindow* editorWnd = creationHint.editorWnd;
				Core::JOwnerPtr<Core::JIdentifier> data = dS.Release(dH);
				if (data.IsValid())
				{  
					if (editorWnd != nullptr && preProcessF != nullptr)
						(*preProcessF)(editorWnd);
					if (!JEditorObjectReleaseInterface::RestoreInstance(std::move(data)))
					{
						JEditorTransition::Instance().Log("Creation fail", "can't restore instance");
						return;
					}
					isReleased = true;
				}
				else
				{
					bool canCreate = objectCreationF != nullptr && CanCreate(std::move(guid), std::move(creationHint), std::forward<Param>(var)...);
					if (!canCreate)
					{
						JEditorTransition::Instance().Log("Creation fail", "can't pass condition");
						return;
					}
					if (editorWnd != nullptr && preProcessF != nullptr)
						(*preProcessF)(editorWnd);
					(*objectCreationF)(std::move(guid), std::move(creationHint), std::forward<Param>(var)...);
				}

				CreatePostProccess(isReleased, guid, creationHint, std::forward<Param>(var)...);
			}
			void Create(const size_t guid, const JEditorCreationHint creationHint, Param... var)
			{
				JEditorWindow* editorWnd = creationHint.editorWnd;
				bool canCreate = objectCreationF != nullptr && CanCreate(std::move(guid), std::move(creationHint), std::forward<Param>(var)...);
				if (!canCreate)
					return;

				if (editorWnd != nullptr && preProcessF != nullptr)
					(*preProcessF)(editorWnd);
				(*objectCreationF)(std::move(guid), std::move(creationHint), std::forward<Param>(var)...);

				CreatePostProccess(false, guid, creationHint, std::forward<Param>(var)...);
			}
			bool CanCreate(const size_t guid, const JEditorCreationHint creationHint, Param... var)
			{
				if (canCreateF == nullptr)
					return true;

				return (*canCreateF)(std::move(guid), std::move(creationHint), std::forward<Param>(var)...);
			}
		private:
			void CreatePostProccess(const bool isReleased,
				const size_t guid,
				const JEditorCreationHint creationHint,
				Param... var)
			{
				JEditorWindow* editorWnd = creationHint.editorWnd;
				Core::JIdentifier* ptr = Core::SearchRawPtr(Core::JIdentifier::StaticTypeInfo(), guid); 
				if (ptr == nullptr)
				{
					JEditorTransition::Instance().Log("Creation fail", "invalid guid");
					return;
				}

				JEditorTransition::Instance().Log(L"Creation success", ptr->GetNameWithType());
				if (creationHint.canSetModBit)
				{
					if (creationHint.isSetOpenDataBit)
						SetModifiedBit(Core::GetUserPtr(creationHint.openDataHint), true);
					if (creationHint.isSetOwnerDataBit)
						SetModifiedBit(Core::GetUserPtr(creationHint.ownerDataHint), true);
					if (creationHint.isSetTargetDataBit)
						SetModifiedBit(Core::GetUserPtr(ptr), true);
				}

				if (isReleased && editorWnd != nullptr && creationHint.notifyPtr != nullptr)
				{
					JEditorPushSelectObjectEvStruct selectEv{ editorWnd->GetOwnerPageType(), editorWnd->GetWindowType(), Core::GetUserPtr(ptr) };
					(editorWnd->*creationHint.notifyPtr)(*JEditorEvent::EvInterface(), editorWnd->GetGuid(), J_EDITOR_EVENT::PUSH_SELECT_OBJECT, &selectEv);
				}

				if (editorWnd != nullptr && postProcessF != nullptr)
					(*postProcessF)(editorWnd, ptr);
			}
		};

		class JEditorObjectDestroyInterface : public JEditorObjectHandlerInterface
		{
		protected:
			using DataHandleStructure = Constants::DataHandleStructure;
		public:
			using PreProcessF = Core::JSFunctorType<void, JEditorWindow*, Core::JIdentifier*>;
			using PostProcessF = Core::JSFunctorType<void, JEditorWindow*>;
		public:
			using DestroyMultiUseTransitionF = Core::JTransitionMultiCreationHandleType<DataHandleStructure&,
				const std::vector<size_t>,
				const bool,
				const JEditorCreationHint>;
			using DestroySingleUseTransitionF = Core::JTransitionCreationHandleType<DataHandleStructure&,
				const size_t,
				const bool,
				const JEditorCreationHint>;
			using DestroyMultiF = Core::JMFunctorType <JEditorObjectDestroyInterface, void, 
				const std::vector<size_t>,
				const bool,
				const JEditorCreationHint>;
			using DestroySingleF = Core::JMFunctorType <JEditorObjectDestroyInterface, bool,
				const size_t,
				const bool,
				const JEditorCreationHint>;
		private:
			std::unique_ptr<PreProcessF::Functor> preProcessF;
			std::unique_ptr<PostProcessF::Functor> postProcessF;
			std::unique_ptr<DestroyMultiUseTransitionF::Functor> destroyMultiUseTransitionF;
			std::unique_ptr<DestroySingleUseTransitionF::Functor> destroySingleUseTransitionF;
			std::unique_ptr<DestroyMultiF::Functor> destroyMultiF;
			std::unique_ptr<DestroySingleF::Functor> destroySingleF;
		public:
			JEditorObjectDestroyInterface();
		public:
			DestroyMultiUseTransitionF::Functor* GetMultiUseTransitionFunctor();
			DestroySingleUseTransitionF::Functor* GetSingleUseTransitionFunctor();
			DestroyMultiF::Functor* GetMultiFunctor();
			DestroySingleF::Functor* GetSingleFunctor();
		public:
			void RegisterDestroyPreProcess(PreProcessF::Ptr ptr);
			void RegisterDestroyPostProcess(PostProcessF::Ptr ptr);
		public:
			//Destroy object and push DataHandleStructure
			void DestroyMultiUseTransition(DataHandleStructure& dS,
				std::vector<Core::JDataHandle>& dHVec,
				const std::vector<size_t> guidVec,
				const bool useTransition,
				JEditorCreationHint creationHint);
			void DestroySingleUseTransition(DataHandleStructure& dS,
				Core::JDataHandle& dH,
				const size_t guid,
				const bool useTransition,
				JEditorCreationHint creationHint);
			void DestroyMulti(const std::vector<size_t> guidVec,
				const bool useTransition,
				JEditorCreationHint creationHint);
			bool DestroySingle(const size_t guid,
				const bool useTransition,
				JEditorCreationHint creationHint);
		private:
			void DestroyPreProccess(Core::JIdentifier* rawPtr,
				const bool useTransition,
				JEditorCreationHint creationHint);
			void DestroyPostProccess(const size_t guid,
				const bool useTransition,
				JEditorCreationHint creationHint);
		};

		class JEditorObjectUndoDestroyInterface : public JEditorObjectHandlerInterface
		{
		protected:
			using NotifyPtr = Constants::NotifyPtr;
			using DataHandleStructure = Constants::DataHandleStructure;
		public:
			using PreProcessF = Core::JSFunctorType<void, JEditorWindow*>;
			using PostProcessF = Core::JSFunctorType<void, JEditorWindow*, Core::JIdentifier*>;
		public:
			using UndoDestroyF = Core::JTransitionMultiCreationHandleType<DataHandleStructure&,
				const JEditorCreationHint>;
		private:
			std::unique_ptr<PreProcessF::Functor> preProcessF;
			std::unique_ptr<PostProcessF::Functor> postProcessF;
			std::unique_ptr<UndoDestroyF::Functor> undoDestroyF;
		public:
			JEditorObjectUndoDestroyInterface();
		public:
			UndoDestroyF::Functor* GetFunctor();
		public:
			void RegisterUndoDestroyPreProcess(PreProcessF::Ptr ptr);
			void RegisterUndoDestroyPostProcess(PostProcessF::Ptr ptr);
		public:
			void UndoDestroy(DataHandleStructure& dS, std::vector<Core::JDataHandle>& dHVec, const JEditorCreationHint creationHint);
		};

		template<typename ObjectCreateInterface>
		class JEditorCreationRequestor
		{
		public:
			using NotifyPtr = Constants::NotifyPtr;
			using AddEventPtr = Constants::AddEventPtr;
			using ClearTaskFunctor = Constants::ClearTaskFunctor;
			using DataHandleStructure = Constants::DataHandleStructure;
		public:
			using CreateInteface = typename ObjectCreateInterface;
			using DestroyInterface = JEditorObjectDestroyInterface;
		private:
			using CreateUseTransitionF = typename ObjectCreateInterface::CreateUseTransitionF;
			using CreateF = typename ObjectCreateInterface::CreateF;
			using DestroyUseTransitionF = JEditorObjectDestroyInterface::DestroySingleUseTransitionF;
			using DestroyF = JEditorObjectDestroyInterface::DestroySingleF;
		private:
			ObjectCreateInterface creationInterface;
			JEditorObjectDestroyInterface destructionInterface;
		public:
			ObjectCreateInterface* GetCreationInterface()
			{
				return &creationInterface;
			}
			JEditorObjectDestroyInterface* GetDestruectionInterface()
			{
				return &destructionInterface;
			}
		public:
			template<typename ...Param>
			void RequestCreateObject(DataHandleStructure& dS,
				const bool useTransition,
				JEditorCreationHint creationHint,
				const size_t guid,
				const JEditorRequestHint requestHint,
				Param... var)
			{
				if (!requestHint.IsValid() || !creationHint.IsValid())
					return;

				if (!creationInterface.CanCreate(std::move(guid), std::move(creationHint), std::forward<Param>(var)...))
				{
					JEditorTransition::Instance().Log("Creation request fail", "can't pass condition");
					return;
				}
				if (useTransition)
				{
					auto doBind = std::make_unique<CreateUseTransitionF::Bind>(*creationInterface.GetCreateUseTransitionFunctor(),
						Core::empty, Core::empty,
						std::move(guid),
						std::move(creationHint),
						std::forward<Param>(var)...);

					auto undoBind = std::make_unique<DestroyUseTransitionF::Bind>(*destructionInterface.GetSingleUseTransitionFunctor(),
						Core::empty, Core::empty,
						std::move(guid),
						true,
						std::move(creationHint));

					size_t evGuid;

					using CreateTransitionTEv = JEditorTCreateBindFuncEvStruct<DataHandleStructure, CreateUseTransitionF::Bind, DestroyUseTransitionF::Bind, false>;
					auto evStruct = JEditorEvent::RegisterEvStruct(std::make_unique<CreateTransitionTEv>
						("Create task", "use transition",creationHint.editorWnd->GetOwnerPageType(), std::move(doBind), std::move(undoBind), dS), evGuid);
					(*requestHint.clearTaskFunctor)(std::vector<size_t>{evGuid});
					(creationHint.editorWnd->*requestHint.addEventPtr)(*JEditorEvent::EvInterface(), creationHint.editorWnd->GetGuid(), J_EDITOR_EVENT::T_BIND_FUNC, evStruct);
				}
				else
				{ 
					auto doBind = std::make_unique<CreateF::CompletelyBind>(*creationInterface.GetCreateFunctor(),
						std::move(guid),
						std::move(creationHint),
						std::forward<Param>(var)...);

					auto evStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorBindFuncEvStruct>
						(std::move(doBind), creationHint.editorWnd->GetOwnerPageType()));
					static_cast<JEditorBindFuncEvStruct*>(evStruct)->SetLog(std::make_unique<Core::JLogBase>("Create task", "Don't use transition"));
					(creationHint.editorWnd->*requestHint.addEventPtr)(*JEditorEvent::EvInterface(), creationHint.editorWnd->GetGuid(), J_EDITOR_EVENT::BIND_FUNC, evStruct);
				}
			}

		};

		class JEditorDestructionRequestor
		{
		public:
			using NotifyPtr = Constants::NotifyPtr;
			using AddEventPtr = Constants::AddEventPtr;
			using ClearTaskFunctor = Constants::ClearTaskFunctor;
			using DataHandleStructure = Constants::DataHandleStructure;
		public:
			using DestroyInterface = JEditorObjectDestroyInterface;
			using UndoDestroyInterface = JEditorObjectUndoDestroyInterface;
		private:
			using DestroyMultiUseTransitionF = JEditorObjectDestroyInterface::DestroyMultiUseTransitionF;
			using DestroyMultiF = JEditorObjectDestroyInterface::DestroyMultiF;
			using UndoDestroyF = JEditorObjectUndoDestroyInterface::UndoDestroyF;
		private:
			JEditorObjectDestroyInterface destructionInterface;
			JEditorObjectUndoDestroyInterface undoDestructionInterface;
		public:
			JEditorObjectDestroyInterface* GetDestruectionInterface();
			JEditorObjectUndoDestroyInterface* GetUndoDestruectionInterface();
		public:
			void RequestDestroyObject(DataHandleStructure& dS,
				const bool useTransition,
				JEditorCreationHint creationHint,
				const std::vector<Core::JUserPtr<Core::JIdentifier>>& idenVec,
				const JEditorRequestHint requestHint);
		};


#define DEFAULT_CD_REQUESTOR(typeName, editorWndName)													\
		class typeName																					\
		{																								\
		private:																						\
			using CreationInterface = JEditorCreationRequestor<JEditorObjectCreateInterface<>>;			\
			using DestructionInterface = JEditorDestructionRequestor;									\
		public:																							\
			using CanCreateF = CreationInterface::CreateInteface::CanCreateF;							\
			using CreateF = CreationInterface::CreateInteface::ObjectCreateF;							\
		public:																							\
			using DataHandleStructure = CreationInterface::DataHandleStructure;							\
			using NotifyPtr = CreationInterface::NotifyPtr;												\
		public:																							\
			DataHandleStructure dS;																		\
		public:																							\
			CreationInterface creation;																	\
			DestructionInterface destructuion;															\
		public:																							\
			using RequestEvF = Core::JSFunctorType<void, editorWndName*>;								\
		public:																							\
			std::unique_ptr<RequestEvF::Functor> reqCreateStateEvF;									\
			std::unique_ptr<RequestEvF::Functor> reqDestroyEvF;										\
		public:																							\
			typeName(RequestEvF::Ptr reqCreateStateEvPtr, RequestEvF::Ptr reqDestroyEvPtr)							\
			{																											\
				reqCreateStateEvF = std::make_unique<RequestEvF::Functor>(reqCreateStateEvPtr);						\
				reqDestroyEvF = std::make_unique<RequestEvF::Functor>(reqDestroyEvPtr);								\
			}																											\
			~typeName()																									\
			{																											\
				dS.Clear();																								\
			}																											\
		};							


	}
}