#pragma once 
#include"../JCullingManager.h" 
#include<vector>

struct ID3D12Device;
struct ID3D12QueryHeap;
struct ID3D12Resource;
namespace Microsoft::WRL
{
	template <typename T> class ComPtr;
}
namespace JinEngine
{
	namespace Graphic
	{ 
		class JDx12CullingResourceHolder;
		class JDx12CullingManager final : public JCullingManager
		{
		private:
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JDx12CullingManager)
		private:
			std::vector<Microsoft::WRL::ComPtr<ID3D12QueryHeap>> occQueryHeap;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
			JDx12CullingResourceHolder* GetDxHolder(const J_CULLING_TYPE type, int index)const noexcept;
			ID3D12Resource* GetResource(const J_CULLING_TYPE type, int index)const noexcept;
			ID3D12QueryHeap* GetQueryHeap(const uint index)const noexcept;
		public:
			void UpdateFrameResourceIndex(const uint frameIndex);
			//for hd occ ... ���� ������Ʈ�� �����ϱ⶧���� �� �����Ӹ��� update�� ������ cpu�� ����
			void CopyCullingResultGpuToCpu(JGraphicDevice* device, const uint frameIndex);
		public:
			void StuffClearValue(JGraphicDevice* device, const J_CULLING_TYPE type) final;
		private:
			void ReBuildBuffer(JCullingInfo* info, JGraphicDevice* device, const size_t capacity, const uint index) final;
		public:
			JUserPtr<JCullingInfo> CreateFrsutumData(JGraphicDevice* device, const JCullingCreationDesc& desc) final;
			JUserPtr<JCullingInfo> CreateHzbOcclusionData(JGraphicDevice* device, const JCullingCreationDesc& desc)final;
			JUserPtr<JCullingInfo> CreateHdOcclusionData(JGraphicDevice* device, const JCullingCreationDesc& desc) final;
		private:
			void PostDestruction(const J_CULLING_TYPE type, const uint index) final;
		public:
			bool TryStreamOutCullingBuffer(JCullingInfo* info, const std::string& logName)final;
		public:
			void Clear() final;
		};
	}
}