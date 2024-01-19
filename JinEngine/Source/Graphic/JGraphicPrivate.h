#pragma once
#include"JGraphicConstants.h"
#include"GraphicResource/JGraphicResourceType.h"  
#include"FrameResource/JFrameResourceEnum.h"   
#include"Culling/JCullingType.h"
#include"../Core/JCoreEssential.h"  
#include<string>
 
namespace JinEngine
{
	struct JGraphicShaderInitData;
	struct JComputeShaderInitData;
	class JGraphicShaderDataHolderBase;
	class JComputeShaderDataHolderBase; 
	class JShader;  
 
	class JMain;
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
		class JCsmTargetInterface;
		class JCsmHandlerInterface;
		class JFrameUpdateData; 
		class JGraphicAdapter;
		class JGuiBackendDataAdapter;
		class JGuiBackendInterface;
		struct JGuiInitData; 
		struct JMipmapGenerationDesc;

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
			private:
				static JOwnerPtr<JGraphicShaderDataHolderBase> StuffGraphicShaderPso(const JGraphicShaderInitData& shaderData);
				static JOwnerPtr<JComputeShaderDataHolderBase> StuffComputeShaderPso(const JComputeShaderInitData& shaderData);
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