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
#include"JCsmType.h"
#include"../Device/JGraphicDeviceUser.h" 
#include"../../Core/Geometry/JBBox.h" 
#include"../../Core/Reflection/JReflection.h"
#include<DirectXCollision.h> 

namespace JinEngine
{
	namespace Graphic
	{
		class JCsmTargetInfo;
		class JCsmHandlerInfo : public JGraphicDeviceUser
		{
			REGISTER_CLASS_USE_ALLOCATOR(JCsmHandlerInfo)
		private:
			struct TargetData
			{
			public:
				JUserPtr<JCsmTargetInfo> info;
				JCsmComputeResult result;
			public:
				TargetData(const JUserPtr<JCsmTargetInfo>& info);
			public:
				bool IsValid()const noexcept;
			};
		private:
			friend class JCsmManager;
		private:
			int index = invalidIndex;
		private:
			JCsmOption option;
			std::vector<TargetData> target;
		private:
			JCsmAreaInfo* areaInfo = nullptr;
		private:
			NotifyAddCsmTargetBindPtr notifyAddCsmTargetB;
			NotifySubtractCsmTargetBindPtr notifySubtractCsmTargetB;
		public:
			int GetIndex()const noexcept;
			JCsmOption GetOption()const noexcept;
			uint GetTargetCount()const noexcept;
			/**
			* @param occur error if index is wrong range
			*/
			const JCsmComputeResult& GetComputeResult(const uint index)const noexcept;
			JCsmAreaInfo* GetAreaInfo()const noexcept;
		public:
			void SetIndex(const int newIndex)noexcept;
			void SetOption(const JCsmOption& newOption)noexcept; 
		public:
			void AddTarget(const JUserPtr<JCsmTargetInfo>& info);
			void RemoveTarget(const uint index);
		private:
			void NotifyAddTarget(const uint index);
			void NotifyPopTarget(const uint index); 
		public:
			void Update(const DirectX::XMMATRIX lightView,
				const DirectX::BoundingBox& sceneBBoxW,			//world bbox 
				const size_t mapSize);
		private:
			void Update(const DirectX::XMMATRIX lightView,
				const DirectX::BoundingBox& sceneBBoxW,			//world bbox
				const DirectX::BoundingFrustum& camFrustumW,	//world frustum
				const size_t mapSize,
				const uint targetIndex);
		protected:
			JCsmHandlerInfo(JCsmAreaInfo* areaInfo,
				NotifyAddCsmTargetBindPtr&& notifyAddCsmTargetB = nullptr,
				NotifySubtractCsmTargetBindPtr&& notifySubtractCsmTargetB = nullptr);
			virtual ~JCsmHandlerInfo() = default;
		};
	}
}