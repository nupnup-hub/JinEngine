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
#include"../JCullingResultHolder.h"
#include"../../Buffer/Dx/JDx12GraphicBuffer.h"

namespace JinEngine
{
	namespace Graphic
	{
		using HardwareOcclusionResultType = uint64;
		class JGraphicDevice;
		class JDx12CullingResourceHolder : public JCullingResultHolder
		{
		public:
			virtual ID3D12Resource* GetResource()const noexcept = 0;  
			virtual JDx12GraphicResourceHolder* GetHolder()const noexcept = 0;
		public: 
			bool IsGpuResource()const noexcept final;
			bool CanSetValue()const noexcept final; 
		public:
			virtual void SutffClearValue(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* uploadBuffer) = 0;
		};
		
		class JDx12ReadBackResourceInterface : public JDx12CullingResourceHolder
		{
		public:
			virtual void CopyOnCpuBuffer(const uint stIndex, const uint count)noexcept = 0;
		public:
			J_GRAPHIC_BUFFER_TYPE GetBufferType()const noexcept final;
		public:
			virtual void SetCurrent(const uint frameIndex) = 0;  
		};

		class JDx12FrustumCullingResultHolder final : public JDx12ReadBackResourceInterface
		{
		public:
			using ResultType = uint32;
		private:
			//JDx12GraphicBufferT<ResultType> result;
			JDx12GraphicBufferT<ResultType> readBack[Constants::gNumFrameResources];
			JDx12GraphicBufferT<ResultType>* current = nullptr;
			ResultType* cpuBuffer = nullptr;
			const J_CULLING_TARGET target;
		public:
			JDx12FrustumCullingResultHolder(const J_CULLING_TARGET target, const uint frameInedx);
			~JDx12FrustumCullingResultHolder();
		public:
			void Culling(const uint index, const bool value)noexcept final;
			void CopyOnCpuBuffer(const uint stIndex, const uint count)noexcept final;
		public:
			uint GetElementSize()const noexcept final;
			uint GetElementCount()const noexcept final;
			J_CULLING_TARGET GetCullingTarget()const noexcept final;
			/*
			* @return ReadBack
			*/
			ID3D12Resource* GetResource()const noexcept final;
			JDx12GraphicResourceHolder* GetHolder()const noexcept final;
		public:
			/*
			* @brief 0 is non culling 1 is culling
			*/
			bool IsCulled(const uint index)const noexcept final;
		public:
			void SetCurrent(const uint frameIndex) final;
		public:
			void Build(JGraphicDevice* device, const uint newCapacity);
			void Clear();
			void SutffClearValue(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* uploadBuffer)final;
		};
		class JHzbDx12CullingResultHolder final : public JDx12ReadBackResourceInterface
		{
		public:
			using ResultType = uint;
		private:
			//read back ...  hzb compute시 ua buffer에 쓰인 값을 복사받는다(in JDx12HZBOccCulling)
			JDx12GraphicBufferT<ResultType> readBack[Constants::gNumFrameResources];
			JDx12GraphicBufferT<ResultType>* current = nullptr;
			ResultType* cpuBuffer = nullptr;
			const J_CULLING_TARGET target;
		public:
			JHzbDx12CullingResultHolder(const J_CULLING_TARGET target, const uint frameInedx);
			~JHzbDx12CullingResultHolder();
		public:
			void Culling(const uint index, const bool value)noexcept final;
			void CopyOnCpuBuffer(const uint stIndex, const uint count)noexcept final;
		public:
			uint GetElementSize()const noexcept final;
			uint GetElementCount()const noexcept final;
			J_CULLING_TARGET GetCullingTarget()const noexcept final;
			ID3D12Resource* GetResource()const noexcept final;
			JDx12GraphicResourceHolder* GetHolder()const noexcept final;
		public:
			void SetCurrent(const uint frameIndex) final;
		public:
			/*
			* @brief 0 is non culling 1 is culling
			*/
			bool IsCulled(const uint index)const noexcept final; 
		public:
			void Build(JGraphicDevice* device, const uint newCapacity);
			void Clear();
			void SutffClearValue(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* uploadBuffer)final;
		};
		class JHdDx12CullingResultHolder final : public JDx12ReadBackResourceInterface
		{
		public:
			using ResultType = HardwareOcclusionResultType;
		private: 
			JDx12GraphicBufferT<ResultType> readBack[Constants::gNumFrameResources];
			JDx12GraphicBufferT<ResultType>* current = nullptr;
			ResultType* cpuBuffer = nullptr;
			const J_CULLING_TARGET target;
		public:
			JHdDx12CullingResultHolder(const J_CULLING_TARGET target, const uint frameIndex);
			~JHdDx12CullingResultHolder();
		public:
			void Culling(const uint index, const bool value)noexcept final;
			void CopyOnCpuBuffer(const uint stIndex, const uint count)noexcept final;
		public:
			uint GetElementSize()const noexcept final;
			uint GetElementCount()const noexcept final;
			J_CULLING_TARGET GetCullingTarget()const noexcept final;
			/*
			* @return ReadBack
			*/
			ID3D12Resource* GetResource()const noexcept final; 
			JDx12GraphicResourceHolder* GetHolder()const noexcept final;
		public:
			void SetCurrent(const uint frameIndex) final;
		public:
			/*
			* @brief 1 is non culling 0 is culling
			*/
			bool IsCulled(const uint index)const noexcept final;
		public:
			void Build(JGraphicDevice* device, const uint newCapacity);
			void Clear();
			void SutffClearValue(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* uploadBuffer)final;
		};
	}
}