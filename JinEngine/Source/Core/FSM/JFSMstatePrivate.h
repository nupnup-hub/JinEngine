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
#include"JFSMinterfacePrivate.h"

namespace JinEngine
{
	namespace Core
	{
		class JFSMdiagram;
		class JFSMstate;
		class JFSMtransition;
		class JFSMstatePrivate : public JFSMinterfacePrivate
		{
		public:
			class CreateInstanceInterface : public JFSMinterfacePrivate::CreateInstanceInterface
			{
			private:
				JOwnerPtr<JIdentifier> Create(JDITypeDataBase* initData)override;
			protected:
				void Initialize(JIdentifier* createdPtr, JDITypeDataBase* initData)noexcept override;
			private:
				void RegisterCash(JIdentifier* createdPtr)noexcept override;
				bool CanCreateInstance(JDITypeDataBase* initData)const noexcept override;
			};
			class DestroyInstanceInterface : public JIdentifierPrivate::DestroyInstanceInterface
			{
			protected:
				void Clear(JIdentifier* ptr, const bool isForced)override;
			private:
				void DeRegisterCash(JIdentifier* ptr)noexcept override;
			};
			class OwnTypeInterface
			{
			private:
				friend class JFSMtransition;
			private:
				static bool AddTransition(const JUserPtr<JFSMtransition>& transition)noexcept;
				static bool RemoveTransition(const JUserPtr<JFSMtransition>& transition)noexcept;
			};
			class UpdateInterface 
			{
			private:
				friend class JFSMdiagram;
			private:
				virtual void Initialize(const JUserPtr<JFSMstate>& state)noexcept;
			};
			class ParameterInterface
			{
			private:
				friend class JFSMdiagram;
			private:
				static void RemoveParameter(const JUserPtr<JFSMstate>& state, const size_t guid);
			};
		public:
			JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept override;
			JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept override;
			virtual JFSMstatePrivate::UpdateInterface& GetUpdateInterface() const noexcept;
		};
	}
}