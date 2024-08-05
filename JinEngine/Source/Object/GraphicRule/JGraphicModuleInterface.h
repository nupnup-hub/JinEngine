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
#include"Csm/JGraphicModuleCsmType.h"
#include"Culling/JGraphicModuleCullingType.h"
#include"GpuAccelerator/JGraphicModuleGpuAcceleratorType.h"
#include"FrameResource/JGraphicModuleFrameResourceType.h"
#include"GraphicResource/JGraphicModuleTextureResourceType.h"
#include"JGraphicModuleManagedDataFrame.h"
#include"JGraphicModuleType.h"

namespace JinEngine
{
	class JObject;
	namespace Rule
	{ 
		class JGraphicModuleInterface
		{
		public:
			virtual JUserPtr<JGraphicModuleManagedDataFrame> Allocate(const JUserPtr<JObject>& object) = 0;
			virtual void DeAllocate(JUserPtr<JGraphicModuleManagedDataFrame>& data) = 0;
		public:
			virtual bool RegisterScene(JGraphicModuleManagedDataFrame* data, const JGraphicSceneRegisterDesc& desc) = 0;
			virtual bool DeRegisterScene(JGraphicModuleManagedDataFrame* data) = 0;
		public:
			//Owner scene had to register before request
			virtual bool RequestExecutableGraphicFeature(JGraphicModuleManagedDataFrame* data, const JGraphicRequestCreationDesc& desc) = 0;
			virtual bool CancelExecutableGraphicFeature(JGraphicModuleManagedDataFrame* data, J_GRAPHIC_REQUEST_TYPE type) = 0;
		public:
			virtual bool CreateGraphicResource(JGraphicModuleManagedDataFrame* data, const JGraphicResourceCreationDesc& desc, const uint count = 1) = 0;
			virtual bool DestroyGraphicResource(JGraphicModuleManagedDataFrame* data, const JGraphicResourceTypeSet& typeSet, const uint count = 1) = 0;
			virtual bool DestroyGraphicResourceByIndex(JGraphicModuleManagedDataFrame* data, const JGraphicResourceTypeSet& typeSet, const uint index, const uint count = 1) = 0;
			virtual bool DestroyAllGraphicsResourcesOfType(JGraphicModuleManagedDataFrame* data, const J_GRAPHIC_RESOURCE_TYPE type) = 0;
			virtual bool DestroyAllGraphicsResources(JGraphicModuleManagedDataFrame* data) = 0; 
		public:
			virtual bool CreateGraphicResourceOption(JGraphicModuleManagedDataFrame* data, const JGraphicResourceCreationDesc& desc) = 0;
			virtual bool DestroyGraphicResourceOption(JGraphicModuleManagedDataFrame* data, const JGraphicResourceTypeSet& type) = 0;
			virtual bool DestroyGraphicAllResourceOption(JGraphicModuleManagedDataFrame* data, const JGraphicResourceTypeSet& typeSet) = 0;
		public:
			virtual bool CreateFrameUploadData(JGraphicModuleManagedDataFrame* data, const JFrameUploadDataCreationDesc& desc) = 0;
			virtual bool DestroyFrameUploadData(JGraphicModuleManagedDataFrame* data, const J_FRAME_RESOURCE_UPLOAD_TYPE type) = 0;
		public:
			virtual bool CreateCullingData(JGraphicModuleManagedDataFrame* data, const JCullingTypeSet& typeSet) = 0;
			virtual bool DestroyCullingData(JGraphicModuleManagedDataFrame* data, const JCullingTypeSet& typeSet) = 0;
			virtual bool DestroyAllCullingDataOfType(JGraphicModuleManagedDataFrame* data, const J_CULLING_TYPE type) = 0;
			virtual bool DestroyAllCullingData(JGraphicModuleManagedDataFrame* data) = 0;
		public:
			virtual bool CreateGpuAccelerator(JGraphicModuleManagedDataFrame* data, const JGpuAcceleratorBuildDesc& desc) = 0;
			virtual bool DestroyGpuAccelerator(JGraphicModuleManagedDataFrame* data) = 0;
		public:
			virtual bool CreateCsmHandler(JGraphicModuleManagedDataFrame* data, const JCsmHandleCreationDesc& desc) = 0;
			virtual bool CreateCsmTarget(JGraphicModuleManagedDataFrame* data, const JCsmTargetCreationDesc& desc) = 0;
			virtual bool DestroyCsmHandler(JGraphicModuleManagedDataFrame* data) = 0;
			virtual bool DestroyCsmTarget(JGraphicModuleManagedDataFrame* data) = 0;
		};
	}
}