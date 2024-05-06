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
#include"../JResourceObject.h"  
#include"../../../Core/Guid/JGuidCreator.h"

namespace JinEngine
{
	namespace Core
	{
		class JFSMparameter; 
	}

	class JAnimationFSMdiagram;
	class JAnimationFSMstate;
	class JAnimationFSMtransition;
	class JAnimationControllerPrivate;
	class JAnimationController final : public JResourceObject
	{
		REGISTER_CLASS_IDENTIFIER_LINE_RESOURCE(JAnimationController) 
	public: 
		class InitData final : public JResourceObject::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData) 
		private:
			friend class JAnimationController;
		private:
			bool makeDiagram = true;
		public:
			InitData(const uint8 formatIndex, const JUserPtr<JDirectory>& directory);
			InitData(const size_t guid,
				const uint8 formatIndex,
				const JUserPtr<JDirectory>& directory);
			InitData(const std::wstring& name,
				const size_t guid,
				const J_OBJECT_FLAG flag,
				const uint8 formatIndex,
				const JUserPtr<JDirectory>& directory);
		};
	private:
		friend class JAnimationControllerPrivate;
		class JAnimationControllerImpl;
	private:
		std::unique_ptr<JAnimationControllerImpl> impl;
	public:
		Core::JIdentifierPrivate& PrivateInterface()const noexcept final;
		J_RESOURCE_TYPE GetResourceType()const noexcept final;
		static constexpr J_RESOURCE_TYPE GetStaticResourceType()noexcept
		{
			return J_RESOURCE_TYPE::ANIMATION_CONTROLLER;
		}
		std::wstring GetFormat()const noexcept final;
		static std::vector<std::wstring> GetAvailableFormat()noexcept;
		uint GetDiagramCount()const noexcept; 
		uint GetParameterCount()const noexcept;   
		JUserPtr<JAnimationFSMdiagram> GetDiagram(const size_t guid)noexcept;
		JUserPtr<JAnimationFSMdiagram> GetDiagramByIndex(const uint index)noexcept;
		const std::vector<JUserPtr<JAnimationFSMdiagram>>& GetDiagramVec()noexcept;
		JUserPtr<Core::JFSMparameter> GetParameter(const size_t guid)noexcept;
		JUserPtr<Core::JFSMparameter> GetParameterByIndex(const uint index)noexcept;
	public:
		bool CanCreateDiagram()const noexcept;
		bool CanCreateParameter()const noexcept;
		bool CanCreateState(JAnimationFSMdiagram* diagram)const noexcept;
		bool IsValid()const noexcept final;
	public:
		JUserPtr<JAnimationFSMdiagram> CreateFSMdiagram(const size_t guid = Core::MakeGuid())noexcept;
		JUserPtr<Core::JFSMparameter> CreateFSMparameter(const size_t guid = Core::MakeGuid())noexcept;
		JUserPtr<JAnimationFSMstate> CreateFSMclip(const JUserPtr<JAnimationFSMdiagram>& diagram, const size_t guid = Core::MakeGuid())noexcept;
		JUserPtr<JAnimationFSMtransition> CreateFsmtransition(const JUserPtr<JAnimationFSMdiagram>& diagram,
			const JUserPtr<JAnimationFSMstate>& from,
			const JUserPtr<JAnimationFSMstate>& to,
			const size_t guid = Core::MakeGuid())noexcept;   
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;  
	private:
		JAnimationController(const InitData& initData);
		~JAnimationController();
	};
}