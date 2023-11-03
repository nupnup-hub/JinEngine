#include"JFrameUpdate.h"
#include"../JGraphicPrivate.h"

namespace JinEngine
{
	namespace Graphic
	{
		namespace
		{
			struct JFrameUpdateIndexHolderInfo
			{
			public:
				JFrameUpdateData* ptr;
				size_t areaGuid; 
			public:
				JFrameUpdateIndexHolderInfo(JFrameUpdateData* ptr, const size_t areaGuid)
					:ptr(ptr), areaGuid(areaGuid)
				{}
			};
			static std::vector<JFrameUpdateIndexHolderInfo> holderVec[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::COUNT];
		 
			static int GetAreaStIndex(const J_UPLOAD_FRAME_RESOURCE_TYPE type, const size_t areaGuid)
			{  
				auto& vec = holderVec[(int)type];
				const int count = (int)vec.size();
				for (int i = 0; i < count; ++i)
				{
					if (vec[i].areaGuid == areaGuid)
						return i;
				}
				return invalidIndex;
			}
			static int GetAreaEdIndex(const J_UPLOAD_FRAME_RESOURCE_TYPE type, const size_t areaGuid, const int areaSt)
			{
				if (areaSt != invalidIndex)
				{
					auto& vec = holderVec[(int)type];
					const int count = (int)vec.size();
					for (int i = areaSt; i < count; ++i)
					{
						if (vec[i].areaGuid != areaGuid)
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
				auto& vec = holderVec[(int)type];
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
			return holderVec[(int)type].size();
		}
		uint JFrameUpdateData::GetTotalFrameCount(const J_UPLOAD_FRAME_RESOURCE_TYPE type)
		{
			const uint registeredCount = GetTotalRegistedCount(type);
			return registeredCount == 0 ? 0 : holderVec[(int)type][registeredCount - 1].ptr->index + holderVec[(int)type][registeredCount - 1].ptr->indexSize;
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
			const ushort indexSize,
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
			auto& vec = holderVec[(int)type];
			const int count = vec.size(); 

			if (count == 0)
			{
				holder->index = 0;
				holder->number = 0;
			}
			else
			{
				auto ptr = vec[count - 1].ptr;
				holder->index = ptr->index + ptr->indexSize;
				holder->number = ptr->number + 1;
			} 
			vec.emplace_back(JFrameUpdateIndexHolderInfo(holder, areaGuid));
			return true;
		}
		bool JFrameUpdateData::Insert(const J_UPLOAD_FRAME_RESOURCE_TYPE type,
			JFrameUpdateData* holder,
			const size_t areaGuid,
			const int areaStIndex,
			const int areaEdIndex)
		{ 
			auto& vec = holderVec[(int)type]; 		 
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

			auto& insertHolder = vec[insertIndex - 1];
			holder->index = insertHolder.ptr->index + insertHolder.ptr->indexSize;
			holder->number = insertHolder.ptr->number + 1;

			vec.insert(vec.begin() + insertIndex, JFrameUpdateIndexHolderInfo(holder, areaGuid));

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
			const int arrayIndex = GetArrayIndex(type, holder); 
			auto& vec = holderVec[(int)type]; 
			vec.erase(vec.begin() + arrayIndex);

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
		void JFrameDirty::SetFrameDirty()noexcept
		{
			frameDirty = Graphic::Constants::gNumFrameResources;
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