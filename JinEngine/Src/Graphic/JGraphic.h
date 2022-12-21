#pragma once  
#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif 
#include"JGraphicInterface.h"
#include"JGraphicOption.h" 
#include"JGraphicInfo.h" 
#include"Utility/JGraphicUpdateHelper.h"
#include"Utility/JGraphicDrawHelper.h"
#include"GraphicResource/JGraphicResourceUserInterface.h"
#include"FrameResource/JAnimationConstants.h" 
#include"../Object/JFrameUpdate.h"
#include"../Object/Component/RenderItem/JRenderLayer.h"   
#include"../Object/Resource/Shader/JShaderFunctionEnum.h" 
#include"../Core/Singleton/JSingletonHolder.h"
#include"../Core/Event/JEventListener.h"
#include"../Core/Func/Callable/JCallable.h"
#include"../Utility/JVector.h"
#include"../Window/JWindowEventType.h"
#include"../../Lib/DirectX/d3dx12.h"

#include<WindowsX.h>
#include<wrl/client.h> 
#include<memory>
#include<dxgi1_4.h>  
#include<d3d12.h>
#include<vector>
#include<unordered_map>

// Link necessary d3d12 libraries.
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

namespace JinEngine
{
	struct JGraphicShaderData;
	class JCamera;
	class JLight;
	class JGameObject;
	class JScene;

	namespace Core
	{
		template<typename T>class JCreateUsingNew;
	}
	namespace Editor
	{
		class JGraphicResourceWatcher;
	}
	namespace Graphic
	{
		struct JFrameResource;
		class JGraphicResourceHandle;
		class JGraphicResourceManager;
		class JHardwareOccCulling;
		class JHZBOccCulling;
		class JOccBase;
		class JDepthMapDebug;
		class JOutline;

		class JGraphicImpl final : public JGraphicApplicationIterface,
			public JGraphicResourceUserInterface,
			public JFrameBuffManagerInterface,
			public Core::JEventListener<size_t, Window::J_WINDOW_EVENT>
		{
		private:
			template<typename T>friend class Core::JCreateUsingNew;
			friend class Editor::JGraphicResourceWatcher; 			//Debug Class
		private:
			size_t guid;
			std::vector<std::unique_ptr<JFrameResource>> frameResources;
			JFrameResource* currFrameResource = nullptr;
			int currFrameResourceIndex = 0;

			// Set true to use 4X MSAA (?.1.8).  The default is false.
			bool      m4xMsaaState = false;    // 4X MSAA enabled
			uint      m4xMsaaQuality = 0;      // quality level of 4X MSAA

			Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> directCmdListAlloc;
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;

			Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
			Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice;
			Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;

			Microsoft::WRL::ComPtr<ID3D12Fence> fence;
			UINT64 currentFence = 0;

			D3D12_VIEWPORT screenViewport;
			D3D12_RECT scissorRect;

			Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
			int currBackBuffer = 0;
		private:
			bool stCommand = false;
		private:
			JGraphicInfo info;
			JGraphicOption option;
			JGraphicUpdateHelper updateHelper;
			std::unique_ptr<JGraphicResourceManager> graphicResource;
			std::unique_ptr<JHardwareOccCulling> hdOccHelper;
			std::unique_ptr<JHZBOccCulling> hzbOccHelper;
			std::unique_ptr<JDepthMapDebug> depthMapDebug;
			std::unique_ptr<JOutline> outlineHelper;
			JOccBase* occBase = nullptr;
		public:
			JGraphicInfo GetGraphicInfo()const noexcept;
			JGraphicOption GetGraphicOption()const noexcept;
			void SetGraphicOption(JGraphicOption newGraphicOption)noexcept;
		public:
			JGraphicDeviceInterface* DeviceInterface()noexcept;
			JGraphicResourceInterface* ResourceInterface()noexcept;
			JGraphicEditorInterface* EditorInterface()noexcept;
			JGraphicCommandInterface* CommandInterface()noexcept;
			JGraphicApplicationIterface* AppInterface()noexcept;
		private:
			void OnEvent(const size_t& senderGuid, const Window::J_WINDOW_EVENT& eventType)final;
		private:
			ID3D12Device* GetDevice() const noexcept final;
		public:
			//Debug 
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(const J_GRAPHIC_RESOURCE_TYPE rType,
				const J_GRAPHIC_BIND_TYPE bType,
				const int rIndex,
				const int bIndex);
		private:
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(const J_GRAPHIC_BIND_TYPE bType, int index)const noexcept final;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(const J_GRAPHIC_BIND_TYPE bType, int index)const noexcept final;
		private:
			JGraphicResourceHandle* Create2DTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadHeap, const std::wstring& path, const std::wstring& oriFormat)final;
			JGraphicResourceHandle* CreateCubeMap(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadHeap, const std::wstring& path, const std::wstring& oriFormat)final;
			JGraphicResourceHandle* CreateRenderTargetTexture(uint textureWidth = 0, uint textureHeight = 0)final;
			JGraphicResourceHandle* CreateShadowMapTexture(uint textureWidth = 0, uint textureHeight = 0)final;
			bool DestroyGraphicTextureResource(JGraphicResourceHandle** handle)final;
			void StuffGraphicShaderPso(JGraphicShaderData* shaderData, J_SHADER_VERTEX_LAYOUT vertexLayout, J_GRAPHIC_SHADER_FUNCTION gFunctionFlag)final;
			void StuffComputeShaderPso(JComputeShaderData* shaderData, J_COMPUTE_SHADER_FUNCTION cFunctionFlag)final;
		private:
			ID3D12CommandQueue* GetCommandQueue()const noexcept final;
			ID3D12CommandAllocator* GetCommandAllocator()const noexcept final;
			ID3D12GraphicsCommandList* GetCommandList()const noexcept final;
			void StartCommand()final;
			void EndCommand()final;
			void FlushCommandQueue()final;
		private:
			void SetImGuiBackEnd()final;
			void Initialize()final;
			void Clear()final;
		private:
			void StartFrame()final;
			void EndFrame()final;
		private:
			void UpdateWait()final;
			void UpdateEngine()final;  
			void UpdateSceneObjectCB(_In_ JScene* scene, _Out_ uint& updateCount, _Out_ uint& hotUpdateCount);
			void UpdateMaterialCB();
			//Always update
			void UpdateScenePassCB(_In_ JScene* scene);
			void UpdateSceneAnimationCB(_In_ JScene* scene, _Out_ uint& updateCount, _Out_ uint& hotUpdateCount);
			void UpdateSceneCameraCB(_In_ JScene* scene, _Out_ uint& updateCount, _Out_ uint& hotUpdateCount);
			//Always update has dirty return 1 or return 0
			void UpdateSceneLightCB(_In_ JScene* scene,  _Out_ uint& updateCount, _Out_ uint& hotUpdateCount);
		private:
			void DrawScene()final;
			void DrawProjectSelector()final;
			void DrawSceneRenderTarget(const JGraphicDrawHelper helper);
			void DrawSceneShadowMap(const JGraphicDrawHelper helper);
			void DrawOcclusionDepthMap(JGraphicDrawHelper helper);
			void DrawGameObject(ID3D12GraphicsCommandList* cmdList,
				const std::vector<JGameObject*>& gameObject,
				const JGraphicDrawHelper helper,
				const bool isDrawShadowMap,
				const bool isAnimationActivated,
				const bool allowOcclusion);
			void DrawSceneBoundingBox(ID3D12GraphicsCommandList* cmdList,
				const std::vector<JGameObject*>& gameObject, 
				const JGraphicDrawHelper helper,
				const bool isAnimationActivated);
		private:
			void ResourceTransition(_In_ ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter);
		private:
			bool InitializeD3D();
			bool InitializeResource();
			void LogAdapters();
			void LogAdapterOutputs(IDXGIAdapter* adapter);
			void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);
			void CreateCommandObjects();
			void CreateSwapChain();
			void BuildRootSignature();
			void BuildFrameResources();
			void ReBuildFrameResource(const J_UPLOAD_RESOURCE_TYPE type, const J_UPLOAD_CAPACITY_CONDITION condition, const uint nowObjCount);
			void ReCompileGraphicShader();
			J_UPLOAD_CAPACITY_CONDITION IsPassRedefineCapacity(const uint capacity, const uint nowCount)const noexcept;
			uint CalculateCapacity(const J_UPLOAD_CAPACITY_CONDITION condition, const uint nowCapacity, const uint nowCount)const noexcept;
			void OnResize();
			ID3D12Resource* CurrentBackBuffer()const;
			D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()	const;
			const std::vector<CD3DX12_STATIC_SAMPLER_DESC> Sampler()const noexcept;
		private:
			void StoreData(); 
			void LoadData();
		private:
			JGraphicImpl();
			~JGraphicImpl();
		};
	}
	using JGraphic = JinEngine::Core::JSingletonHolder<Graphic::JGraphicImpl>;
}