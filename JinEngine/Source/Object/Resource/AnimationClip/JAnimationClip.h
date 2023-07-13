#pragma once
#include<vector>  
#include"JAnimationData.h"
#include"../JResourceObject.h"
#include"../Skeleton/JSkeletonAsset.h"

namespace JinEngine
{
	class JSkeletonAsset;
	namespace Core
	{ 
		class JAnimationUpdateData;
	}
	struct JAnimationData;
	class JAnimationClipPrivate;
	class JAnimationClip final : public JResourceObject
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JAnimationClip)
		REGISTER_CLASS_IDENTIFIER_DEFAULT_LAZY_DESTRUCTION
	public: 
		class InitData final: public JResourceObject::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			std::unique_ptr<JAnimationData> anidata;
		public:
			InitData(const uint8 formatIndex, 
				const JUserPtr<JDirectory>& directory,
				std::unique_ptr<JAnimationData>&& anidata);
			InitData(const size_t guid,
				const uint8 formatIndex,
				const JUserPtr<JDirectory>& directory,
				std::unique_ptr<JAnimationData>&& anidata);
			InitData(const std::wstring& name,
				const size_t guid,
				const J_OBJECT_FLAG flag,
				const uint8 formatIndex, 
				const JUserPtr<JDirectory>& directory,
				std::unique_ptr<JAnimationData>&& anidata);
		public:
			bool IsValidData()const noexcept final; 
		};
	protected: 
		class LoadMetaData final : public JResourceObject::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(LoadMetaData)
		public:
			JUserPtr<JSkeletonAsset> clipSkeletonAsset = nullptr;
			float updateFramePerSecond = 0;
			bool isLooping = false;
		public:
			LoadMetaData(const JUserPtr<JDirectory>& directory);
		};
	private:
		friend class JAnimationClipPrivate;
		class JAnimationClipImpl;
	private:
		std::unique_ptr<JAnimationClipImpl> impl;
	public:  
		Core::JIdentifierPrivate& PrivateInterface()const noexcept final;
		JUserPtr<JSkeletonAsset> GetClipSkeletonAsset()const noexcept;
		float GetUpdateFPS()const noexcept;
		uint GetSampleCount()const noexcept;
		uint GetSampleKeyCount(const uint sampleIndex)const noexcept;
		J_RESOURCE_TYPE GetResourceType()const noexcept final;
		static constexpr J_RESOURCE_TYPE GetStaticResourceType()noexcept
		{
			return J_RESOURCE_TYPE::ANIMATION_CLIP;
		}
		std::wstring GetFormat()const noexcept final;
		static std::vector<std::wstring> GetAvailableFormat()noexcept;
	public:
		void SetClipSkeletonAsset(JUserPtr<JSkeletonAsset> newClipSkeletonAsset)noexcept;
		void SetUpdateFPS(float value)noexcept;
		void SetLoop(bool value)noexcept;
	public:
		bool IsLoop()const noexcept;
		bool IsSameSkeleton(JSkeletonAsset* srcSkeletonAsset)noexcept;
		bool IsValid()const noexcept final;
	public:
		void ClipEnter(Core::JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber, const float timeOffset)noexcept;
		void ClipClose()noexcept; 
		void Update(Core::JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private:
		JAnimationClip(InitData& initData);
		~JAnimationClip();
	};
}