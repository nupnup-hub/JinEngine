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
#include"JBufferType.h" 
#include"../Device/JGraphicDevice.h" 
#include"../../Core/JCoreEssential.h"
#include"../../Core/Exception/JExceptionMacro.h"
#include"../../Core/Interface/JValidInterface.h" 

namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicDevice;
		class JGraphicBufferBase : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			virtual ~JGraphicBufferBase() = default;
		public:
			virtual void Build(JGraphicDevice* device, const uint elementcount) = 0;
			virtual void Clear()noexcept = 0;
		public:
			virtual void CopyData(const uint elementIndex, const void* data) = 0;
			virtual void CopyData(const uint elementIndex, const uint count, const void* data, const uint dataElementSize) = 0;
			template<typename T>
			void CopyData(const uint elementIndex, const uint count, const T& data)
			{
				CopyData(elementIndex, count, &data, sizeof(T));
			}
			template<typename T>
			void CopyData(const uint elementIndex, const uint count, const T* data)
			{
				CopyData(elementIndex, count, data, sizeof(T));
			}
			template<typename T>
			void CopyData(const uint elementIndex, const uint count, const std::vector<T>& dataVec)
			{
				CopyData(elementIndex, count, dataVec.data(), sizeof(T));
			}
		public:
			virtual J_GRAPHIC_BUFFER_TYPE GetBufferType()const noexcept = 0;
			virtual uint GetElementCount()const noexcept = 0;
			virtual uint GetElementByteSize()const noexcept = 0;
			virtual BYTE* GetCpuBytePointer()const noexcept = 0;
		public:
			virtual bool CanMappedCpuPointer()const noexcept = 0;
		};
	}
}