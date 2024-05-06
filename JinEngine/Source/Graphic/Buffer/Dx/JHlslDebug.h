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
#include"JDx12GraphicBuffer.h"
#include"../JBufferType.h" 

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