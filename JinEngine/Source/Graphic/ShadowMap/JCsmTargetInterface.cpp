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


#include"JCsmTargetInterface.h"
#include"JCsmTargetInfo.h" 
#include"../JGraphicPrivate.h"
#include"../../Core/Guid/JGuidCreator.h"

namespace JinEngine::Graphic
{
	void JCsmTargetInterface::AddCsmTargetInfo(const JUserPtr<JCsmTargetInfo>& info)
	{ 
		info->SetFrustumPtr(GetCsmTargetBoundingFrustumF::Functor(&JCsmTargetInterface::GetBoundingFrustum, this));
		infoVec.push_back(info);
	}
	void JCsmTargetInterface::RemoveCsmTargetInfo(const JUserPtr<JCsmTargetInfo>& info)
	{
		int index; 
		if (GetTargetInfo(info->GetHandlerGuid(), index) == nullptr)
			return;
			 
		infoVec.erase(infoVec.begin() + index);
	}
	void JCsmTargetInterface::TrySetGetFrustumPtr()noexcept
	{ 
		for (auto& data : infoVec)
			data->SetFrustumPtr(GetCsmTargetBoundingFrustumF::Functor(&JCsmTargetInterface::GetBoundingFrustum, this));
	}
	int JCsmTargetInterface::GetCsmTargetIndex()const noexcept
	{  
		return targetIndex;
	}
	JUserPtr<JCsmTargetInfo> JCsmTargetInterface::GetTargetInfo(const size_t handlerGuid)const noexcept
	{
		int index;
		return GetTargetInfo(handlerGuid, index);
	} 
	JUserPtr<JCsmTargetInfo> JCsmTargetInterface::GetTargetInfo(const size_t handlerGuid, _Out_ int& index)const noexcept
	{
		const uint count = (uint)infoVec.size();
		for (uint i = 0; i < count; ++i)
		{
			if (infoVec[i]->GetHandlerGuid() == handlerGuid)
			{
				index = i;
				return infoVec[i];
			}
		}
		index = -1;
		return nullptr;
	}
	bool JCsmTargetInterface::HasCsmTargetInfo()const noexcept
	{
		return infoVec.size() > 0;
	}
	bool JCsmTargetInterface::RegisterCsmTargetInterface()
	{
		return JGraphicPrivate::CsmInterface::RegisterTarget(this);
	}
	bool JCsmTargetInterface::DeRegisterCsmTargetInterface()
	{
		const bool res = JGraphicPrivate::CsmInterface::DeRegisterTarget(this);
		infoVec.clear();
		return res;
	}
}