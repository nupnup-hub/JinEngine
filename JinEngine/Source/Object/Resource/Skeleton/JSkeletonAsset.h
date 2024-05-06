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
#include"JSkeletonType.h"
#include"Avatar/JAvatar.h"  
#include"../JResourceObject.h" 
#include"../../../Core/Animation/Joint.h"

namespace JinEngine
{
	struct JSkeleton;
	class JAvatar;
	class JSkeletonAssetPrivate;
	class JSkeletonAsset final : public JResourceObject
	{
		REGISTER_CLASS_IDENTIFIER_LINE_RESOURCE(JSkeletonAsset) 
	public: 
		class InitData final : public JResourceObject::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			std::vector<Core::Joint> joint;
			Core::JTypeInstanceSearchHint modelHint;
		public:
			InitData(const uint8 formatIndex,
				const JUserPtr<JDirectory>& directory,
				const Core::JTypeInstanceSearchHint& modelHint,
				std::vector<Core::Joint>&& joint);
			InitData(const size_t guid,
				const uint8 formatIndex,
				const JUserPtr<JDirectory>& directory,
				const Core::JTypeInstanceSearchHint& modelHint,
				std::vector<Core::Joint>&& joint);
			InitData(const std::wstring& name,
				const size_t guid,
				const J_OBJECT_FLAG flag,
				const uint8 formatIndex,
				const JUserPtr<JDirectory>& directory,
				const Core::JTypeInstanceSearchHint& modelHint,
				std::vector<Core::Joint>&& joint);
		public:
			bool IsValidData()const noexcept final;
		};
	protected: 
		class LoadMetaData final : public JResourceObject::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(LoadMetaData)
		public:
			JAvatar avatar;
			JSKELETON_TYPE skeletonType; 
			Core::JTypeInstanceSearchHint modelHint;
		public:
			bool isValidAvatar = false;
		public:
			LoadMetaData(const JUserPtr<JDirectory>& directory);
		};
	private:
		friend class JSkeletonAssetPrivate;
		class JSkeletonAssetImpl;
	private:
		std::unique_ptr<JSkeletonAssetImpl> impl;
	public:
		Core::JIdentifierPrivate& PrivateInterface()const noexcept final;
		J_RESOURCE_TYPE GetResourceType()const noexcept final;
		static constexpr J_RESOURCE_TYPE GetStaticResourceType()noexcept
		{
			return J_RESOURCE_TYPE::SKELETON;
		}
		std::wstring GetFormat()const noexcept final;
		static std::vector<std::wstring> GetAvailableFormat()noexcept;
	public:
		JUserPtr<JSkeleton> GetSkeleton()const noexcept;
		JUserPtr<JAvatar> GetAvatar()const noexcept;
		JSKELETON_TYPE GetSkeletonType()const noexcept; 
		size_t GetSkeletonHash()const noexcept;
		std::wstring GetJointName(int index)const noexcept;
		std::vector<std::vector<uint8>> GetSkeletonTreeIndexVec()const noexcept;
		Core::JTypeInstanceSearchHint GetModelHint()const noexcept;
	public:
		void SetSkeletonType(JSKELETON_TYPE skeletonType)noexcept;
	public:
		bool HasAvatar()const noexcept;
		bool IsRegularChildJointIndex(uint8 childIndex, uint8 parentIndex)const noexcept;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private:	
		JSkeletonAsset(InitData& initData); 
		~JSkeletonAsset();
	};
}
 