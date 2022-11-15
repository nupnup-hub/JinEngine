#pragma once  
#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif 
#include"JGraphicInterface.h"
#include"JGraphicTextureUserInterface.h"
#include"JGraphicOption.h" 
#include"FrameResource/JAnimationConstants.h"
#include"../Object/JFrameUpdate.h"
#include"../Object/Component/RenderItem/JRenderLayer.h"   
#include"../Object/Resource/Shader/JShaderFunctionEnum.h" 
#include"../Core/Singleton/JSingletonHolder.h"
#include"../Core/Event/JEventListener.h"
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
		class JGraphicTextureHandle;
		class JGraphicResourceManager;
		class JOcclusionCulling;

		class JGraphicImpl final : public JGraphicApplicationIterface,
			public JGraphicTextureUserInterface,
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
			std::unique_ptr<JGraphicResourceManager> graphicResource;

			int currBackBuffer = 0;
		private:
			int width = 0;
			int height = 0;
			uint occlusionWidth = 512;
			uint occlusionHeight = 256;
			const uint defaultShadowWidth = 2048;
			const uint defaultShadowHeight = 2048;
			bool stCommand = false;
		private:
			JGraphicOption graphicOption;
			std::unique_ptr<JOcclusionCulling> occHelper;
		public:
			JGraphicOption GetGraphicOption()const noexcept;
			void SetGraphicOption(const JGraphicOption& newGraphicOption)noexcept;
		public:
			JGraphicDeviceInterface* DeviceInterface()noexcept;
			JGraphicResourceInterface* ResourceInterface()noexcept;
			JGraphicEditorInterface* EditorInterface()noexcept;
			JGraphicCommandInterface* CommandInterface()noexcept;
			JGraphicApplicationIterface* AppInterface()noexcept;
		public:
			//Debug
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetOcclusionSrvHandle();
		private:
			void OnEvent(const size_t& senderGuid, const Window::J_WINDOW_EVENT& eventType)final;
		private:
			ID3D12Device* GetDevice() const noexcept final;
		private:
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuSrvDescriptorHandle(int index)const noexcept final;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuSrvDescriptorHandle(int index)const noexcept final;
		private:
			JGraphicTextureHandle* Create2DTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadHeap, const std::wstring& path, const std::wstring& oriFormat)final;
			JGraphicTextureHandle* CreateCubeTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadHeap, const std::wstring& path, const std::wstring& oriFormat)final;
			JGraphicTextureHandle* CreateRenderTargetTexture(uint textureWidth = 0, uint textureHeight = 0)final;
			JGraphicTextureHandle* CreateShadowMapTexture(uint textureWidth = 0, uint textureHeight = 0)final;
			bool DestroyGraphicTextureResource(JGraphicTextureHandle** handle)final;
			void StuffGraphicShaderPso(JGraphicShaderData* shaderData, J_SHADER_VERTEX_LAYOUT vertexLayout, J_SHADER_FUNCTION functionFlag)final;
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
			void UpdateSceneObjectCB(_In_ JScene* scene, uint& objCBoffset, _Out_ uint& updateCount, _Out_ uint& hotUpdateCount);
			void UpdateMaterialCB();
			//Always update
			void UpdateScenePassCB(_In_ JScene* scene, uint& passCBoffset);
			void UpdateSceneAnimationCB(_In_ JScene* scene, uint& aniCBoffset, _Out_ uint& updateCount, _Out_ uint& hotUpdateCount);
			void UpdateSceneCameraCB(_In_ JScene* scene, uint& camCBoffset, _Out_ uint& updateCount, _Out_ uint& hotUpdateCount);
			//Always update has dirty return 1 or return 0
			void UpdateSceneLightCB(_In_ JScene* scene, uint& lightCBoffset, uint& shadowCBoffset, _Out_ uint& updateCount, _Out_ uint& hotUpdateCount);
		private:
			void DrawScene()final;
			void DrawProjectSelector()final;
			void DrawSceneRenderTarget(_In_ JScene* scene,
				_In_ JCamera* camera,
				const uint objCBoffset,
				const uint passCBoffset,
				const uint aniCBoffset,
				const uint camCBoffset,
				const uint lightCBoffset,
				const bool isOcclusionActivated);
			void DrawSceneShadowMap(_In_ JScene* scene,
				_In_ JLight* light,
				const uint objCBoffset,
				const uint passCBoffset,
				const uint aniCBoffset,
				const uint shadowCBoffset);
			void DepthTest(_In_ JScene* scene, const uint objCBoffset);
			void DrawGameObject(ID3D12GraphicsCommandList* cmdList,
				const std::vector<JGameObject*>& gameObject,
				const uint objCBoffset,
				const uint aniCBoffset,
				const bool isDrawShadowMap,
				const bool isAnimationActivated,
				const bool isOcclusionActivated);
			void DrawSceneBoundingBox(ID3D12GraphicsCommandList* cmdList,
				const std::vector<JGameObject*>& gameObject,
				const uint objCBoffset,
				const bool isAnimationActivated);
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

			void OnResize();
			ID3D12Resource* CurrentBackBuffer()const;
			D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()	const;
			const std::vector<CD3DX12_STATIC_SAMPLER_DESC> Sampler()const noexcept;
		private:
			void StoreOptionData()final;
			void LoadOptionData()final;
		private:
			JGraphicImpl();
			~JGraphicImpl();
		};
	}
	using JGraphic = JinEngine::Core::JSingletonHolder<Graphic::JGraphicImpl>;
}