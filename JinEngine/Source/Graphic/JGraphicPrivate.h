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
#include"JGraphicConstants.h" 
#include"GraphicResource/JGraphicResourceType.h"  
#include"FrameResource/JFrameResourceEnum.h"   
#include"Culling/JCullingType.h"
#include"Accelerator/JGpuAcceleratorType.h"  
 
namespace JinEngine
{ 
	class JShader;  
	class JMain;
	class JComponent;
	struct JGraphicShaderInitData;
	struct JComputeShaderInitData;
	namespace Core
	{
		class JDataHandle;
		class JGraphicException;
		struct JStaticMeshVertex;
		struct JSkinnedMeshVertex;
		struct J1BytePosVertex;
	}
	namespace Editor
	{ 
		class JGraphicResourceWatcher;
	}
	namespace Graphic
	{
		class JGraphicResourceManager;
		class JGraphicResourceInfo;
		class JGraphicResourceInterface;
		class JGraphicResourceUserInterface; 
		class JCullingInfo;
		class JCullingInterface;
		class JGpuAcceleratorInfo;
		class JGpuAcceleratorInterface;
		class JCsmTargetInterface;
		class JCsmHandlerInterface;
		class JFrameUpdateData; 
		class JGraphicAdapter;
		class JGuiBackendDataAdapter;
		class JGuiBackendInterface;
		class JShaderDataHolder;
		struct JGuiInitData; 
		struct JMipmapGenerationDesc;
		struct JConvertColorDesc;
		class JGraphicPrivate
		{
		public:
			class ResourceInterface
			{
			private:
				friend class JShader;
				friend class JGraphicResourceInfo;
				friend class JGraphicResourceInterface; 
				friend class JCullingInterface;			//for create occ resource
			private:
				//private name is for debug
				static JUserPtr<JGraphicResourceInfo> CreateResource(const JGraphicResourceCreationDesc& createDesc, const J_GRAPHIC_RESOURCE_TYPE rType);
				static bool CreateOption(JUserPtr<JGraphicResourceInfo>& info, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType);
			private:
				static bool DestroyGraphicTextureResource(JGraphicResourceInfo* info);
				static bool DestroyGraphicOption(JUserPtr<JGraphicResourceInfo>& info, const J_GRAPHIC_RESOURCE_OPTION_TYPE optype);
			private:
				static bool SetMipmap(const JUserPtr<JGraphicResourceInfo>& info, JTextureCreationDesc createDesc);
				static bool SetTextureDetail(const JUserPtr<JGraphicResourceInfo>& info, const JConvertColorDesc& convertDesc);
			private:
				static JOwnerPtr<JShaderDataHolder> StuffGraphicShaderPso(const JGraphicShaderInitData& shaderData);
				static JOwnerPtr<JShaderDataHolder> StuffComputeShaderPso(const JComputeShaderInitData& shaderData);
			private:
				//Debug
				static bool MipmapBindForDebug(const JUserPtr<JGraphicResourceInfo>& info, _Out_ std::vector<ResourceHandle>& gpuHandle, _Out_ std::vector<Core::JDataHandle>& dataHandle);
				static void ClearMipmapBind( _In_ std::vector<Core::JDataHandle>& dataHandle);
			};
			class CullingInterface
			{
			private:
				friend class JCullingInfo;
				friend class JCullingInterface;
			private:
				static JUserPtr<JCullingInfo> CreateFrsutumCullingResultBuffer(const J_CULLING_TARGET target, const bool useGpu = false);
				static JUserPtr<JCullingInfo> CreateHzbOccCullingResultBuffer();
				static JUserPtr<JCullingInfo> CreateHdOccCullingResultBuffer(); 
			private:
				static bool DestroyCullignData(JCullingInfo* cullingInfo);
			};
			class AcceleratorInterface
			{
			private:
				friend class JGpuAcceleratorInterface;
			private:
				static JUserPtr<JGpuAcceleratorInfo> CreateGpuAccelerator(const JGpuAcceleratorBuildDesc& desc);
			private:
				static bool DestroyGpuAccelerator(JGpuAcceleratorInfo* info);
			private:
				static void UpdateTransform(JGpuAcceleratorInfo* info, const JUserPtr<JComponent>& comp);
				static void AddComponent(JGpuAcceleratorInfo* info, const JUserPtr<JComponent>& comp);
				static void RemoveComponent(JGpuAcceleratorInfo* info, const JUserPtr<JComponent>& comp);
			};
			class CsmInterface
			{
			private:
				friend class JCsmTargetInterface;
				friend class JCsmHandlerInterface;
			private:
				static bool RegisterHandler(JCsmHandlerInterface* handler);
				static bool DeRegisterHandler(JCsmHandlerInterface* handler);
				static bool RegisterTarget(JCsmTargetInterface* target);
				static bool DeRegisterTarget(JCsmTargetInterface* target);
			}; 
			class DebugInterface
			{
			private:
				friend class Editor::JGraphicResourceWatcher;
			private:
				static JGraphicResourceManager* GetGraphicResourceManager()noexcept;
			}; 
			class MainAccess
			{
			private:
				friend class JMain;
			private:
				//guiManagerInterface is deleted when app close
				static void Initialize(std::unique_ptr<JGraphicAdapter>&& adapter,
					std::unique_ptr<JGuiBackendDataAdapter>&& guiAdapter,
					JGuiBackendInterface* guiBackendInterface);
				static void Clear();
			private:
				static std::unique_ptr<JGuiInitData> GetGuiInitData()noexcept;
			private:
				static void UpdateWait();  
				static void UpdateFrame();
			private:  
				static void Draw(const bool allowDrawScene);
			private:
				static void FlushCommandQueue();
			private:
				static void WriteLastRsTexture();
			};
		};
	}
}