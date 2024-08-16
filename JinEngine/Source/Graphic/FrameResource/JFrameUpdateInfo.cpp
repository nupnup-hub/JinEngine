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


#include"JFrameUpdateInfo.h"
#include"../JGraphicPrivate.h"
 
namespace JinEngine
{
	namespace Graphic
	{ 
		JFrameUpdateInfo::JFrameUpdateInfo(const J_FRAME_RESOURCE_UPLOAD_TYPE type, JFrameUpdateAreaInfo* areaInfo)
			:type(type), areaInfo(areaInfo)
		{}
		int JFrameUpdateInfo::GetNumber()const noexcept
		{ 
			return number;
		}
		int JFrameUpdateInfo::GetFrameIndex()const noexcept
		{
			return index;
		}
		int JFrameUpdateInfo::GetFrameIndexSize()const noexcept
		{
			return indexSize;
		}
		J_FRAME_RESOURCE_UPLOAD_TYPE JFrameUpdateInfo::GetType()const noexcept
		{
			return type;
		}
		JFrameUpdateAreaInfo* JFrameUpdateInfo::GetAreaInfo()const noexcept
		{
			return areaInfo;
		}
		uint8 JFrameUpdateInfo::GetSortOrder()const noexcept
		{
			return sortOrder;
		}
		void JFrameUpdateInfo::SetNumber(const int newNumber) noexcept
		{
			number = newNumber;
		}
		void JFrameUpdateInfo::SetFrameIndex(const int newIndex) noexcept
		{
			index = newIndex;
		}
		void JFrameUpdateInfo::SetFrameIndexSize(const int newSize) noexcept
		{
			indexSize = newSize;
		}
		void JFrameUpdateInfo::SetSordOrder(const int8 newOrder)noexcept
		{
			sortOrder = newOrder;
		} 
		/*
		void JFrameUpdateInfo::SetAreaInfo(JFrameUpdateAreaInfo* newAreaInfo)noexcept
		{
			areaInfo = newAreaInfo;
		}
		*/
		bool JFrameUpdateInfo::HasValidFrameIndex()const noexcept
		{
			return index != invalidIndex;
		}

		JFrameUpdateAreaInfo::JFrameUpdateAreaInfo(size_t guid, int stIndex, int count)
			:guid(guid), stIndex(stIndex), count(count)
		{}
		int JFrameUpdateAreaInfo::GetInfoCount()const noexcept
		{
			return count;
		}
		int JFrameUpdateAreaInfo::GetStIndex()const noexcept
		{
			return stIndex;
		}
		int JFrameUpdateAreaInfo::GetEdIndex()const noexcept
		{
			return stIndex + count;
		}
		void JFrameUpdateAreaInfo::IncreaseStIndex()
		{
			++stIndex; 
		}
		void JFrameUpdateAreaInfo::DecreaseStIndex()
		{
			--stIndex; 
		}
		void JFrameUpdateAreaInfo::IncreaseCount()
		{
			++count;
		}
		void JFrameUpdateAreaInfo::DecreaseCount()
		{
			--count;
		}
	}
}