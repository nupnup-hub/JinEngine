/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/

#pragma once
#include"../../Core/JCoreEssential.h"

namespace JinEngine
{
	//Rule: Engined defined class 대상.

#define ALLOC_GRAPHIC_MODULE_DATA(typeName, _graphicData, _thisPointer)																									\
	auto notifyReAllocPtr = [](const JFastPtr<JGraphicModuleManagedDataFrame>& data, JObject* object)					\
	{																																\
		static_cast<typeName*>(object)->impl->graphicData = data;																	\
	};																																\
	_graphicData = GraphicModuleInterface()->Allocate(JGraphicModuleManagedDataCreationDesc(_thisPointer, notifyReAllocPtr))	\
 

#define INTERFACE_ALLOC_GRAPHIC_MODULE_DATA() ALLOC_GRAPHIC_MODULE_DATA(ThisType, impl->graphicData, impl->thisPointer)
#define IMPL_ALLOC_GRAPHIC_MODULE_DATA() ALLOC_GRAPHIC_MODULE_DATA(ThisInterfaceType, graphicData, thisPointer)


#define DEALLOC_GRAPHIC_MODULE_DATA()  GraphicModuleInterface()->DeAllocate(impl->graphicData);
#define IMPL_DEALLOC_GRAPHIC_MODULE_DATA() GraphicModuleInterface()->DeAllocate(graphicData);


#define IMPL_REGISTER_TRANFORM_FRAME_DRITY_LISTENER()							\
																				\
		auto listenerLam = [](JUserPtr<JObject> obj)							\
		{	 																								\
			return static_cast<ThisInterfaceType*>(obj.Get())->ModuleManagedData()->GetFrameUpdateUserInterface();\
		};																\
																		\
		JFrameDirtyListener listener;									\
		listener.getFrameUserBind = Core::UniqueBind(std::make_unique<JFrameDirtyListener::GetListenerF>(listenerLam), JUserPtr<JObject>(thisPointer));	\
		listener.guid = thisPointer->GetGuid();																											\
																																						\
		auto transform = thisPointer->GetOwner()->GetTransform();																						\
		if(transform == nullptr || !transform->IsActivated())																														\
			return;																																		\
																																						\
		auto tFInterface = transform->ModuleManagedData()->GetFrameUpdateUserInterface();																\
		tFInterface->TryRegisterDirtyListener(std::move(listener));																						\


#define IMPL_DEREGISTER_TRANFORM_FRAME_DRITY_LISTENER()							\
																				\
		auto transform = thisPointer->GetOwner()->GetTransform();				\
		if (transform == nullptr || !transform->IsActivated())					\
			return;																\
																				\
		auto tFInterface = transform->ModuleManagedData()->GetFrameUpdateUserInterface();\
		tFInterface->TryDeRegisterDirtyListener(thisPointer->GetGuid());					\


	//일반적으로 Always -> Hot 순으로 Update를 실시한다
	//필요하다면 JTransition처럼 Pre - Post 접두사를 붙은 함수를 추가하자. 
	//Rule: function name is HotUpdate
#define IMPL_REGISTER_FRAME_UPDATE_ACTION_HOT()										\
																					\
		auto hotUpdateLam = [](JUserPtr<JObject> obj)								\
		{																			\
			static_cast<ThisInterfaceType*>(obj.Get())->impl->HotUpdate();			\
		};																			\
																					\
		auto bind = JFrameObjectUpdateF::CreateCompletelyBind(hotUpdateLam, JUserPtr<JObject>(thisPointer));	\
		graphicData->GetFrameUpdateUserInterface()->RegisterObjectUpdateB(std::move(bind));					\
																											\


		//Rule: function name is AlwaysUpdate
#define IMPL_REGISTER_FRAME_UPDATE_ACTION_ALWAYS()									\
																					\
		auto alwaysUpdateLam = [](JUserPtr<JObject> obj)							\
		{																			\
			static_cast<ThisInterfaceType*>(obj.Get())->impl->AlwaysUpdate();		\
		};																			\
																					\
		auto bind = JFrameObjectUpdateF::CreateCompletelyBind(alwaysUpdateLam, JUserPtr<JObject>(thisPointer));	\
		graphicData->GetFrameUpdateUserInterface()->RegisterObjectUpdateB(nullptr, std::move(bind));					\
																											\


#define IMPL_REGISTER_FRAME_UPDATE_ACTION()											\
																					\
		auto hotUpdateLam = [](JUserPtr<JObject> obj)								\
		{																			\
			static_cast<ThisInterfaceType*>(obj.Get())->impl->HotUpdate();			\
		};																			\
		auto alwaysUpdateLam = [](JUserPtr<JObject> obj)							\
		{																			\
			static_cast<ThisInterfaceType*>(obj.Get())->impl->AlwaysUpdate();		\
		};																			\
																					\
		auto hotBind = JFrameObjectUpdateF::CreateCompletelyBind(hotUpdateLam, JUserPtr<JObject>(thisPointer));					\
		auto alwaysBind = JFrameObjectUpdateF::CreateCompletelyBind(alwaysUpdateLam, JUserPtr<JObject>(thisPointer));			\
		graphicData->GetFrameUpdateUserInterface()->RegisterObjectUpdateB(std::move(hotBind), std::move(alwaysBind));		\
																																\

 

#define  IMPL_DEREGISTER_FRAME_UPDATE_ACTION() graphicData->GetFrameUpdateUserInterface()->DeRegisterObjectUpdateB();
}
