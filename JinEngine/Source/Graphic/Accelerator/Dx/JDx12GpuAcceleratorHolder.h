#pragma once
#include"../JGpuAcceleratorHolder.h"
#include"../../Buffer/Dx/JDx12GraphicBuffer.h"
#include<wrl/client.h>   
#include<d3dx12.h>
#include<memory>
#include<unordered_map>

namespace JinEngine
{
    class JComponent;
	namespace Graphic
	{
        class JGraphicResourceInfo; 
        class JDx12GpuAcceleratorManager;

        //Top level accelerator structure
        class JTlasHolder
        {
        public:
            JDx12GraphicBuffer accelerationStructure;
        public: 
            JTlasHolder(JDx12GraphicBuffer&& buffer);
            ~JTlasHolder() = default;
            JTlasHolder(const JTlasHolder& rhs) = delete;
            JTlasHolder(JTlasHolder&& rhs) = default;
            JTlasHolder& operator=(const JTlasHolder& rhs) = delete;
            JTlasHolder& operator=(JTlasHolder&& rhs) = default;
        };

        //Bottom level accelerator structure
        //per submesh(one submesh one blas)
        class JBlasHolder
        {
        public:
            uint index;
            uint refInstanceCount = 0;
            std::vector<JDx12GraphicBuffer> buffer;        //same as mesh submesh count
        public: 
            JBlasHolder(uint index);
            JBlasHolder(uint index, std::vector<JDx12GraphicBuffer>&& buffer);
            ~JBlasHolder() = default;
            JBlasHolder(const JBlasHolder& rhs) = delete;
            JBlasHolder(JBlasHolder&& rhs) = default;
            JBlasHolder& operator=(const JBlasHolder& rhs) = delete;
            JBlasHolder& operator=(JBlasHolder&& rhs) = default;
        };
         
        //instance per component(rItem, light) submesh
        //one component can allocate multiple instance
        class JInstanceCache 
        {
        public:
            JUserPtr<JComponent> comp;
        public: 
            uint index = 0;
            uint descStIndex = 0;   
        public:
            JInstanceCache(const JUserPtr<JComponent>& comp, const uint index, const uint descStIndex);
        };

        /*
        * Dx12GpuAcceleratorManager에게 관리되며
        * 종속된 Scene에 등록된 Component에 따라 data가 구성되며
        * 유효한 Component가 하나도 없을시 Resource를 clear된 상태를 유지하며
        * User는 HasData()로 Holder에 유효성을 검사할 필요가있다. (ex: gpu binding)
        */
        class JDx12GpuAcceleratorHolder : public JGpuAcceleratorHolder
        { 
        private:
            friend class JDx12GpuAcceleratorManager;
        private:
            std::unique_ptr<JTlasHolder> tlas;
            std::vector<std::unique_ptr<JBlasHolder>> blasVec;
            std::unordered_map<size_t, JBlasHolder*> blasMap;       //key = mesh guid
        private:
            JDx12GraphicBufferT<D3D12_RAYTRACING_INSTANCE_DESC> instanceData;
        private:
            //for fast rebuild
            std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instanceDescVec;        
            std::vector<std::unique_ptr<JInstanceCache>> instanceCacheVec;
            std::unordered_map<size_t, JInstanceCache*> instanceCacheMap;       //key = component guid
        public:
            JDx12GpuAcceleratorHolder(std::unique_ptr<JTlasHolder>&& tlas,
                std::vector<std::unique_ptr<JBlasHolder>>&& blasVec,
                std::unordered_map<size_t, JBlasHolder*>&& blasMap,
                JDx12GraphicBufferT<D3D12_RAYTRACING_INSTANCE_DESC>&& instanceData,
                std::vector<D3D12_RAYTRACING_INSTANCE_DESC>&& instanceDescVec,
                std::vector<std::unique_ptr<JInstanceCache>>&& instanceCacheVec,
                std::unordered_map<size_t, JInstanceCache*>&& instanceCacheMap);
            ~JDx12GpuAcceleratorHolder();
        public:
            void Clear()noexcept final;
        private:
            void ClearResource()noexcept;
        public:
            J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
            JTlasHolder* GetTlas()const noexcept;
            D3D12_GPU_VIRTUAL_ADDRESS GetTlasGpuAddress()const noexcept;
            uint GetBlasCount()const noexcept;
            JBlasHolder* GetBlas(const size_t key)const noexcept;  
        public:
            bool HasData()const noexcept;
        private:
            void AddBlas(std::unique_ptr<JBlasHolder>&& newBlas, const size_t key);
            void RemoveBlas(const uint index, const size_t key);
        private:
            void Swap(std::unique_ptr<JTlasHolder>&& tlasHolder); 
        };
	}
}