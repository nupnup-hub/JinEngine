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
#include"../../../../Core/FSM/JFSMtransitionPrivate.h"
#include"../../../../Core/File/JFileIOResult.h" 

namespace JinEngine
{
	class JAnimationFSMstate;
	class JAnimationFSMstatePrivate;
	class JAnimationFSMtransition;
	class JFileIOTool;
	class JAnimationFSMtransitionPrivate final : public Core::JFSMtransitionPrivate
	{
	public:
		class AssetDataIOInterface final
		{
		private:
			friend class JAnimationFSMstate;
			friend class JAnimationFSMstatePrivate;
		private:
			Core::J_FILE_IO_RESULT LoadAssetData(JFileIOTool& tool, const JUserPtr<JAnimationFSMtransition>& trans);
			Core::J_FILE_IO_RESULT StoreAssetData(JFileIOTool& tool, const JUserPtr<JAnimationFSMtransition>& trans);
		};
		class CreateInstanceInterface final : public JFSMtransitionPrivate::CreateInstanceInterface
		{
		private:
			JOwnerPtr<Core::JIdentifier> Create(Core::JDITypeDataBase* initData)final;
			void Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept final;
			bool CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept final;
		};
		class UpdateInterface final : public JFSMtransitionPrivate::UpdateInterface
		{
		private:
			void Initialize(const JUserPtr<Core::JFSMtransition>& trans)noexcept final;
		};
	public:
		JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
		JAnimationFSMtransitionPrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept;
		JFSMtransitionPrivate::UpdateInterface& GetUpdateInterface()const noexcept final;
	};
}