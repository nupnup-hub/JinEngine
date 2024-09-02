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
#include"../../Core/Pointer/JOwnerPtr.h"
#include"Csm/JGraphicModuleCsmUserAccess.h"
#include"Culling/JGraphicModuleCullingUserAccess.h"
#include"FrameResource/JGraphicModuleFrameResourceUserAccess.h"
#include"GpuAccelerator/JGraphicModuleGpuAcceleratorUserAccess.h"
#include"GraphicResource/JGraphicModuleTextureResourceUserAccess.h"

namespace JinEngine
{
	class JObject;
	/**
	* Object private data used by graphic progress
	*/

	struct JGraphicModuleManagedDataCreationDesc;
	class JGraphicModuleManagedDataFrame
	{
		REGISTER_CLASS_USE_ALLOCATOR(JGraphicModuleManagedDataFrame)
	public:
		using NotifyReAllocPtr = Core::JSFunctorType<void, const Core::JFastPtr<JGraphicModuleManagedDataFrame>&, JObject*>::Ptr;
	private:
		JWeakPtr<JObject> object;
		NotifyReAllocPtr notifyReAllocPtr = nullptr;
	public:
		JGraphicModuleManagedDataFrame(const JGraphicModuleManagedDataCreationDesc& desc);
		virtual ~JGraphicModuleManagedDataFrame() = default;
	public:
		JWeakPtr<JObject> Object()const noexcept;
	public:
		virtual JCsmHandleUserInterface* GetCsmHandleUserInterface()const noexcept = 0;
		virtual JCsmTargetUserInterface* GetCsmTargetUserInterface()const noexcept = 0;
		virtual JCullingUserInterface* GetCullingUserInterface()const noexcept = 0;
		virtual JFrameUpdateUserInterface* GetFrameUpdateUserInterface()const noexcept = 0;
		virtual JGpuAcceleratorUserInterface* GetGpuAcceleratorUserInterface()const noexcept = 0;
		virtual JGraphicResourceUserInterface* GetGraphicResourceUserInterface()const noexcept = 0;
	public:
		bool CanAccessCsmHandle()const noexcept;
		bool CanAccessCsmTarget()const noexcept;
		bool CanAccessCullingResource()const noexcept;
		bool CanAccessFrameResource()const noexcept;
		bool CanAccessGpuAccelerator()const noexcept;
		bool CanAccessGraphicResource()const noexcept;
	protected:
		static void NotifyReAlloc(const Core::JFastPtr<JGraphicModuleManagedDataFrame>& newData);
	};

	struct JGraphicModuleManagedDataCreationDesc
	{
	public:
		using NotifyReAllocPtr = JGraphicModuleManagedDataFrame::NotifyReAllocPtr;
	public:
		JWeakPtr<JObject> object;
		NotifyReAllocPtr notifyReAllocPtr = nullptr;
	public:
		JGraphicModuleManagedDataCreationDesc(const JWeakPtr<JObject>& object, NotifyReAllocPtr notifyReAllocPtr);
	};

}