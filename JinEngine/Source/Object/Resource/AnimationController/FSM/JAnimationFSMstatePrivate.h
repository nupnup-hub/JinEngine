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
#include"../../../../Core/FSM/JFSMstatePrivate.h"
#include"../../../../Core/File/JFileIOResult.h"
#include"../../../../Core/Math/JVector.h" 

namespace JinEngine
{
	class JSkeletonAsset;
	class JAnimationFSMdiagram;
	class JAnimationFSMstate;
	class JAnimationUpdateData;

	class JFileIOTool;
	class JAnimationFSMstatePrivate : public Core::JFSMstatePrivate
	{
	public:
		class AssetDataIOInterface
		{
		private:
			friend class JAnimationFSMdiagram;
		private:
			virtual Core::J_FILE_IO_RESULT LoadAssetData(JFileIOTool& tool, const JUserPtr<JAnimationFSMstate>& state) = 0;	//load state data and create transition
			virtual Core::J_FILE_IO_RESULT StoreAssetData(JFileIOTool& tool, const JUserPtr<JAnimationFSMstate>& state) = 0;
		protected:
			static Core::J_FILE_IO_RESULT LoadAssetCommonData(JFileIOTool& tool, const JUserPtr<JAnimationFSMstate>& state);
			static Core::J_FILE_IO_RESULT StoreAssetCommonData(JFileIOTool& tool, const JUserPtr<JAnimationFSMstate>& state);
		};
		class CreateInstanceInterface : public JFSMstatePrivate::CreateInstanceInterface
		{
		protected:
			void Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept override;
		};
		class UpdateInterface : public JFSMstatePrivate::UpdateInterface
		{
		private:
			friend class JAnimationFSMdiagram;
		private:
			virtual void Enter(const JUserPtr<JAnimationFSMstate>& state, JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept = 0;
			virtual void Update(const JUserPtr<JAnimationFSMstate>& state, JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept = 0;
			virtual void Close(const JUserPtr<JAnimationFSMstate>& state, JAnimationUpdateData* updateData)noexcept = 0;
			virtual void GetRegisteredSkeleton(const JUserPtr<JAnimationFSMstate>& state, std::vector<JUserPtr<JSkeletonAsset>>& skeletonVec)noexcept = 0;
		};
		class EditorInterface
		{
		private:
			static JVector2<float> GetPos(const JUserPtr<JAnimationFSMstate>& state) noexcept;
			static void SetPos(const JUserPtr<JAnimationFSMstate>& state, const JVector2<float>& newPos)noexcept;
		};
	public:
		virtual JAnimationFSMstatePrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept = 0;
	};
}