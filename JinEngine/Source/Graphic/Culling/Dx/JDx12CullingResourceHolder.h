#pragma once
#include"../JCullingResultHolder.h"
#include"../../Buffer/Dx/JDx12GraphicBuffer.h"

namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicDevice;

		class JDx12CullingResourceHolder : public JCullingResultHolder
		{
		public:
			virtual ID3D12Resource* GetResource()const noexcept = 0;
		public:
			bool IsGpuResource()const noexcept final;
			bool CanSetValue()const noexcept final;
		};

		class JHzbDx12CullingResultHolder final : public JDx12CullingResourceHolder
		{
		public:
			using ResultType = uint;
		private:
			JDx12GraphicBuffer<ResultType> readBack;	//read back ...  hzb compute시 ua buffer에 쓰인 값을 복사받는다(in JDx12HZBOccCulling)
		public:
			JHzbDx12CullingResultHolder();
			~JHzbDx12CullingResultHolder();
		public:
			void Culling(const uint index, const bool value)noexcept final;
		public:
			uint GetBuffSize()const noexcept final;
			ID3D12Resource* GetResource()const noexcept final;
		public:
			/*
			* @brief 0 is non culling 1 is culling
			*/
			bool IsCulled(const uint index)const noexcept final;
		public:
			void Build(JGraphicDevice* device, const uint newCapacity);
			void Clear();
		};

		class JHdDx12CullingResultHolder final : public JDx12CullingResourceHolder
		{
		public:
			using ResultType = uint64;
		private:
			JDx12GraphicBuffer<ResultType> result;
			JDx12GraphicBuffer<ResultType> readBack;	//for debug
		public:
			JHdDx12CullingResultHolder();
			~JHdDx12CullingResultHolder();
		public:
			void Culling(const uint index, const bool value)noexcept final;
		public:
			uint GetBuffSize()const noexcept final;
			/*
			* @return Predict
			*/
			ID3D12Resource* GetResource()const noexcept final;
			ID3D12Resource* GetPredictResource()const noexcept;
			ID3D12Resource* GetReadBackResource()const noexcept;
		public:
			/*
			* @brief 1 is non culling 0 is culling
			*/
			bool IsCulled(const uint index)const noexcept final;
		public:
			void Build(JGraphicDevice* device, const uint newCapacity);
			void Clear();
		};
	}
}