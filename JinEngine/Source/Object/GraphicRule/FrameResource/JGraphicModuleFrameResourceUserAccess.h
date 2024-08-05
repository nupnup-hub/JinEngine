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
#include"JGraphicModuleFrameResourceType.h"
#include"../../../Core/Func/Functor/JFunctor.h"

namespace JinEngine
{
	class JFrameUpdateUserInterface;
	class JObject;
	struct JFrameDirtyListener
	{
	public:
		using GetListenerF = Core::JSFunctorType<JFrameUpdateUserInterface*, JUserPtr<JObject>>::Functor;
		using GetListenerB = std::unique_ptr<Core::JBindHandle<GetListenerF, JUserPtr<JObject>>>;			// obj, index
	public:
		GetListenerB getFrameUserBind;
		size_t guid;
	};  

	using JFrameObjectUpdateF = Core::JSFunctorType<void, JUserPtr<JObject>>;
	using JFrameObjectUpdateB = std::unique_ptr<JFrameObjectUpdateF::CompletelyBind>;
 
	class JFrameUpdateUserInterface
	{  
	public:
		JFrameUpdateUserInterface() = default;
		virtual ~JFrameUpdateUserInterface() = default;
	public:
		virtual int GetNumber(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept = 0;
		virtual int GetFrameIndex(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept = 0;
		virtual int GetFrameIndexSize(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept = 0; 
		virtual int GetLocalFrameCount(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept = 0; 
	public:
		virtual void SetFrameDirty()noexcept = 0;
	public:
		virtual bool IsDirted()const noexcept = 0;
		virtual bool HasSpace(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept = 0;
	public:
		virtual void OffFrameDirty()noexcept = 0;
	public:
		virtual bool TryRegisterDirtyListener(const JFrameDirtyListener& listener) = 0;
		virtual bool TryDeRegisterDirtyListener(const size_t guid) = 0;
	public:
		virtual bool RegisterObjectUpdateB(JFrameObjectUpdateB&& bind) = 0;
		virtual bool DeRegisterObjectUpdateB() = 0;
	};
}
