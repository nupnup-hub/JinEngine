#include"../Object/Resource/Shader/JShaderFunctionEnum.h"
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
	struct JShaderData;
	class ProjectSelector;
	class ImGuiManager;
	class JCamera;
	class JLight;
	class JMeshGeometry;
	class JShader;
	class JTexture;
	class JResourceManager;
	class JResourceIO;
	class ObjectDetail;
	class WindowDirectory;
	class MainSceneEditor;
	class PreviewSceneEditor;
	class SceneViewer;

	namespace Application
	{
		class JApplication;
	}
	namespace Core
	{
		class JGraphicException;
	}
	namespace Graphic
	{
		struct JGraphicTextureHandle;

		class JGraphicDeviceInterface
		{
		private:
			friend class JMeshGeometry;
			friend class JResourceManager;
			friend class Core::JGraphicException;		 
		protected:
			virtual ~JGraphicDeviceInterface() = default;
		private:
			virtual ID3D12Device* GetDevice() const noexcept = 0;
		};

		class JGraphicResourceInterface : public JGraphicDeviceInterface
		{
		private:
			friend class ProjectSelector;
			friend class JCamera;
			friend class JLight;
			friend class JShader;
			friend class JTexture;
			friend class ObjectDetail;
			friend class WindowDirectory;
			friend class MainSceneEditor;
			friend class PreviewSceneEditor;
			friend class SceneViewer;

		private:
			virtual CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuSrvDescriptorHandle(int index)const noexcept = 0;
			virtual CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuSrvDescriptorHandle(int index)const noexcept = 0;

			virtual JGraphicTextureHandle* Create2DTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadHeap, const std::string& path) = 0;
			virtual JGraphicTextureHandle* CreateCubeTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadHeap, const std::string& path) = 0;
			virtual JGraphicTextureHandle* CreateRenderTargetTexture(uint textureWidth = 0, uint textureHeight = 0) = 0;
			virtual JGraphicTextureHandle* CreateShadowMapTexture(uint textureWidth = 0, uint textureHeight = 0) = 0;
			virtual bool EraseGraphicTextureResource(JGraphicTextureHandle* handle) = 0;
			virtual void StuffShaderPso(JShaderData* shaderData, J_SHADER_VERTEX_LAYOUT vertexLayout, J_SHADER_FUNCTION functionFlag) = 0;
		};

		class JGraphicEditorInterface : public JGraphicResourceInterface
		{
		private:
			friend class ImGuiManager;
		private:
			virtual void SetImGuiBackEnd() = 0;
		};

		class JGraphicCommandInterface : public JGraphicEditorInterface
		{
		private:
			friend class JResourceManager;
			friend class JResourceIO;
			friend class JMeshGeometry;
			friend class Application::JApplication;
		private:
			virtual ID3D12CommandQueue* GetCommandQueue()const noexcept = 0;
			virtual ID3D12CommandAllocator* GetCommandAllocator()const noexcept = 0;
			virtual ID3D12GraphicsCommandList* GetCommandList()const noexcept = 0;
			virtual void StartCommand() = 0;
			virtual void EndCommand() = 0;
			virtual void FlushCommandQueue() = 0;
		};

		class JGraphicFrameInterface : public JGraphicCommandInterface
		{
		private:
			friend class Application::JApplication;
		private:
			virtual void SetImGuiBackEnd() = 0;
			virtual void StartFrame() = 0;
			virtual void EndFrame() = 0;
			virtual void UpdateWait() = 0;
			virtual void UpdateEngine() = 0;
			virtual void DrawScene() = 0;
			virtual void DrawProjectSelector() = 0;
		};
	}
}