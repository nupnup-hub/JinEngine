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


#include"JCullingInterface.h"
#include"JCullingInfo.h" 

namespace JinEngine
{
	namespace Graphic
	{ 
		int JCullingInterface::GetArrayIndex(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept
		{
			auto info = GetCullingInfo(type, target);
			return info != nullptr ? info->GetArrayIndex() : invalidIndex;
		}
		uint JCullingInterface::GetResultBufferSize(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept
		{
			auto info = GetCullingInfo(type, target);
			return info != nullptr ? info->GetResultBufferSize() : 0;
		}
		float JCullingInterface::GetUpdateFrequency(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept
		{
			auto info = GetCullingInfo(type, target);
			return info != nullptr ? info->GetUpdateFrequency() : 0;
		} 
		void JCullingInterface::SetCulling(const J_CULLING_TYPE type, const J_CULLING_TARGET target, const uint index)noexcept
		{
			auto info = GetCullingInfo(type, target);
			if (info == nullptr)
				return;

			info->Culling(index, true);
		}
		void JCullingInterface::OffCulling(const J_CULLING_TYPE type, const J_CULLING_TARGET target, const uint index)noexcept
		{
			auto info = GetCullingInfo(type, target);
			if (info == nullptr)
				return;

			info->Culling(index, false);
		}
		void JCullingInterface::OffCullingArray(const J_CULLING_TYPE type, const J_CULLING_TARGET target)noexcept
		{
			auto info = GetCullingInfo(type, target);
			if (info == nullptr)
				return;

			const uint count = (uint)info->GetResultBufferElementCount();
			for (uint i = 0; i < count; ++i)
				info->Culling(i, false);
		}
		bool JCullingInterface::IsCulled(const J_CULLING_TYPE type, const J_CULLING_TARGET target, const uint index)const noexcept
		{
			auto info = GetCullingInfo(type, target);
			return info != nullptr ? info->IsCulled(index) : false;
		}   
		bool JCullingInterface::IsUpdateEnd(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept
		{
			auto info = GetCullingInfo(type, target);
			return info != nullptr ? info->IsUpdateEnd() : true;
		}
		bool JCullingInterface::HasCullingData(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept
		{
			return GetCullingInfo(type, target) != nullptr;
		}

		/*
		void JCullingTypePerSingleTargetHolder::DestroyCullingData(const J_CULLING_TYPE type)noexcept
		{
			for (uint i = 0; i < (uint)J_CULLING_TARGET::COUNT; ++i)
				JCullingInterface::DestroyCullingData(info[(uint)type][i]);
		}
		*/ 
		void JCullingTypePerSingleTargetHolder::AddInfo(const JUserPtr<JCullingInfo>& newInfo)
		{
			const J_CULLING_TYPE newType = newInfo->GetCullingType();
			const J_CULLING_TARGET newTarget = newInfo->GetCullingTarget();
			info[(uint)newType][(uint)newTarget] = newInfo;
		}
		void JCullingTypePerSingleTargetHolder::RemoveInfo(DestoryInfoF& destroyF, const J_CULLING_TYPE type, const J_CULLING_TARGET target)noexcept
		{ 
			destroyF(info[(uint)type][(uint)target].Release());
		}
		void JCullingTypePerSingleTargetHolder::RemoveInfoOfType(DestoryInfoF& destroyF, const J_CULLING_TYPE type)noexcept
		{
			for (uint i = 0; i < (uint)J_CULLING_TARGET::COUNT; ++i)
				destroyF(info[(uint)type][i].Release());			 
		}
		void JCullingTypePerSingleTargetHolder::RemoveInfoAll(DestoryInfoF& destroyF)noexcept
		{
			for (uint i = 0; i < (uint)J_CULLING_TYPE::COUNT; ++i)
			{
				for (uint j = 0; j < (uint)J_CULLING_TARGET::COUNT; ++j)
					destroyF(info[i][j].Release());
			} 
		}
		JUserPtr<JCullingInfo> JCullingTypePerSingleTargetHolder::GetCullingInfo(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept
		{
			return info[(uint)type][(uint)target];
		}
		bool JCullingTypePerSingleTargetHolder::IsCulled(const J_CULLING_TARGET target, const uint index)const noexcept
		{ 
			return JCullingInterface::IsCulledT(target, index, info, std::make_index_sequence<(uint)J_CULLING_TYPE::COUNT>());
			//return (info[(uint)J_CULLING_TYPE::FRUSTUM][(uint)target] != nullptr ? info[(uint)J_CULLING_TYPE::FRUSTUM][(uint)target]->IsCulled(index) : false) ||
			//	(info[(uint)J_CULLING_TYPE::HZB_OCCLUSION][(uint)target] != nullptr ? info[(uint)J_CULLING_TYPE::HZB_OCCLUSION][(uint)target]->IsCulled(index) : false) ||
			//	(info[(uint)J_CULLING_TYPE::HD_OCCLUSION][(uint)target] != nullptr ? info[(uint)J_CULLING_TYPE::HD_OCCLUSION][(uint)target]->IsCulled(index) : false);
		}
		bool JCullingTypePerSingleTargetHolder::IsUpdateEnd(const J_CULLING_TYPE type)const noexcept
		{
			return JCullingInterface::IsUpdateEndT(type, info, std::make_index_sequence<(uint)J_CULLING_TARGET::COUNT>());
		}
		bool JCullingTypePerSingleTargetHolder::HasSpace(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept
		{
			return info[(uint)type][(uint)target] == nullptr;
		}
		bool JCullingTypePerSingleTargetHolder::IsValidType(const J_CULLING_TYPE type, const J_CULLING_TARGET target) const noexcept
		{
			return true;
		}
	}
}