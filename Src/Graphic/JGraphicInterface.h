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
	class JMeshGeometry;
	class JShader; 
	class JResourceManagerImpl;
	class JResourceIO;   

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
		class JImGuiImpl; 
	}
	namespace Graphic
	{
		class JGraphicTextureHandle;
		class JGraphicTexture;

		class JGraphicDeviceInterface
		{
		private:
			friend class JMeshGeometry;
			friend class JResourceManagerImpl;
			friend class Core::JGraphicException;		 
		protected:
			virtual ~JGraphicDeviceInterface() = default;
		private:
			virtual ID3D12Device* GetDevice() const noexcept = 0;
		};

		class JGraphicResourceInterface : public JGraphicDeviceInterface
		{
		private: 
			friend class JShader;       
			friend class JGraphicTextureHandle;
			friend class JGraphicTexture;
			friend class Editor::JImGuiImpl;
		private: 
			virtual CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuSrvDescriptorHandle(int index)const noexcept = 0;
			virtual CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuSrvDescriptorHandle(int index)const noexcept = 0;
		private:
			virtual JGraphicTextureHandle* Create2DTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadHeap, const std::wstring& path, const std::wstring& oriFormat) = 0;
			virtual JGraphicTextureHandle* CreateCubeTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadHeap, const std::wstring& path, const std::wstring& oriFormat) = 0;
			virtual JGraphicTextureHandle* CreateRenderTargetTexture(uint textureWidth = 0, uint textureHeight = 0) = 0;
			virtual JGraphicTextureHandle* CreateShadowMapTexture(uint textureWidth = 0, uint textureHeight = 0) = 0;
			virtual bool DestroyGraphicTextureResource(JGraphicTextureHandle** handle) = 0;
			virtual void StuffShaderPso(JShaderData* shaderData, J_SHADER_VERTEX_LAYOUT vertexLayout, J_SHADER_FUNCTION functionFlag) = 0;
		};

		class JGraphicEditorInterface : public JGraphicResourceInterface
		{
		private:
			friend class Editor::JImGui;
		private:
			virtual void SetImGuiBackEnd() = 0;
		};

		class JGraphicCommandInterface : public JGraphicEditorInterface
		{
		private:  
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

		class JGraphicApplicationIterface : public JGraphicCommandInterface
		{
		private:
			friend class Application::JApplication;
		private:
			virtual void Initialize() = 0;
			virtual void Clear() = 0;
		private:
			virtual void SetImGuiBackEnd() = 0;
		private:
			virtual void StartFrame() = 0;
			virtual void EndFrame() = 0;
		private:
			virtual void UpdateWait() = 0;
			virtual void UpdateEngine() = 0;
		private:
			virtual void DrawScene() = 0;
			virtual void DrawProjectSelector() = 0;
		};
	}
}