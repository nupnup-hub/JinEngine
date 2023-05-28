#pragma once
#include"GraphicResource/JGraphicResourceType.h"
#include"Upload/JUploadType.h"
#include"../Object/Resource/Shader/JExtraPsoType.h"
#include"../Object/Resource/Shader/JShaderFunctionEnum.h"
#include"../Object/Resource/Shader/JShaderGraphicPsoCondition.h"
#include"../Core/JDataType.h"
#include<string>

struct ID3D12Device;
struct ID3D12CommandQueue;
struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;
struct CD3DX12_CPU_DESCRIPTOR_HANDLE;
struct CD3DX12_GPU_DESCRIPTOR_HANDLE;
struct ID3D12Resource;

namespace Microsoft
{
	namespace WRL
	{
		template<typename T>class ComPtr;
	}
}

namespace JinEngine
{
	struct JGraphicShaderData;
	struct JComputeShaderData;
	class JMeshGeometry;
	class JMaterial;
	class JShader;
	class JScene;
	class JResourceManager; 

	namespace Application
	{
		class JApplication;
	}
	namespace Core
	{
		class JGraphicException;
	}
	namespace Editor
	{
		class JImGui; 
		class JGraphicResourceWatcher;
	}
	namespace Graphic
	{
		class JGraphicResourceManager;
		class JGraphicResourceInfo;
		class JGraphicSingleResourceInterface;
		class JGraphicSingleResourceUserInterface;
		class JGraphicMultiResourceInterface; 
		class JGraphicMultiResourceUserInterface;
		class JCullingInfo;
		class JCullingInterface;
		class JFrameUpdateData;

		class JGraphicPrivate
		{
		public:
			class DeviceInterface
			{
			private:
				friend class JMeshGeometry; 
				friend class Core::JGraphicException;  
			private:
				static ID3D12Device* GetDevice() noexcept;
			};

			class ResourceInterface
			{
			private:
				friend class JShader;
				friend class JGraphicResourceInfo;
				friend class JGraphicSingleResourceInterface;
				friend class JGraphicMultiResourceInterface;
				friend class JGraphicSingleResourceUserInterface;
				friend class JGraphicMultiResourceUserInterface;
				friend class JCullingInterface;			//for create occ resource
			private:
				static CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(const J_GRAPHIC_BIND_TYPE bType, int index)noexcept;
				static CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(const J_GRAPHIC_BIND_TYPE bType, int index)noexcept;
				static CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(const J_GRAPHIC_RESOURCE_TYPE rType,
					const J_GRAPHIC_BIND_TYPE bType,
					int rIndex,
					int bIndex)noexcept;
				static CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(const J_GRAPHIC_RESOURCE_TYPE rType,
					const J_GRAPHIC_BIND_TYPE bType,
					int rIndex,
					int bIndex)noexcept;
			private:
				static JUserPtr<JGraphicResourceInfo> CreateSceneDepthStencilResource();
				static JUserPtr<JGraphicResourceInfo> CreateSceneDepthStencilDebugResource();
				static JUserPtr<JGraphicResourceInfo> CreateDebugDepthStencilResource();
				static void CreateOcclusionHZBResource(_Out_ JUserPtr<JGraphicResourceInfo>& outOccDsInfo, _Out_ JUserPtr<JGraphicResourceInfo>& outOccMipMapInfo);
				static JUserPtr<JGraphicResourceInfo> CreateOcclusionHZBResourceDebug();
				static JUserPtr<JGraphicResourceInfo> Create2DTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadHeap, const std::wstring& path, const std::wstring& oriFormat);
				static JUserPtr<JGraphicResourceInfo> CreateCubeMap(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadHeap, const std::wstring& path, const std::wstring& oriFormat);
				static JUserPtr<JGraphicResourceInfo> CreateRenderTargetTexture(uint textureWidth = 0, uint textureHeight = 0);
				static JUserPtr<JGraphicResourceInfo> CreateShadowMapTexture(uint textureWidth = 0, uint textureHeight = 0);
			private:
				static bool DestroyGraphicTextureResource(JGraphicResourceInfo* info);
			private:
				static void StuffGraphicShaderPso(JGraphicShaderData* shaderData,
					const J_SHADER_VERTEX_LAYOUT vertexLayout,
					const J_GRAPHIC_SHADER_FUNCTION gFunctionFlag,
					const JShaderGraphicPsoCondition& psoCondition,
					const J_GRAPHIC_EXTRA_PSO_TYPE extraType = J_GRAPHIC_EXTRA_PSO_TYPE::NONE);
				static void StuffComputeShaderPso(JComputeShaderData* shaderData, const J_COMPUTE_SHADER_FUNCTION cFunctionFlag);
			};
			class CullingInterface
			{
			private:
				friend class JCullingInfo;
				friend class JCullingInterface;
			private:
				static JUserPtr<JCullingInfo> CreateFrsutumCullingResultBuffer();
				static JUserPtr<JCullingInfo> CreateOccCullingResultBuffer();
			private:
				static bool DestroyCullignData(JCullingInfo* cullignInfo);
			};

			class EditorInterface
			{
			private:
				friend class Editor::JImGui;
			private:
				static void SetImGuiBackEnd();
			};
			class DebugInterface
			{
			private:
				friend class Editor::JGraphicResourceWatcher;
			private:
				static JGraphicResourceManager* GetGraphicResourceManager()noexcept;
			};
			class CommandInterface
			{
			private:
				friend class JMeshGeometry;
				friend class JMaterial;
				friend class Application::JApplication;
			private:
				static ID3D12CommandQueue* GetCommandQueue()noexcept;
				static ID3D12CommandAllocator* GetCommandAllocator()noexcept;
				static ID3D12GraphicsCommandList* GetCommandList()noexcept;
				static void StartCommand();
				static void EndCommand();
				static void FlushCommandQueue();
			};
			class AppAccess
			{
			private:
				friend class Application::JApplication;
			private:
				static void Initialize();
				static void Clear(); 
			private:
				static void StartFrame();
				static void EndFrame();
			private:
				static void UpdateWait();
				static void UpdateEngine();
			private:
				static void DrawScene();
				static void DrawProjectSelector();
			private:
				static void WriteLastRsTexture();
			};
		};
	}
}