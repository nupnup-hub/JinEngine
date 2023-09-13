#include"JEditorObjectCreationInterface.h"  
#include"../../Object/JObject.h"
#include"../../Core/Identity/JIdentifierPrivate.h"
#include"../../Core/FSM/JFSMinterface.h"

namespace JinEngine
{
	namespace Editor
	{ 
		namespace
		{
			using ReleaseInterface = JEditorObjectReleaseInterface;
		}
		JEditorCreationHint::JEditorCreationHint(JEditorWindow* editorWnd,
			bool isSetOpenDataBit,
			bool isSetOwnerDataBit,
			bool isSetTargetDataBit,
			bool canSetModBit, 
			Core::JTypeInstanceSearchHint openDataHint,
			Core::JTypeInstanceSearchHint ownerDataHint,
			NotifyPtr notifyPtr)
			:editorWnd(editorWnd),
			isSetOpenDataBit(isSetOpenDataBit),
			isSetOwnerDataBit(isSetOwnerDataBit),
			isSetTargetDataBit(isSetTargetDataBit),
			canSetModBit(canSetModBit), 
			openDataHint(openDataHint),
			ownerDataHint(ownerDataHint),
			notifyPtr(notifyPtr)
		{}  
		bool JEditorCreationHint::IsValid()const noexcept
		{
			return editorWnd != nullptr;
		}

		JEditorRequestHint::JEditorRequestHint(AddEventPtr addEventPtr, ClearTaskFunctor* clearTaskFunctor)
			:addEventPtr(addEventPtr), clearTaskFunctor(clearTaskFunctor)
		{}
		bool JEditorRequestHint::IsValid()const noexcept
		{
			return addEventPtr != nullptr && clearTaskFunctor != nullptr;
		}

		JEditorObjectDestroyInterface::JEditorObjectDestroyInterface()
		{ 
			destroyMultiUseTransitionF = std::make_unique<DestroyMultiUseTransitionF::Functor>(&JEditorObjectDestroyInterface::DestroyMultiUseTransition, this);
			destroySingleUseTransitionF = std::make_unique<DestroySingleUseTransitionF::Functor>(&JEditorObjectDestroyInterface::DestroySingleUseTransition, this);
			destroyMultiF = std::make_unique<DestroyMultiF::Functor>(&JEditorObjectDestroyInterface::DestroyMulti, this);
			destroySingleF = std::make_unique<DestroySingleF::Functor>(&JEditorObjectDestroyInterface::DestroySingle, this);
		}
		JEditorObjectDestroyInterface::DestroyMultiUseTransitionF::Functor* JEditorObjectDestroyInterface::GetMultiUseTransitionFunctor()
		{
			return destroyMultiUseTransitionF.get();
		}
		JEditorObjectDestroyInterface::DestroySingleUseTransitionF::Functor* JEditorObjectDestroyInterface::GetSingleUseTransitionFunctor()
		{
			return destroySingleUseTransitionF.get();
		}
		JEditorObjectDestroyInterface::DestroyMultiF::Functor* JEditorObjectDestroyInterface::GetMultiFunctor()
		{
			return destroyMultiF.get();
		}
		JEditorObjectDestroyInterface::DestroySingleF::Functor* JEditorObjectDestroyInterface::GetSingleFunctor()
		{
			return destroySingleF.get();
		} 
		void JEditorObjectDestroyInterface::RegisterDestroyPreProcess(PreProcessF::Ptr ptr)
		{
			preProcessF = std::make_unique<PreProcessF::Functor>(ptr);
		}
		void JEditorObjectDestroyInterface::RegisterDestroyPostProcess(PostProcessF::Ptr ptr)
		{
			postProcessF = std::make_unique<PostProcessF::Functor>(ptr);
		}
		void JEditorObjectDestroyInterface::DestroyMultiUseTransition(DataHandleStructure& dS,
			std::vector<Core::JDataHandle>& dHVec,
			const std::vector<size_t> guidVec,
			const bool useTransition,
			const JEditorCreationHint creationHint)
		{ 
			uint failCount = 0;
			JEditorWindow* editorWnd = creationHint.editorWnd;
			for (const auto& data : guidVec)
			{
				Core::JDataHandle newHandle = dS.CreateInvalidHandle();
				DestroySingleUseTransition(dS, newHandle, data, useTransition, creationHint);
				if (newHandle.IsValid())
					dS.PushValidHandle(newHandle, dHVec);
				else
				{
					JEditorTransition::Instance().Log("Destruction Multi Fail", "Count: " + std::to_string(failCount));
					++failCount;
				}
			}
		}
		void JEditorObjectDestroyInterface::DestroySingleUseTransition(DataHandleStructure& dS,
			Core::JDataHandle& dH,
			const size_t guid,
			const bool useTransition,
			JEditorCreationHint creationHint)
		{
			Core::JIdentifier* rawPtr = Core::SearchRawPtr<Core::JIdentifier>(Core::JIdentifier::StaticTypeInfo(), guid);
			if (rawPtr == nullptr)
			{
				JEditorTransition::Instance().Log("Destruction Fail invalid guid typeName: " + Core::JIdentifier::StaticTypeInfo().Name());
				return;
			}
			DestroyPreProccess(rawPtr, useTransition, creationHint);
			if (useTransition)
			{
				Core::JDataHandle newHandle = dS.Add(ReleaseInterface::ReleaseInstance(rawPtr));
				dS.TransitionHandle(newHandle, dH);
				if(!dH.IsValid())
					JEditorTransition::Instance().Log(L"Destruction transition handle fail", rawPtr->GetNameWithType());
			}
			else
			{
				Core::JTypeInfo& typeInfo = rawPtr->GetTypeInfo();
				Core::JIdentifier::BeginDestroy(rawPtr);  
			}
			JEditorTransition::Instance().Log(L"Destruction success", rawPtr->GetNameWithType());
			DestroyPostProccess(guid, useTransition, creationHint); 
		}
		void JEditorObjectDestroyInterface::DestroyMulti(const std::vector<size_t> guidVec,
			const bool useTransition,
			JEditorCreationHint creationHint)
		{
			uint failCount = 0;
			JEditorWindow* editorWnd = creationHint.editorWnd;
			for (const auto& data : guidVec)
			{ 
				DestroySingle(data, useTransition, creationHint);
				if (!DestroySingle(data, useTransition, creationHint))
				{
					JEditorTransition::Instance().Log("Destruction Fail", "Count: " + std::to_string(failCount));
					++failCount;
				}
			}
		}
		bool JEditorObjectDestroyInterface::DestroySingle(const size_t guid,
			const bool useTransition,
			JEditorCreationHint creationHint)
		{
			Core::JIdentifier* rawPtr = Core::SearchRawPtr<Core::JIdentifier>(Core::JIdentifier::StaticTypeInfo(), guid);
			if (rawPtr == nullptr)
				return false;

			DestroyPreProccess(rawPtr, useTransition, creationHint);
			Core::JTypeInfo& typeInfo = rawPtr->GetTypeInfo();
			Core::JIdentifier::BeginDestroy(rawPtr);
			DestroyPostProccess(guid, useTransition, creationHint);

			return true;
		}
		void JEditorObjectDestroyInterface::DestroyPreProccess(Core::JIdentifier* rawPtr,
			const bool useTransition,
			JEditorCreationHint creationHint)
		{ 
			JEditorWindow* editorWnd = creationHint.editorWnd;
			if (editorWnd != nullptr && preProcessF != nullptr)
				(*preProcessF)(editorWnd, rawPtr);
 
			if (creationHint.canSetModBit)
			{
				if (creationHint.isSetOpenDataBit)
					SetModifiedBit(Core::GetUserPtr<Core::JIdentifier>(creationHint.openDataHint), true);
				if (creationHint.isSetOwnerDataBit)
					SetModifiedBit(Core::GetUserPtr<Core::JIdentifier>(creationHint.ownerDataHint), true);
				if (creationHint.isSetTargetDataBit)
					SetModifiedBit(Core::GetUserPtr(rawPtr), true);
			}

			//destroy되는 대상을 selecte map에서 erase
			if (editorWnd != nullptr && creationHint.notifyPtr != nullptr)
			{
				JEditorPopSelectObjectEvStruct deselectEv{editorWnd->GetOwnerPageType(), Core::GetUserPtr(rawPtr), JEditorEvStruct::RANGE::ALL };
				(editorWnd->*creationHint.notifyPtr)(*JEditorEvent::EvInterface(), editorWnd->GetGuid(), J_EDITOR_EVENT::POP_SELECT_OBJECT, &deselectEv);
			}		 
		}
		void JEditorObjectDestroyInterface::DestroyPostProccess(const size_t guid,
			const bool useTransition,
			JEditorCreationHint creationHint)
		{
			JEditorWindow* editorWnd = creationHint.editorWnd;
			if (editorWnd != nullptr && postProcessF != nullptr)
				(*postProcessF)(editorWnd);
		}

		JEditorObjectUndoDestroyInterface::JEditorObjectUndoDestroyInterface()
		{
			undoDestroyF = std::make_unique<UndoDestroyF::Functor>(&JEditorObjectUndoDestroyInterface::UndoDestroy, this);
		} 
		JEditorObjectUndoDestroyInterface::UndoDestroyF::Functor* JEditorObjectUndoDestroyInterface::GetFunctor()
		{
			return undoDestroyF.get();
		}
		void JEditorObjectUndoDestroyInterface::RegisterUndoDestroyPreProcess(PreProcessF::Ptr ptr)
		{
			preProcessF = std::make_unique<PreProcessF::Functor>(ptr);
		}
		void JEditorObjectUndoDestroyInterface::RegisterUndoDestroyPostProcess(PostProcessF::Ptr ptr)
		{
			postProcessF = std::make_unique<PostProcessF::Functor>(ptr);
		}
		void JEditorObjectUndoDestroyInterface::UndoDestroy(DataHandleStructure& dS, std::vector<Core::JDataHandle>& dHVec, const JEditorCreationHint creationHint)
		{
			JEditorWindow* editorWnd = creationHint.editorWnd;
			const uint count = (uint)dHVec.size();
			for (uint i = 0; i < count; ++i)
			{
				auto data = dS.Release(dHVec[i]);
				if (data.IsValid())
				{
					if (editorWnd != nullptr && preProcessF != nullptr)
						(*preProcessF)(editorWnd);
					Core::JIdentifier* ptr = data.Get();
					ReleaseInterface::RestoreInstance(std::move(data));
					JEditorTransition::Instance().Log(L"Cancel destruction success ", ptr->GetNameWithType());

					if (creationHint.canSetModBit)
					{
						if (creationHint.isSetOpenDataBit)
							SetModifiedBit(Core::GetUserPtr<Core::JIdentifier>(creationHint.openDataHint), true);
						if (creationHint.isSetOwnerDataBit)
							SetModifiedBit(Core::GetUserPtr<Core::JIdentifier>(creationHint.ownerDataHint), true);
						if (creationHint.isSetTargetDataBit)
							SetModifiedBit(Core::GetUserPtr(ptr), true);
					}
					
					//undo 되는 target을 select
					if (editorWnd != nullptr && creationHint.notifyPtr != nullptr)
					{ 
						JEditorPushSelectObjectEvStruct selectEv{editorWnd->GetOwnerPageType(), editorWnd->GetWindowType(), Core::GetUserPtr(ptr), JEditorEvStruct::RANGE::ALL };
						(editorWnd->*creationHint.notifyPtr)(*JEditorEvent::EvInterface(), editorWnd->GetGuid(), J_EDITOR_EVENT::PUSH_SELECT_OBJECT, &selectEv);
					} 

					if (editorWnd != nullptr && postProcessF != nullptr)
						(*postProcessF)(editorWnd, ptr);
				}
				else
					JEditorTransition::Instance().Log("Cancel destruction fail", "Invalid release data");
			}
			dHVec.clear();
		}

		JOwnerPtr<Core::JIdentifier> JEditorObjectReleaseInterface::ReleaseInstance(Core::JIdentifier* ptr)
		{ 
			return JOwnerPtr<Core::JIdentifier>::ConvertChild(Core::JIdentifierPrivate::ReleaseInterface::ReleaseInstance(ptr));
		}
		bool JEditorObjectReleaseInterface::RestoreInstance(JOwnerPtr<Core::JIdentifier>&& instance)
		{
			return Core::JIdentifierPrivate::ReleaseInterface::RestoreInstance(std::move(instance));
		}

		JEditorObjectDestroyInterface* JEditorDestructionRequestor::GetDestruectionInterface()
		{
			return &destructionInterface;
		}
		JEditorObjectUndoDestroyInterface* JEditorDestructionRequestor::GetUndoDestruectionInterface()
		{
			return &undoDestructionInterface;
		}
		void JEditorDestructionRequestor::RequestDestroyObject(DataHandleStructure& dS,
			const bool useTransition,
			JEditorCreationHint creationHint,
			const std::vector<JUserPtr<Core::JIdentifier>>& idenVec,
			const JEditorRequestHint requestHint)
		{
			if (!requestHint.IsValid() || !creationHint.IsValid())
				return;

			std::vector<size_t> guidVec;
			for (const auto& data : idenVec)
			{
				if (data.IsValid())
					guidVec.push_back(data->GetGuid());
			}
			if (guidVec.size() == 0)
				return;

			if (useTransition)
			{
				auto doBind = std::make_unique<DestroyMultiUseTransitionF::Bind>(*destructionInterface.GetMultiUseTransitionFunctor(),
					Core::empty, Core::empty,
					std::move(guidVec),
					true,
					std::move(creationHint));
				auto undoBind = std::make_unique<UndoDestroyF::Bind>(*undoDestructionInterface.GetFunctor(),
					Core::empty, Core::empty,
					std::move(creationHint));

				size_t evGuid;
				using DestroyTEv = JEditorTCreateBindFuncEvStruct<DataHandleStructure, DestroyMultiUseTransitionF::Bind, UndoDestroyF::Bind, true>;
				auto evStruct = JEditorEvent::RegisterEvStruct(std::make_unique<DestroyTEv>
					("Destroy group task", "use transition", creationHint.editorWnd->GetOwnerPageType(), std::move(doBind), std::move(undoBind), dS), evGuid);
				(*requestHint.clearTaskFunctor)(std::vector<size_t>{evGuid});
				(creationHint.editorWnd->*requestHint.addEventPtr)(*JEditorEvent::EvInterface(), creationHint.editorWnd->GetGuid(), J_EDITOR_EVENT::T_BIND_FUNC, evStruct);
			}
			else
			{
				auto doBind = std::make_unique<DestroyMultiF::CompletelyBind>(*destructionInterface.GetMultiFunctor(),
					std::move(guidVec),
					true,
					std::move(creationHint));
				auto evStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorBindFuncEvStruct>
					(std::move(doBind), creationHint.editorWnd->GetOwnerPageType()));
				static_cast<JEditorBindFuncEvStruct*>(evStruct)->SetLog(std::make_unique < Core::JLogBase>("Destroy task", "don't use transition"));
				(creationHint.editorWnd->*requestHint.addEventPtr)(*JEditorEvent::EvInterface(), creationHint.editorWnd->GetGuid(), J_EDITOR_EVENT::BIND_FUNC, evStruct);
			}
		}
	}
}