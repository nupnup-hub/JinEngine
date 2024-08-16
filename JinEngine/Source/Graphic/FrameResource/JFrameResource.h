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
#include"JFrameResource.h" 
#include"JFrameResourceType.h" 
#include"../JGraphicConstants.h"
#include"../JGraphicSubClassInterface.h"
#include"../Device/JGraphicDeviceUser.h"
#include"../DataSet/JGraphicObjectDataSet.h"
#include"../../Core/JCoreEssential.h" 

namespace JinEngine
{
	class JObject;
	namespace Graphic
	{
		struct JGraphicInfo;
		class JGraphicDevice;
		class JGraphicBufferBase;
		class JFrameUpdateInterface;
		class JFrameResourceManager;

		class JFrameResource : public JGraphicDeviceUser, public JGraphicSubClassInterface
		{ 
		public:
			virtual void Initialize(JGraphicDevice* device);
			virtual void Clear();
		public:
			virtual JGraphicBufferBase* GetGraphicBufferBase(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept = 0;
			virtual uint GetElementCount(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept = 0;
			virtual GraphicFence GetFenceValue()const noexcept = 0;
			uint GetLocalLightCount()const noexcept;  
		public:
			void CopyData(const J_FRAME_RESOURCE_UPLOAD_TYPE type, const uint elementIndex, const void* data);
			void CopyData(const J_FRAME_RESOURCE_UPLOAD_TYPE type, const uint elementIndex, const uint count, const void* data, const uint dataElementSize);
			template<typename T>
			void CopyData(const J_FRAME_RESOURCE_UPLOAD_TYPE type, const uint elementIndex, const uint count, const std::vector<T>& dataVec)
			{
				CopyData(type, elementIndex, count, dataVec.data(), sizeof(T));
			}
		public:
			virtual void MoveData(const J_FRAME_RESOURCE_UPLOAD_TYPE type, const uint index, const uint moveCount) = 0;
		public:
			virtual void ReBuild(JGraphicDevice* device, const J_FRAME_RESOURCE_UPLOAD_TYPE type, const uint newCount);
		};
	}
}