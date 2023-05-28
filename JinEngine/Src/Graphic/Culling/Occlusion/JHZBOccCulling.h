#pragma once 
#include"../../Upload/JUploadBuffer.h"
#include"../../Utility/JHlslDebug.h"
#include"../../../Utility/JVector.h"  
#include"../../../Core/Pointer/JOwnerPtr.h"
#include<DirectXCollision.h> 
#include<vector>

struct ID3D12RootSignature;
struct ID3D12Device;
struct ID3D12GraphicsCommandList;
struct ID3D12Resource;

namespace JinEngine
{
	class JCamera;
	class JLight;
	class JGameObject;
	class JRenderItem;
	class JScene;

	namespace Graphic
	{
		struct JGraphicInfo;
		struct JGraphicOption;
		struct JHzbOccDepthMapInfoConstants; 
		class JFrameResource;
		class JCullingUserInterface;
		class JCullingInfo;

		class JHZBOccCulling
		{ 
		private: 
			std::unique_ptr<JUploadBuffer<JHzbOccDepthMapInfoConstants>> occDepthMapInfoCB;	//static resource
			std::unique_ptr <JUploadBuffer<float>> occQueryOutBuffer;
			std::vector<std::unique_ptr<JUploadBuffer<float>>>occResultBuffer;
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature; 
		public:
			void Initialize(ID3D12Device* d3dDevice, const JGraphicInfo& info);
			void Clear();
		public:
			bool BuildOccBuffer(ID3D12Device* device, const size_t initCapacity, const JUserPtr<JCullingInfo>& cullingInfo);
			void ReBuildOccBuffer(ID3D12Device* device, const size_t capacity, const std::vector<JUserPtr<JCullingInfo>>& cullingInfo);
			void DestroyOccBuffer(JCullingInfo* cullignInfo);
		public:
			ID3D12RootSignature* GetRootSignature()const noexcept; 
			uint GetPassElementCount()const noexcept;
		public:
			//Set upload buffer and shader macro
			void ReBuildOcclusionMapInfoConstants(ID3D12Device* device, const JGraphicInfo& info);	//미적용 occlusion map option 설정하는 인터페이스 추가필요
			void ReBuildObjectConstants(ID3D12Device* device, const uint objectCapacity);
			//pass count == drawList cam count
			void ReBuildPassConstants(ID3D12Device* device, const uint passCapacity);
		public:
			void DepthMapDownSampling(ID3D12GraphicsCommandList* commandList,
				JFrameResource* currFrame,
				CD3DX12_GPU_DESCRIPTOR_HANDLE depthMapSrvHandle,
				CD3DX12_GPU_DESCRIPTOR_HANDLE mipMapSrvHandle,
				CD3DX12_GPU_DESCRIPTOR_HANDLE mipMapUavHandle, 
				const uint samplingCount,
				const uint srvDescriptorSize,
				const uint passCBIndex);
			void OcclusionCulling(ID3D12GraphicsCommandList* commandList,
				JFrameResource* currFrame,
				CD3DX12_GPU_DESCRIPTOR_HANDLE mipMapStHandle,
				const uint passCBIndex,
				const JCullingUserInterface& cullUser);
			void PrintDebug(const JUserPtr<JCullingInfo>& cullingInfo); 
		private:
			void BuildRootSignature(ID3D12Device* d3dDevice, const uint occlusionDsvCapacity);
			void BuildUploadBuffer(ID3D12Device* device, const uint objectCapacity, const uint occlusionMapCapacity);
		};
	}
}