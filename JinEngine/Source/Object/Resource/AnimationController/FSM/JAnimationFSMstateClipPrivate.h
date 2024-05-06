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
#include"JAnimationFSMstatePrivate.h" 

namespace JinEngine
{  
	class JAnimationFSMstateClipPrivate : public JAnimationFSMstatePrivate
	{
	public:
		class AssetDataIOInterface final : public JAnimationFSMstatePrivate::AssetDataIOInterface
		{
		private:
			Core::J_FILE_IO_RESULT LoadAssetData(JFileIOTool& tool, const JUserPtr<JAnimationFSMstate>& state) final;
			Core::J_FILE_IO_RESULT StoreAssetData(JFileIOTool& tool, const JUserPtr<JAnimationFSMstate>& state) final;
		};
		class CreateInstanceInterface final : public JAnimationFSMstatePrivate::CreateInstanceInterface
		{
		private:
			JOwnerPtr<Core::JIdentifier> Create(Core::JDITypeDataBase* initData)final;
			void Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept final;
			bool CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept final;
		};
		class DestroyInstanceInterface final : public JAnimationFSMstatePrivate::DestroyInstanceInterface
		{
		protected:
			void Clear(Core::JIdentifier* ptr, const bool isForced)final;
		};
		class UpdateInterface final : public JAnimationFSMstatePrivate::UpdateInterface
		{
		private:
			void Initialize(const JUserPtr<Core::JFSMstate>& state)noexcept final;
			void Enter(const JUserPtr<JAnimationFSMstate>& state, JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept final;
			void Update(const JUserPtr<JAnimationFSMstate>& state, JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept final;
			void Close(const JUserPtr<JAnimationFSMstate>& state, JAnimationUpdateData* updateData)noexcept final;
			void GetRegisteredSkeleton(const JUserPtr<JAnimationFSMstate>& state, std::vector<JUserPtr<JSkeletonAsset>>& skeletonVec)noexcept final;
		};
	public:
		JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
		JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept final;
		JAnimationFSMstatePrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept final;
		JFSMstatePrivate::UpdateInterface& GetUpdateInterface()const noexcept final;
	};
}