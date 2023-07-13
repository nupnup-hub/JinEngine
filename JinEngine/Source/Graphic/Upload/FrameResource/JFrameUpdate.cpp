#include"JFrameUpdate.h"
#include"../../JGraphicPrivate.h"

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
				return -1;
			}
			static int GetAreaEdIndex(const J_UPLOAD_FRAME_RESOURCE_TYPE type, const size_t areaGuid)
			{
				int areaSt = GetAreaStIndex(type, areaGuid);
				if (areaSt != -1)
				{
					auto& vec = holderVec[(int)type];
					const int count = (int)vec.size();
					for (int i = areaSt; i < count; ++i)
					{
						if (vec[i].areaGuid != areaGuid)
							return i;
					}
				}
				return -1;
			}
		}
 
		int JFrameUpdateData::GetUploadIndex()const noexcept
		{
			return index;
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
		bool JFrameUpdateData::HasMovedDirty()const noexcept
		{
			return movedDataDirty;
		}
		void JFrameUpdateData::RegisterFrameData(const J_UPLOAD_FRAME_RESOURCE_TYPE type, JFrameUpdateData* holder, const size_t areaGuid, const uint indexSize)
		{
			if (holder->index != -1 || indexSize == -1)
				return;

			int areaEd = GetAreaEdIndex(type, areaGuid);
			if (areaEd == -1)
				PushBack(type, holder, areaGuid, indexSize);
			else
				Insert(type, holder, areaGuid, areaEd, indexSize);
		}
		void JFrameUpdateData::DeRegisterFrameData(const J_UPLOAD_FRAME_RESOURCE_TYPE type, JFrameUpdateData* holder)
		{
			if (holder->index == -1)
				return;

			Pop(type, holder);
		}
		void JFrameUpdateData::ReRegisterFrameData(const J_UPLOAD_FRAME_RESOURCE_TYPE type, JFrameUpdateData* holder)
		{
			if (holder->index == -1)
				return;

			const int index = holder->index;
			holderVec[(int)type][index].ptr = holder;
		}
		bool JFrameUpdateData::PushBack(const J_UPLOAD_FRAME_RESOURCE_TYPE type, JFrameUpdateData* holder, const size_t areaGuid, const uint indexSize)
		{
			auto& vec = holderVec[(int)type];
			const int count = vec.size();

			if (count == 0)
				holder->index = 0;
			else
			{
				auto ptr = vec[count - 1].ptr;
				holder->index = ptr->index + ptr->indexSize;
			}

			holder->indexSize = indexSize;
			vec.emplace_back(JFrameUpdateIndexHolderInfo(holder, areaGuid)); 
			return true;
		}
		bool JFrameUpdateData::Insert(const J_UPLOAD_FRAME_RESOURCE_TYPE type, JFrameUpdateData* holder, const size_t areaGuid, const int index, const uint indexSize)
		{
			auto& vec = holderVec[(int)type]; 
			auto prePtr = vec[index].ptr;

			//holder->index = prePtr->index + prePtr->indexSize;
			holder->index = prePtr->index;
			holder->indexSize = indexSize;
			holderVec[(int)type].insert(vec.begin() + index, JFrameUpdateIndexHolderInfo(holder, areaGuid));
			
			const int count = (int)vec.size();
			for (int i = index + 1; i < count; ++i)
			{
				vec[i].ptr->index += indexSize;
				vec[i].ptr->SetMovedDirty();
			}
			return true;
		}
		bool JFrameUpdateData::Pop(const J_UPLOAD_FRAME_RESOURCE_TYPE type, JFrameUpdateData* holder)
		{
			auto& vec = holderVec[(int)type];
			vec.erase(vec.begin() + holder->index);

			const int indexSize = holder->indexSize;
			const int count = (int)vec.size();
			for (int i = holder->index; i < count; ++i)
			{
				vec[i].ptr->index -= indexSize;
				vec[i].ptr->SetMovedDirty();
			}
			holder->index = -1;
			holder->indexSize = -1;
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