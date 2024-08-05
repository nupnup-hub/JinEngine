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
#include"../JGraphicConstants.h"  
#include"../../Core/Empty/JEmptyType.h"
#include"../../Core/JCoreEssential.h"
#include"../../Core/Guid/JGuidCreator.h"
#include"../../Core/Pointer/JOwnerPtr.h" 
#include"../../Object/GraphicRule/FrameResource/JGraphicModuleFrameResourceType.h"

namespace JinEngine
{
	namespace Graphic
	{ 
		class JFrameUpdateAreaInfo
		{
		public:
			const size_t guid = 0;
		public:
			int stIndex = 0;
			int edIndex = 0;
		public:
			JFrameUpdateAreaInfo(size_t guid, int stIndex = 0, int edIndex = 0);
		public:
			int GetInfoCount()const noexcept;
			int GetStIndex()const noexcept;
			int GetEdIndex()const noexcept;
		public:
			int Increase();
			int Decrease();
		public:
			int IncreaseEdIndex();
			int DecreaseEdIndex();
		};
		 
		class JFrameUpdateInfo
		{
			REGISTER_CLASS_USE_ALLOCATOR(JFrameUpdateInfo)
		private:
			//same as holdervec index
			int number = 0; 
		private:
			//frame index	calculated) index = pre data index + pre data indexSize
			int index = -1;
			//frame index size
			int indexSize = -1;
		private:
			const J_FRAME_RESOURCE_UPLOAD_TYPE type;
		private:
			JFrameUpdateAreaInfo* areaInfo = nullptr;
		private:
			//int8 movedDataDirty = 0;
			uint8 sortOrder = UCHAR_MAX;		//ascending order number
		protected:
			JFrameUpdateInfo(const J_FRAME_RESOURCE_UPLOAD_TYPE type, JFrameUpdateAreaInfo* areaInfo);
			virtual ~JFrameUpdateInfo() = default;
		public:
			int GetNumber()const noexcept;
			int GetFrameIndex()const noexcept;
			int GetFrameIndexSize()const noexcept;
			J_FRAME_RESOURCE_UPLOAD_TYPE GetType()const noexcept;
			JFrameUpdateAreaInfo* GetAreaInfo()const noexcept;
			uint8 GetSortOrder()const noexcept;
		public:
			void SetNumber(const int newNumber) noexcept;
			void SetFrameIndex(const int newIndex) noexcept;
			void SetFrameIndexSize(const int newSize) noexcept;
			void SetSordOrder(const int8 newOrder)noexcept; 
			//void SetAreaInfo(JFrameUpdateAreaInfo* newAreaInfo)noexcept; 
		public:
			bool HasValidFrameIndex()const noexcept; 
		};
	}
}