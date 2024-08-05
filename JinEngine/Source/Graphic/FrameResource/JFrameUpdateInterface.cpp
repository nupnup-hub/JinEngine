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


#include"JFrameUpdateInterface.h" 

namespace JinEngine
{
	namespace Graphic
	{ 
		bool JFrameDirtyBase::IsFrameHotDirted()const noexcept
		{
			return  GetFrameDirtyMax() > 0 && GetFrameDirty() == (GetFrameDirtyMax());
		}
		bool JFrameDirtyBase::IsLastFrameHotUpdated()const noexcept
		{
			return GetFrameDirty() == (GetFrameDirtyMax() - 1);
		}

		int JFrameDirtyTrigger::GetFrameDirty()const noexcept
		{
			return 0;
		}
		int JFrameDirtyTrigger::GetFrameDirtyMax()const noexcept
		{
			return 0;
		}
		void JFrameDirtyTrigger::SetFrameDirty()noexcept
		{ 
		}
		bool JFrameDirtyTrigger::IsFrameDirted()const noexcept
		{
			return false;
		}
		void JFrameDirtyTrigger::MinusFrameDirty()noexcept
		{

		}
		void JFrameDirtyTrigger::OffFrameDirty()noexcept
		{

		}


		int JFrameDirty::GetFrameDirty()const noexcept
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
		bool JFrameDirty::IsFrameDirted()const noexcept
		{
			return frameDirty;
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

		bool JFrameUpdateInterface::Register(const JFrameUploadDataCreationDesc& desc)
		{

		}
		void JFrameUpdateInterface::DeRegister()
		{

		}
		int JFrameUpdateInterface::GetNumber(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept
		{
			auto user = GetFrameInfo(type);
			return user != nullptr ? user->GetNumber() : invalidIndex;
		}
		int JFrameUpdateInterface::GetFrameIndex(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept
		{
			auto user = GetFrameInfo(type);
			return user != nullptr ? user->GetFrameIndex() : invalidIndex;
		}
		int JFrameUpdateInterface::GetFrameIndexSize(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept
		{
			auto user = GetFrameInfo(type);
			return user != nullptr ? user->GetFrameIndexSize() : invalidIndex;
		}
		int JFrameUpdateInterface::GetLocalFrameCount(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept
		{
			auto user = GetFrameInfo(type);
			return user != nullptr ? user->GetAreaInfo()->GetInfoCount() : invalidIndex;
		}
		void JFrameUpdateInterface::SetFrameDirty()noexcept
		{
			auto dirtyBase = GetDirtyBase();
			if (dirtyBase == nullptr)
				return;

			dirtyBase->SetFrameDirty();
		}
		bool JFrameUpdateInterface::IsDirted()const noexcept
		{
			auto dirtyBase = GetDirtyBase();
			if (dirtyBase == nullptr)
				return false;

			return dirtyBase->IsFrameDirted();
		}
		void JFrameUpdateInterface::OffFrameDirty()noexcept
		{
			auto dirtyBase = GetDirtyBase();
			if (dirtyBase == nullptr)
				return;

			dirtyBase->OffFrameDirty();
		}
		void JFrameUpdateInterface::TryExecuteObjectUpdateBind()
		{
			if (objectUpdateB == nullptr)
				return;

			objectUpdateB->InvokeCompletelyBind();
		}
		bool JFrameUpdateInterface::RegisterObjectUpdateB(JFrameObjectUpdateB&& bind)
		{
			objectUpdateB = std::move(bind);
			return true;
		}
		bool JFrameUpdateInterface::DeRegisterObjectUpdateB()
		{
			objectUpdateB = nullptr;
			return true;
		}
	}
}
