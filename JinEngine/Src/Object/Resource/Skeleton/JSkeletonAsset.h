#pragma once
#include"JSkeletonType.h"
#include"Joint.h"
#include"Avatar/JAvatar.h"  
#include"../JResourceObject.h" 
 
namespace JinEngine
{
	struct JSkeleton;
	class JAvatar;
	class JSkeletonAssetPrivate;
	class JSkeletonAsset final : public JResourceObject
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JSkeletonAsset)
		REGISTER_CLASS_IDENTIFIER_DEFAULT_LAZY_DESTRUCTION
	public: 
		class InitData final : public JResourceObject::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			std::vector<Joint> joint;
		public:
			InitData(const uint8 formatIndex,
				const JUserPtr<JDirectory>& directory,
				std::vector<Joint>&& joint);
			InitData(const size_t guid,
				const uint8 formatIndex,
				const JUserPtr<JDirectory>& directory,
				std::vector<Joint>&& joint);
			InitData(const std::wstring& name,
				const size_t guid,
				const J_OBJECT_FLAG flag,
				const uint8 formatIndex,
				const JUserPtr<JDirectory>& directory,
				std::vector<Joint>&& joint);
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
 