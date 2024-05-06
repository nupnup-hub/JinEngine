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
#include"JDx12GraphicResourceConstants.h"
#include"../JGraphicResourceManager.h"  
#include"../../JGraphicEnum.h"
#include"../../../Core/JCoreEssential.h"  
#include"../../../Core/Pointer/JOwnerPtr.h"
#include"../../../Core/Handle/JDataHandleStructure.h"
#include"../../../../ThirdParty/DirectX/Tk/Src/d3dx12.h"  
#include<WindowsX.h> 
#include<wrl/client.h>  
#include<d3d12.h>
#include<dxgi1_4.h>  

namespace JinEngine
{
	namespace Editor
	{
		class JGraphicResourceWatcher;
	}
	class JMeshGeometry;
	namespace Graphic
	{
		struct JDx12GraphicResourceHolderDesc;
		class JDx12GraphicResourceHolder;
		struct JDx12SwapChainCreationData
		{
		public:
			ID3D12Device* device;
			IDXGIFactory4* dxgiFactory;
			ID3D12CommandQueue* commandQueue; 
			Microsoft::WRL::ComPtr<IDXGISwapChain3>* swapChain;
			uint width;
			uint height;
			bool m4xMsaaState;
			uint m4xMsaaQuality; 
		};
		class JGraphicResourceUserInterface;
		class JDx12GraphicResourceInfo;
		class JDx12GraphicResourceManager final : public JGraphicResourceManager
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JDx12GraphicResourceManager)
		private:
			struct ExtraOption
			{
			public:
				bool bindResourceManually = false;
			};
		private:
			using CreationProcessPtr = JUserPtr<JGraphicResourceInfo>(JDx12GraphicResourceManager::*)(const J_GRAPHIC_RESOURCE_TYPE, JGraphicDevice*, const JGraphicResourceCreationDesc&, const ExtraOption&);
			using MPBStructure = Core::JDataHandleStructure<MPBCapactiy(), MPBInfo, true>;
		private:
			//friend class JGraphic;
			friend class Editor::JGraphicResourceWatcher;
		private:
			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap;
			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap;
			uint rtvDescriptorSize = 0;
			uint dsvDescriptorSize = 0;
			uint cbvSrvUavDescriptorSize = 0;
		private:
			std::vector<JOwnerPtr<JDx12GraphicResourceInfo>> resource[(uint)J_GRAPHIC_RESOURCE_TYPE::COUNT];
			/*
			* Type별 allocate information desc
			* GraphicResourceInfo와 GraphicResourceInfo가 사용하는 Option은 같은 Desc를 공유한다.
			* Bind는 GraphicResourceInfo -> Option순으로 일어나지만 bind type에 대해 fixed한 type만 존재하는게 아니니
			* 관리시에는 isFixed trigger등을 참조해서 필요하다면 특정영역에 모든 GraphicResourceInfo를 참조할 필요가있다.
			*/
			ResourceTypeDesc typeDesc[(uint)J_GRAPHIC_RESOURCE_TYPE::COUNT];
		private:
			JUserPtr<JGraphicResourceInfo> defaultSceneDsInfo;	//cashing
		private:
			MPBStructure mpb; 
		public: 
			~JDx12GraphicResourceManager();
		public:
			void Initialize(JGraphicDevice* device)final;
			void Clear()final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuRtvDescriptorHandle(int index)const noexcept;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuRtvDescriptorHandle(int index)const noexcept;
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuDsvDescriptorHandle(int index)const noexcept;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuDsvDescriptorHandle(int index)const noexcept;
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuSrvDescriptorHandle(int index)const noexcept;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuSrvDescriptorHandle(int index)const noexcept;
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(const J_GRAPHIC_BIND_TYPE bType, int index)const noexcept;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(const J_GRAPHIC_BIND_TYPE bType, int index)const noexcept;
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(const J_GRAPHIC_RESOURCE_TYPE rType,
				const J_GRAPHIC_BIND_TYPE bType,
				const int rIndex,
				const int bIndex);
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(const J_GRAPHIC_RESOURCE_TYPE rType,
				const J_GRAPHIC_BIND_TYPE bType,
				const int rIndex,
				const int bIndex);
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetFirstGpuSrvDescriptorHandle(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept;
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetMPBCpuDescriptorHandle(const Core::JDataHandle& handle, const J_GRAPHIC_BIND_TYPE bType)const noexcept;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetMPBGpuDescriptorHandle(const Core::JDataHandle& handle, const J_GRAPHIC_BIND_TYPE bType)const noexcept;
			ID3D12DescriptorHeap* GetDescriptorHeap(const J_GRAPHIC_BIND_TYPE bType)const noexcept;
			uint GetDescriptorSize(const J_GRAPHIC_BIND_TYPE bType)const noexcept; 
			D3D12_VERTEX_BUFFER_VIEW VertexBufferView(const JUserPtr<JMeshGeometry>& mesh)const noexcept;
			D3D12_INDEX_BUFFER_VIEW IndexBufferView(const JUserPtr<JMeshGeometry>& mesh)const noexcept;
		public: 
			uint GetResourceCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final;
			uint GetResourceCapacity(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final;
			uint GetViewCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)const noexcept final;
			uint GetViewCapacity(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)const noexcept final;
			uint GetViewOffset(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)const noexcept final;
			uint GetTotalViewCount(const J_GRAPHIC_BIND_TYPE bType)const noexcept final;
			uint GetTotalViewCapacity(const J_GRAPHIC_BIND_TYPE bType)const noexcept final;
			ResourceHandle GetResourceGpuHandle(const J_GRAPHIC_BIND_TYPE bType, int index)const noexcept final;
			ResourceHandle GetMPBResourceCpuHandle(const Core::JDataHandle& handle, const J_GRAPHIC_BIND_TYPE bType)const noexcept final;
			ResourceHandle GetMPBResourceGpuHandle(const Core::JDataHandle& handle, const J_GRAPHIC_BIND_TYPE bType)const noexcept final;
			ID3D12Resource* GetResource(const J_GRAPHIC_RESOURCE_TYPE rType, int index)const noexcept;
			ID3D12Resource* GetOptionResource(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, int index)const noexcept;
			JGraphicResourceInfo* GetInfo(const J_GRAPHIC_RESOURCE_TYPE rType, int index)const noexcept final;
			JGraphicResourceInfo* GetMPBInfo(const Core::JDataHandle& handle)const noexcept final;
			JDx12GraphicResourceInfo* GetDxInfo(const J_GRAPHIC_RESOURCE_TYPE rType, int index)const noexcept;
			JDx12GraphicResourceInfo* GetDefaultSceneDsInfo()const noexcept;
			JDx12GraphicResourceHolder* GetDxHolder(const J_GRAPHIC_RESOURCE_TYPE rType, int index)const noexcept;
			JDx12GraphicResourceHolder* GetOptionHolder(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, int index)const noexcept;
		private:
			uint GetHeapIndex(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType);
		public:
			void SetViewCount(JDx12GraphicResourceInfo* handlePtr, const J_GRAPHIC_BIND_TYPE bType, const int nextViewIndex);
			void SetOptionViewCount(JDx12GraphicResourceInfo* handlePtr, const J_GRAPHIC_BIND_TYPE bType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const int nextViewIndex);
		public:
			bool CanCreateResource(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final;
			bool CanCreateResource(const J_GRAPHIC_RESOURCE_TYPE rType, JGraphicDevice* device)noexcept;
			bool CanCreateOptionResource(const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final;
		private:
			bool HasDependency(const JGraphicInfo::TYPE type)const noexcept final;
			bool HasDependency(const JGraphicOption::TYPE type)const noexcept final;
		private:
			void NotifyGraphicInfoChanged(const JGraphicInfoChangedSet& set)final;
			void NotifyGraphicOptionChanged(const JGraphicOptionChangedSet& set)final;
		public:
			void CreateSwapChainBuffer(JDx12SwapChainCreationData& creationData);
			JUserPtr<JGraphicResourceInfo> CreateResource(JGraphicDevice* device, const JGraphicResourceCreationDesc& creationDesc, const J_GRAPHIC_RESOURCE_TYPE rType)final;
			bool CreateOption(JGraphicDevice* device, JUserPtr<JGraphicResourceInfo> info, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)final;
			bool DestroyGraphicTextureResource(JGraphicDevice* device, JGraphicResourceInfo* info)final;
			bool DestroyGraphicOption(JGraphicDevice* device, JUserPtr<JGraphicResourceInfo>& info, const J_GRAPHIC_RESOURCE_OPTION_TYPE optype)final;
		private: 
			JUserPtr<JDx12GraphicResourceInfo> CommonCreationProcess(JGraphicDevice* device, const JGraphicResourceCreationDesc& creationDesc, const J_GRAPHIC_RESOURCE_TYPE rType, const ExtraOption& extraOption = ExtraOption());
			JUserPtr<JDx12GraphicResourceInfo> CreateTexture2D(JGraphicDevice* device, const JGraphicResourceCreationDesc& creationDesc);
			JUserPtr<JDx12GraphicResourceInfo> CreateCubeMap(JGraphicDevice* device, const JGraphicResourceCreationDesc& creationDesc);
			JUserPtr<JDx12GraphicResourceInfo> CreateTextureCommon(JGraphicDevice* device, const JGraphicResourceCreationDesc& creationDesc);
			JUserPtr<JDx12GraphicResourceInfo> CreateLightLinkedList(JGraphicDevice* device, const JGraphicResourceCreationDesc& creationDesc);
			JUserPtr<JDx12GraphicResourceInfo> CreateResourceInfo(const J_GRAPHIC_RESOURCE_TYPE rType, JDx12GraphicResourceHolderDesc&& result);
			bool CreateOption(const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, JDx12GraphicResourceInfo* dxInfo, JDx12GraphicResourceHolderDesc&& result);
		private:
			void ClearDescViewCountUntil(JDx12GraphicResourceInfo* dxInfo);
			void ReBind(ID3D12Device* device, const J_GRAPHIC_RESOURCE_TYPE rType, const uint index, const int additionalArrayIndex = 0);
		public:
			bool ReAllocTypePerAllResource(JGraphicDevice* device, const J_GRAPHIC_RESOURCE_TYPE rType);
			bool ReAllocTypePerAllResource(JGraphicDevice* device, const JGraphicResourceCreationDesc& creationDesc, const J_GRAPHIC_RESOURCE_TYPE rType) final;
		public: 
			bool CopyResource(JGraphicDevice* device, const JUserPtr<JGraphicResourceInfo>& from, const JUserPtr<JGraphicResourceInfo>& to) final;
		public:
			bool SettingMipmapBind(JGraphicDevice* device, const JUserPtr<JGraphicResourceInfo>& info, const bool isReadOnly, _Out_ std::vector<Core::JDataHandle>& handle)final;
			void DestroyMPB(JGraphicDevice* device, Core::JDataHandle& handle) final;
			uint GetMPBOffset(const J_GRAPHIC_BIND_TYPE bType)const noexcept;
		private:
			void BuildResource(JGraphicDevice* device);
			void BuildTypeDesc(ID3D12Device* device);
			void BuildRtvDescriptorHeaps(ID3D12Device* device);
			void BuildDsvDescriptorHeaps(ID3D12Device* device);
			void BuildSrvDescriptorHeaps(ID3D12Device* device);
		private:
			void ClearResource();
		public:
			void ResizeWindow(JGraphicDevice* device)final;
		public:
			void StoreTexture(JGraphicDevice* device, const J_GRAPHIC_RESOURCE_TYPE rType, const int index, const std::wstring& path)final;
		public:
			static void RegisterTypeData();
		};

		struct JDx12GraphicResourceComputeSet
		{
		private:
			static constexpr J_GRAPHIC_RESOURCE_OPTION_TYPE invalidType = J_GRAPHIC_RESOURCE_OPTION_TYPE::COUNT;
		public:
			JDx12GraphicResourceInfo* info = nullptr;
			JDx12GraphicResourceManager* gm = nullptr;
			JDx12GraphicResourceHolder* holder = nullptr;
			ID3D12Resource* resource = nullptr; 
		public:
			uint viewOffset = 0;
		private: 
			//if construct by option
			//set valid enum
			J_GRAPHIC_RESOURCE_OPTION_TYPE opType = J_GRAPHIC_RESOURCE_OPTION_TYPE::COUNT;
		public:
			JDx12GraphicResourceComputeSet() = default;
			JDx12GraphicResourceComputeSet(JDx12GraphicResourceManager* gm, JGraphicResourceInfo* gInfo);
			JDx12GraphicResourceComputeSet(JDx12GraphicResourceManager* gm, const JUserPtr<JGraphicResourceInfo>& info);
			JDx12GraphicResourceComputeSet(JDx12GraphicResourceManager* gm, const JGraphicResourceUserInterface& gInterface, const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType);
			JDx12GraphicResourceComputeSet(JDx12GraphicResourceManager* gm, const JGraphicResourceUserInterface& gInterface, const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex);
			JDx12GraphicResourceComputeSet(JDx12GraphicResourceManager* gm, JGraphicResourceInfo* gInfo, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType);
			JDx12GraphicResourceComputeSet(JDx12GraphicResourceManager* gm, const JUserPtr<JGraphicResourceInfo>& gInfo, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType);
		public:
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuDsvHandle()const noexcept;
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuRtvHandle()const noexcept;
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuSrvHandle()const noexcept;
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuUavHandle()const noexcept;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuDsvHandle()const noexcept;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuRtvHandle()const noexcept;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuSrvHandle()const noexcept;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuUavHandle()const noexcept;
		public:
			bool IsValid()const noexcept;
		}; 
		struct JDx12GraphicResourceComputeSetBufferBase
		{
		public:
			virtual JDx12GraphicResourceComputeSet& operator()(const uint index) = 0; 
		public:
			virtual void Push(JDx12GraphicResourceComputeSet&& set) = 0;
		public:
			virtual uint GetMaxCount()const noexcept = 0;
			virtual uint GetValidCount()const noexcept = 0;
		public:
			virtual void SetValidCount(const uint count) = 0;
		};
		template<int count>
		struct JDx12GraphicResourceComputeSetBuffer : public JDx12GraphicResourceComputeSetBufferBase
		{
		public:
			JDx12GraphicResourceComputeSet buffer[count]; 
		public:
			uint validCount = 0;
		public:
			JDx12GraphicResourceComputeSet& operator()(const uint index)final
			{
				return buffer[index];
			}
		public:
			void Push(JDx12GraphicResourceComputeSet&& set)
			{
				if (validCount >= count)
					return;

				buffer[validCount] = std::move(set);
				++validCount;
			}
		public:
			uint GetMaxCount()const noexcept
			{
				return count;
			}
			uint GetValidCount()const noexcept
			{
				return validCount;
			}
		public:
			void SetValidCount(const uint count)final
			{
				validCount = count;
			}
		};
 
	}
}