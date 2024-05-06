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
		class JFSMstate; 
		class JFSMparameter;
		class JFSMcondition;
		class JFSMdiagram;
		class JFSMdiagramOwnerInterface;
		class JFSMdiagramPrivate : public JFSMinterfacePrivate
		{
		public:
			class CreateInstanceInterface : public JFSMinterfacePrivate::CreateInstanceInterface
			{ 
			private:
				JOwnerPtr<JIdentifier> Create(JDITypeDataBase* initData)override;
			protected:
				void Initialize (JIdentifier* createdPtr, JDITypeDataBase* initData)noexcept override;
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
				friend class JFSMstate;
			private:
				static bool AddState(const JUserPtr<JFSMstate>& state)noexcept;
				static bool RemoveState(const JUserPtr<JFSMstate>& state)noexcept;
			};
			class OwnerTypeInterface
			{
			private:
				friend class JFSMdiagramOwnerInterface;
			private:
				static void SetOwnerPointer(const JUserPtr<JFSMdiagram>& digram, JFSMdiagramOwnerInterface* ownInterface)noexcept;
			};
			class ParamInterface
			{
			private:
				friend class JFSMcondition;
			private:
				static std::vector<JUserPtr<JFSMparameter>> GetStorageParameter(const JUserPtr<JFSMdiagram>& diagram)noexcept;
			};
		public:
			JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept override;
			JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept override;
		};
	}
}