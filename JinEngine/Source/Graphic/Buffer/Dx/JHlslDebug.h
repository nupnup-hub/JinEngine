#pragma once
#include"JDx12GraphicBuffer.h"
#include"../JBufferType.h"
#include<memory>

namespace JinEngine
{
	namespace Graphic
	{
		class JHlslDebugBase
		{ 
		public:
			virtual JDx12GraphicBufferInterface* GetUABufferInterface() = 0;
			virtual JDx12GraphicBufferInterface* GetRBBufferInterface() = 0;
		};
		template<typename T>
		class JHlslDebug : public JHlslDebugBase
		{
		private:
			const std::wstring name;
		private:
			std::unique_ptr<JDx12GraphicBufferT<T>> uaResult;
			std::unique_ptr<JDx12GraphicBufferT<T>> rbResult;
			int rootParamIndex;
		private:
			T* mappedData;
		public:
			JHlslDebug(const std::wstring name, const int rootParamIndex)
				:name(name), rootParamIndex(rootParamIndex)
			{}
			~JHlslDebug()
			{
				Clear();
			}
		public:
			void Build(JGraphicDevice* device, int capacity)
			{ 
				uaResult = std::make_unique<JDx12GraphicBufferT<T>>((name + L"_HlslDebug").c_str(), J_GRAPHIC_BUFFER_TYPE::UNORDERED_ACCEESS);
				rbResult = std::make_unique<JDx12GraphicBufferT<T>>((name + L"_HlslDebug").c_str(), J_GRAPHIC_BUFFER_TYPE::READ_BACK);
				
				uaResult->Build(device, capacity);
				rbResult->Build(device, capacity);
			}
			void Clear()
			{ 
				if (mappedData != nullptr)
					UnMap();
				uaResult.reset();
				rbResult.reset();
			}
		public:
			T* Map(_Out_ int& count)
			{
				if(mappedData == nullptr)
					ThrowIfFailedHr(rbResult->GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&mappedData)));
				count = rbResult->GetElementCount();
				return mappedData;
			}
			void UnMap()
			{
				rbResult->GetResource()->Unmap(0, nullptr);
				mappedData = nullptr;
			} 
		public:
			uint GetRootParamIndex()const noexcept
			{
				return rootParamIndex;
			}
			JDx12GraphicBufferInterface* GetUABufferInterface() final
			{
				return uaResult.get();
			}
			JDx12GraphicBufferInterface* GetRBBufferInterface() final
			{
				return rbResult.get();
			}
		};
	}
}