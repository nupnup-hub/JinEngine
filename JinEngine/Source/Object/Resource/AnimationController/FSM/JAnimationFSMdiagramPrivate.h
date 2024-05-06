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
#include "../../../../Core/FSM/JFSMdiagramPrivate.h" 
#include"../../../../Core/File/JFileIOResult.h"

namespace JinEngine
{
	class JAnimationController;
	namespace Graphic
	{
		struct JAnimationConstants;
	}
	namespace Core
	{
		class JFSMdiagramOwnerInterface;
	}
	class JAnimationUpdateData;
	class JAnimationFSMdiagram;
	class JFileIOTool;
	class JAnimationFSMdiagramPrivate final : public Core::JFSMdiagramPrivate
	{
	public:
		class AssetDataIOInterface
		{
		private:
			friend class JAnimationController;
		private:
			static JUserPtr<JAnimationFSMdiagram> LoadAssetData(JFileIOTool& tool, Core::JFSMdiagramOwnerInterface* fsmOwner);	//load diagram data and create state
			static Core::J_FILE_IO_RESULT StoreAssetData(JFileIOTool& tool, const JUserPtr<JAnimationFSMdiagram>& diagram);
		};
		class CreateInstanceInterface final : public Core::JFSMdiagramPrivate::CreateInstanceInterface
		{
		private:
			JOwnerPtr<Core::JIdentifier> Create(Core::JDITypeDataBase* initData)final;
			void Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept final;
			bool CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept final;
		};
		class UpdateInterface final
		{
		private:
			friend class JAnimationController;
		private:
			static void Initialize(const JUserPtr<JAnimationFSMdiagram>& diagram, JAnimationUpdateData* updateData, const uint layerNumber)noexcept;
			static void Enter(const JUserPtr<JAnimationFSMdiagram>& diagram, JAnimationUpdateData* updateData, const uint layerNumber);
			static void Update(const JUserPtr<JAnimationFSMdiagram>& diagram, JAnimationUpdateData* updateData, Graphic::JAnimationConstants& animationConstatns, const uint layerNumber)noexcept;
		};
	public:
		Core::JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
	};
}