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


#include"JCullingManager.h"
#include"JCullingInfo.h"  
#include"JCullingResultHolder.h" 
#include"../Device/JGraphicDevice.h"
namespace JinEngine
{
	namespace Graphic
	{	 		
		/*	
		* create resource senario
		1. Create resource and info(Owner Pointer)
		2. caller(impl) is derived interface(has cinfo and access cmanager public method)
		3. impl interface class can derived user access class(restrict version impl derived interface)
		*/
		JCullingManager::~JCullingManager()
		{
			ClearResource();
		}
		uint JCullingManager::GetCullingInfoCount(const J_CULLING_TYPE type)const noexcept
		{
			return (uint)cullingInfo[(uint)type].size();
		}
		JUserPtr<JCullingInfo> JCullingManager::GetCullingInfo(const J_CULLING_TYPE type, const uint index)const noexcept
		{
			return cullingInfo[(uint)type].size() > index ? JUserPtr<JCullingInfo>{cullingInfo[(uint)type][index]} : nullptr;
		}  
		std::vector<JUserPtr<JCullingInfo>> JCullingManager::GetCullingInfoVec(const J_CULLING_TYPE type)const noexcept
		{
			const uint count = (uint)cullingInfo[(uint)type].size();
			std::vector<JUserPtr<JCullingInfo>> user(count);
			for (uint i = 0; i < count; ++i)
				user[i] = cullingInfo[(uint)type][i];
			return user;
		}
		JCullingResultHolder* JCullingManager::GetHolder(JCullingInfo* info)const noexcept
		{
			return info != nullptr ? info->resultHolder.get() : nullptr;
		}
		std::vector<JOwnerPtr<JCullingInfo>>& JCullingManager::GetCullinginfoRefVec(const J_CULLING_TYPE type)noexcept
		{
			return cullingInfo[(uint)type];
		}
		void JCullingManager::ReBuildBuffer(const J_CULLING_TYPE type, JGraphicDevice* device, const size_t capacity)
		{
			auto& typeVec = cullingInfo[(uint)type];
			const uint count = (uint)cullingInfo[(uint)type].size();
			for (uint i = 0; i < count; ++i)
				ReBuildBuffer(typeVec[i].Get(), device, capacity, i);		
		}
		void JCullingManager::ReBuildBuffer(const J_CULLING_TYPE type, JGraphicDevice* device, const size_t capacity, const J_CULLING_TARGET bufferTarget)
		{
			auto& typeVec = cullingInfo[(uint)type];
			const uint count = (uint)cullingInfo[(uint)type].size();
			for (uint i = 0; i < count; ++i)
			{
				if(Core::HasSQValueEnum(typeVec[i]->GetCullingTarget(), bufferTarget))
					ReBuildBuffer(typeVec[i].Get(), device, capacity, i);
			}
		}
		JUserPtr<JCullingInfo> JCullingManager::PostCreation(JOwnerPtr<JCullingInfo>&& newInfo, const J_CULLING_TYPE type)
		{
			const uint infoIndex = (uint)cullingInfo[(uint)type].size();
			newInfo->SetArrayIndex(infoIndex);
			cullingInfo[(uint)type].push_back(std::move(newInfo));
			return cullingInfo[(uint)type][infoIndex];
		}
		bool JCullingManager::DestroyCullingData(JCullingInfo* info, JGraphicDevice* device)
		{
			if (info == nullptr)
				return false;

			bool startCommandThisCreation = false;
			if (device->CanStartPublicCommand())
			{
				device->FlushCommandQueue();
				device->StartPublicCommand();
				startCommandThisCreation = true;
			}

			const J_CULLING_TYPE type = info->GetCullingType();
			const uint count = (uint)cullingInfo[(uint)type].size();

			const uint index = info->GetArrayIndex();
			for (uint i = index + 1; i < count; ++i)
				cullingInfo[(uint)type][i]->SetArrayIndex(i - 1);

			cullingInfo[(uint)type].erase(cullingInfo[(uint)type].begin() + index);
			info = nullptr;

			PostDestruction(type, index);
			if (startCommandThisCreation)
			{
				device->EndPublicCommand();
				device->FlushCommandQueue();
			}
			return true;
		} 
		void JCullingManager::Initialize(JGraphicDevice* device)
		{ 
		}
		void JCullingManager::Clear()
		{
			ClearResource();
		}
		void JCullingManager::ClearResource()
		{
			for (uint i = 0; i < (uint)J_CULLING_TYPE::COUNT; ++i)
				cullingInfo[i].clear();
		}
		void JCullingManager::RegisterTypeData()
		{}
	}
}