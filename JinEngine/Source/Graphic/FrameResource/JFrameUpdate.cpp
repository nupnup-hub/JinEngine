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


#include"JFrameUpdate.h"
#include"../JGraphicPrivate.h"
 
namespace JinEngine
{
	namespace Graphic
	{
		namespace
		{
			struct JFrameUpdateAreaInfo
			{
			public:
				size_t guid = 0;
				int holderCount = 0;
			public:
				JFrameUpdateAreaInfo(size_t guid, int initCount)
					:guid(guid), holderCount(initCount)
				{} 
			};
			struct JFrameUpdateIndexHolderInfo
			{
			public:
				JFrameUpdateData* ptr = nullptr;
				JFrameUpdateAreaInfo* areaInfo = nullptr;
			public:
				JFrameUpdateIndexHolderInfo(JFrameUpdateData* ptr, JFrameUpdateAreaInfo* areaInfo)
					:ptr(ptr), areaInfo(areaInfo)
				{ } 
			};
			using AreaInfoVec = std::vector<std::unique_ptr<JFrameUpdateAreaInfo>>;
			using FrameUpdateHolderVec = std::vector<JFrameUpdateIndexHolderInfo>;

			static AreaInfoVec areaInfoVec[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::COUNT];
			static FrameUpdateHolderVec holderVec[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::COUNT];

			static int GetAreaVecIndex(const J_UPLOAD_FRAME_RESOURCE_TYPE type, const size_t areaGuid) noexcept
			{
				AreaInfoVec& vec = areaInfoVec[(int)type];
				const int count = (int)vec.size();
				for (int i = 0; i < count; ++i)
				{
					if (vec[i]->guid == areaGuid)
						return i;
				}
				return invalidIndex;
			}
			static int GetAreaStIndex(const J_UPLOAD_FRAME_RESOURCE_TYPE type, const size_t areaGuid)
			{
				FrameUpdateHolderVec& vec = holderVec[(int)type];
				const int count = (int)vec.size();
				for (int i = 0; i < count; ++i)
				{
					if (vec[i].areaInfo->guid == areaGuid)
						return i;
				}
				return invalidIndex;
			}
			static int GetAreaEdIndex(const J_UPLOAD_FRAME_RESOURCE_TYPE type, const size_t areaGuid, const int areaSt)
			{
				if (areaSt != invalidIndex)
				{
					FrameUpdateHolderVec& vec = holderVec[(int)type];
					const int count = (int)vec.size();
					for (int i = areaSt; i < count; ++i)
					{
						if (vec[i].areaInfo->guid != areaGuid)
							return i - 1;
					}
					return count - 1;
				}
				return invalidIndex;
			}
			static int GetAreaEdIndex(const J_UPLOAD_FRAME_RESOURCE_TYPE type, const size_t areaGuid)
			{
				return GetAreaEdIndex(type, areaGuid, GetAreaStIndex(type, areaGuid));
			}
			static int GetArrayIndex(const J_UPLOAD_FRAME_RESOURCE_TYPE type, JFrameUpdateData* ptr)noexcept
			{
				FrameUpdateHolderVec& vec = holderVec[(int)type];
				const uint count = (uint)vec.size();
				for (uint i = 0; i < count; ++i)
				{
					if (vec[i].ptr == ptr)
						return i;
				}
				return invalidIndex;
			}
		}

		int JFrameUpdateData::GetFrameIndex()const noexcept
		{
			return index;
		}
		int JFrameUpdateData::GetFrameIndexSize()const noexcept
		{
			return indexSize;
		}
		uint JFrameUpdateData::GetTotalRegistedCount(const J_UPLOAD_FRAME_RESOURCE_TYPE type)
		{
			return (uint)holderVec[(int)type].size();
		}
		uint JFrameUpdateData::GetTotalFrameCount(const J_UPLOAD_FRAME_RESOURCE_TYPE type)
		{ 
			//만약 frame holderVec[index].ptr이 nullptr 이면
			//memory extend시에 호출되는 NotifyReAlloc에서 ReRegister를 호출하지 않았는지 확인해보자.
			const uint registeredCount = GetTotalRegistedCount(type);
			return registeredCount == 0 ? 0 : (holderVec[(int)type][registeredCount - 1].ptr->index + holderVec[(int)type][registeredCount - 1].ptr->indexSize);
		}
		uint JFrameUpdateData::GetAreaRegistedCount(const J_UPLOAD_FRAME_RESOURCE_TYPE type, const size_t areaGuid)
		{
			int index = GetAreaVecIndex(type, areaGuid);
			return index != invalidIndex ? areaInfoVec[(int)type][index]->holderCount : 0;
		}
		uint JFrameUpdateData::GetAreaRegistedOffset(const J_UPLOAD_FRAME_RESOURCE_TYPE type, const size_t areaGuid)
		{
			int result = GetAreaStIndex(type, areaGuid);
			return result == invalidIndex ? 0 : result;
		}
		void JFrameUpdateData::SetUploadIndex(const int value) noexcept
		{
			index = value;
		}
		void JFrameUpdateData::SetMovedDirty()noexcept
		{
			movedDataDirty = Constants::gNumFrameResources;
		}
		void JFrameUpdateData::MinusMovedDirty()noexcept
		{
			--movedDataDirty;
			if (movedDataDirty < 0)
				movedDataDirty = 0;
		}
		bool JFrameUpdateData::HasValidFrameIndex()const noexcept
		{
			return index != invalidIndex;
		}
		bool JFrameUpdateData::HasMovedDirty()const noexcept
		{
			return movedDataDirty;
		}
		void JFrameUpdateData::RegisterFrameData(const J_UPLOAD_FRAME_RESOURCE_TYPE type,
			JFrameUpdateData* holder,
			const size_t areaGuid,
			const uint indexSize,
			const uint8 sortOrder)
		{
			if (holder == nullptr || holder->HasValidFrameIndex() || indexSize < 1)
				return;

			holder->indexSize = indexSize;
			holder->sortOrder = sortOrder;

			const int areaSt = GetAreaStIndex(type, areaGuid);
			const int areaEd = GetAreaEdIndex(type, areaGuid, areaSt);
			if (areaEd == invalidIndex)
				PushBack(type, holder, areaGuid);
			else
				Insert(type, holder, areaGuid, areaSt, areaEd);
		}
		void JFrameUpdateData::DeRegisterFrameData(const J_UPLOAD_FRAME_RESOURCE_TYPE type, JFrameUpdateData* holder)
		{
			if (holder == nullptr || !holder->HasValidFrameIndex())
				return;

			Pop(type, holder);
		}
		void JFrameUpdateData::ReRegisterFrameData(const J_UPLOAD_FRAME_RESOURCE_TYPE type, JFrameUpdateData* holder)
		{
			//check is registed
			if (holder == nullptr || !holder->HasValidFrameIndex())
				return;

			const int index = holder->number;
			holderVec[(int)type][index].ptr = holder;
		}
		bool JFrameUpdateData::PushBack(const J_UPLOAD_FRAME_RESOURCE_TYPE type, JFrameUpdateData* holder, const size_t areaGuid)
		{
			FrameUpdateHolderVec& vec = holderVec[(int)type];
			AreaInfoVec& areaVec = areaInfoVec[(int)type];
			const int count = vec.size();
 
			if (count == 0)
			{
				holder->index = 0;
				holder->number = 0;
			}
			else
			{
				JFrameUpdateData* ptr = vec[count - 1].ptr;
				holder->index = ptr->index + ptr->indexSize;
				holder->number = ptr->number + 1;
			}
			areaVec.push_back(std::make_unique<JFrameUpdateAreaInfo>(areaGuid, 1)); 
			vec.push_back(JFrameUpdateIndexHolderInfo(holder, areaVec[areaVec.size() - 1].get()));
			return true;
		}
		bool JFrameUpdateData::Insert(const J_UPLOAD_FRAME_RESOURCE_TYPE type,
			JFrameUpdateData* holder,
			const size_t areaGuid,
			const int areaStIndex,
			const int areaEdIndex)
		{
			FrameUpdateHolderVec& vec = holderVec[(int)type];
			AreaInfoVec& areaVec = areaInfoVec[(int)type];
 
			int insertIndex = -1;
			if (holder->sortOrder != UCHAR_MAX)
			{
				for (int i = areaStIndex; i <= areaEdIndex; ++i)
				{
					if (vec[i].ptr->sortOrder > holder->sortOrder)
					{
						insertIndex = i;
						break;
					}
				}
			}
			//holder->sortOrder == -1 or holder->sortOrder is last
			if (insertIndex == invalidIndex)
				insertIndex = areaEdIndex + 1;

			JFrameUpdateIndexHolderInfo& insertHolder = vec[insertIndex - 1];
			holder->index = insertHolder.ptr->index + insertHolder.ptr->indexSize;
			holder->number = insertHolder.ptr->number + 1;
			 
			JFrameUpdateAreaInfo* areaInfo = insertHolder.areaInfo;
			vec.insert(vec.begin() + insertIndex, JFrameUpdateIndexHolderInfo(holder, areaInfo));
			++areaInfo->holderCount;

			const int count = (int)vec.size();
			for (int i = insertIndex + 1; i < count; ++i)
			{
				vec[i].ptr->index += holder->indexSize;
				++vec[i].ptr->number;
				vec[i].ptr->SetMovedDirty(); 
			} 
			return true;
		}
		bool JFrameUpdateData::Pop(const J_UPLOAD_FRAME_RESOURCE_TYPE type, JFrameUpdateData* holder)
		{
			//const int arrayIndex = GetArrayIndex(type, holder); 
			const int arrayIndex = holder->number;

			FrameUpdateHolderVec& vec = holderVec[(int)type];
			AreaInfoVec& areaVec = areaInfoVec[(int)type];

			JFrameUpdateAreaInfo* areaInfo = vec[arrayIndex].areaInfo;
			const size_t areaGuid = areaInfo->guid;

			vec.erase(vec.begin() + arrayIndex); 
			--areaInfo->holderCount;
			if (areaInfo->holderCount <= 0)
				areaVec.erase(areaVec.begin() + GetAreaVecIndex(type, areaGuid));

			const int indexSize = holder->indexSize;
			const int count = (int)vec.size();
			for (int i = arrayIndex; i < count; ++i)
			{
				vec[i].ptr->index -= indexSize;
				--vec[i].ptr->number;
				vec[i].ptr->SetMovedDirty();
			}
			holder->index = invalidIndex;
			holder->indexSize = invalidIndex;
			return true;
		}

		void JFrameDirtyTrigger::SetFrameDirty()noexcept {}

		int JFrameDirty::GetFrameDirty()noexcept
		{
			return frameDirty;
		}
		int JFrameDirty::GetFrameDirtyMax()const noexcept
		{
			return Constants::gNumFrameResources;
		}
		void JFrameDirty::SetFrameDirty()noexcept
		{
			frameDirty = GetFrameDirtyMax();
		}
		void JFrameDirty::SetLastFrameHotUpdatedTrigger(const bool value)noexcept
		{
			isLastHotUpdated = value;
		}
		void JFrameDirty::SetLastFrameUpdatedTrigger(const bool value)noexcept
		{
			isLastUpdated = value;
		}
		bool JFrameDirty::IsFrameDirted()const noexcept
		{
			return frameDirty;
		}
		bool JFrameDirty::IsFrameHotDirted()const noexcept
		{
			return frameDirty == GetFrameDirtyMax();
		}
		bool JFrameDirty::IsLastFrameHotUpdated()const noexcept
		{
			return isLastHotUpdated;
		}
		bool JFrameDirty::IsLastFrameUpdated()const noexcept
		{
			return isLastUpdated;
		}
		void JFrameDirty::MinusFrameDirty()noexcept
		{
			--frameDirty;
			if (frameDirty < 0)
				frameDirty = 0;
		}
		void JFrameDirty::OffFrameDirty()noexcept
		{
			frameDirty = 0;
		}
	}
}