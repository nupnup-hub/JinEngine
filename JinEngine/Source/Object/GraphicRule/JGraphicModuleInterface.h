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
#include"Shader/JGraphicModuleShaderHolder.h"
#include"JGraphicModuleManagedDataFrame.h"
#include"JGraphicModuleType.h"

namespace JinEngine
{
	class JObject;
	namespace Rule
	{
		class JGraphicModuleInterface
		{
			//Creation
		public:
			/*
			* timing
			* Component는 모두 Activate와 DeActivate 호출시에 Allocate, DeAllocate을 수행한다.
			* Resource는 RAII을 준수해서 할당될시 즉시 사용가능한 상태가되며 
			* Activate가 아닌 Initialize 단계에서 Import나 Load등의 함수호출로 인해 GraphicModule에 접근이 
			* 필요할 수 있으므로 각 Resource에 구현에 맞게 호출타이밍을 조절하도록하자.
			*/
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
			virtual bool DestroyGraphicResource(JGraphicModuleManagedDataFrame* data, const JGraphicResourceTypeSet& typeSet, const uint localIndex = 0, const uint count = 1) = 0;
			virtual bool DestroyAllGraphicsResourcesOfType(JGraphicModuleManagedDataFrame* data, const J_GRAPHIC_RESOURCE_TYPE type) = 0;
			virtual bool DestroyAllGraphicsResources(JGraphicModuleManagedDataFrame* data) = 0;
		public:
			virtual bool CreateGraphicResourceOption(JGraphicModuleManagedDataFrame* data, const JGraphicResourceTypeSet& typeSet, const uint localIndex = 0) = 0;
			virtual bool DestroyGraphicResourceOption(JGraphicModuleManagedDataFrame* data, const JGraphicResourceTypeSet& typeSet, const uint localIndex = 0) = 0;
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
			virtual bool CreateCsmHandler(JGraphicModuleManagedDataFrame* data, JCsmHandleCreationDesc& desc) = 0;
			virtual bool CreateCsmTarget(JGraphicModuleManagedDataFrame* data, JCsmTargetCreationDesc& desc) = 0;
			virtual bool DestroyCsmHandler(JGraphicModuleManagedDataFrame* data) = 0;
			virtual bool DestroyCsmTarget(JGraphicModuleManagedDataFrame* data) = 0;
		public:
			//Shader isn't use interface 
			//because less features to be provided as an interface 
			virtual JOwnerPtr<JShaderDataHolder> CreateGraphicShader(const JGraphicShaderInitData& initData) = 0;
			virtual JOwnerPtr<JShaderDataHolder> CreateComputeShader(const JComputeShaderInitData& initData) = 0;
		public:
			//GraphicResource feature
			/**
			* @brief non mipmap일 경우와 graphic api defined로 mipmap을 변경하고 싶은경우 해당하는 resource를 다시 만들 필요가있으며
			* 그밖에 경우에만 desc에 맞는 새로운 mipmap을 생성
			*/
			virtual bool SetMipmap(JGraphicModuleManagedDataFrame* data, const JGraphicResourceTypeSet& typeSet, const uint dataIndex, JTextureCreationDesc& createDesc) = 0;
			/**
			* @brief color curve를 조정하며 현재는 reverseY만 추가된상태.
			*/
			virtual bool SetTextureDetail(JGraphicModuleManagedDataFrame* data, const JGraphicResourceTypeSet& typeSet, const uint dataIndex, const JConvertColorDesc& convertDesc) = 0;
			//Debug
			/**
			* @brief for debug texture mipmap
			*/
			virtual bool TryFirstGraphicResourceMipmapBind(JGraphicModuleManagedDataFrame* data, _Inout_ std::vector<ResourceHandle>& gpuHandle, _Inout_ std::vector<Core::JDataHandle>& dataHandle)const = 0;
			virtual void ClearFirstGraphicResourceMipmapBind(JGraphicModuleManagedDataFrame* data, _Inout_ std::vector<Core::JDataHandle>& dataHandle) = 0;
		public:
			//GpuAccelerator feature
			virtual void UpdateTransform(JGpuAcceleratorUserInterface* gUser, const JUserPtr<JComponent>& comp)noexcept = 0;
			virtual void AddComponent(JGpuAcceleratorUserInterface* gUser, const JUserPtr<JComponent>& newComp)noexcept = 0;
			virtual void RemoveComponent(JGpuAcceleratorUserInterface* gUser, const JUserPtr<JComponent>& comp)noexcept = 0;
		public:
			//Check graphic feature
			virtual bool IsActivatedDeferredRendering()const noexcept = 0;
			virtual bool IsActivatedRaytracing()const noexcept = 0;
			virtual bool IsActivatedRaytracingGI()const noexcept = 0;
			virtual bool IsActivatedPostprocessing()const noexcept = 0;
		};
	}
}