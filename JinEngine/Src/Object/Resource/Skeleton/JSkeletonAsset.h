#pragma once
#include"JSkeletonType.h"
#include"../JResourceObject.h" 

namespace JinEngine
{
	struct JSkeleton;
	class JAvatar;
	class JSkeletonAssetPrivate;
	class JSkeletonAsset final : public JResourceObject
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JSkeletonAsset)
	public: 
		class InitData final : public JResourceObject::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			std::unique_ptr<JSkeleton> skeleton;
		public:
			InitData(const uint8 formatIndex,
				JDirectory* directory,
				std::unique_ptr<JSkeleton>&& skeleton);
			InitData(const size_t guid,
				const uint8 formatIndex,
				JDirectory* directory,
				std::unique_ptr<JSkeleton>&& skeleton);
			InitData(const std::wstring& name,
				const size_t guid,
				const J_OBJECT_FLAG flag,
				const uint8 formatIndex,
				JDirectory* directory,
				std::unique_ptr<JSkeleton>&& skeleton);
		public:
			bool IsValidData()const noexcept final;
		};
	protected: 
		class LoadMetaData final : public JResourceObject::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(LoadMetaData)
		public:
			std::unique_ptr<JAvatar> avatar = nullptr;
			JSKELETON_TYPE skeletonType; 
		public:
			LoadMetaData(JDirectory* directory);
		};
	private:
		friend class JSkeletonAssetPrivate;
		class JSkeletonAssetImpl;
	private:
		std::unique_ptr<JSkeletonAssetImpl> impl;
	public:
		Core::JIdentifierPrivate& GetPrivateInterface()const noexcept final;
		J_RESOURCE_TYPE GetResourceType()const noexcept final;
		static constexpr J_RESOURCE_TYPE GetStaticResourceType()noexcept
		{
			return J_RESOURCE_TYPE::SKELETON;
		}
		std::wstring GetFormat()const noexcept final;
		static std::vector<std::wstring> GetAvailableFormat()noexcept;
	public:
		JSkeleton* GetSkeleton()noexcept;
		JAvatar* GetAvatar()noexcept;
		JSKELETON_TYPE GetSkeletonType()const noexcept; 
		size_t GetSkeletonHash()const noexcept;
		std::wstring GetJointName(int index)noexcept;
		std::vector<std::vector<uint8>> GetSkeletonTreeIndexVec()noexcept;
	public:
		void SetSkeletonType(JSKELETON_TYPE skeletonType)noexcept;
	public:
		bool HasAvatar()noexcept; 
		bool IsRegularChildJointIndex(uint8 childIndex, uint8 parentIndex)noexcept; 
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private:	
		JSkeletonAsset(InitData& initData); 
		~JSkeletonAsset();
	};
}
 