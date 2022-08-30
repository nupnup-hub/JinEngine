#pragma once
#include<vector> 
#include"JAnimationSample.h"
#include"JAnimationClipInterface.h"

namespace JinEngine
{
	class JSkeletonAsset;
	namespace Core
	{
		struct JAnimationTime;
		struct JAnimationShareData;
		struct JFbxAnimationData;
		class JFbxFileLoaderImpl;
	}

	class JAnimationClip : public JAnimationClipInterface
	{
		REGISTER_CLASS(JAnimationClip)
	private:
		friend class Core::JFbxFileLoaderImpl;
	protected:
		struct AnimationClipMetadata : public ObjectMetadata
		{
		public:
			bool hasSkeleton = false;
			bool isLooping = false;
			std::string skeletonName;
			size_t skeletonGuid;
		};
	private:
		JSkeletonAsset* clipSkeletonAsset = nullptr;
		//same Indexing
		std::vector<JAnimationSample>animationSample;
		//oriSkeleton
		size_t oriSkeletoHash;
		uint32 clipLength;
		float framePerSecond;
		bool isLooping = false;
		bool matchClipSkeleton = false;
	public:  
		JSkeletonAsset* GetClipSkeletonAsset()noexcept;
		bool GetIsLoop()const noexcept;
		uint GetSampleCount()const noexcept;
		uint GetSampleKeyCount(const uint sampleIndex)const noexcept;
		J_RESOURCE_TYPE GetResourceType()const noexcept final;
		static constexpr J_RESOURCE_TYPE GetStaticResourceType()noexcept
		{
			return J_RESOURCE_TYPE::ANIMATION_CLIP;
		}
		std::wstring GetFormat()const noexcept final;
		static std::vector<std::wstring> GetAvailableFormat()noexcept;

		void SetClipSkeletonAsset(JSkeletonAsset* clipSkeletonAsset)noexcept;

		bool IsSameSkeleton(JSkeletonAsset* srcSkeletonAsset)noexcept;
		void ClipEnter(Core::JAnimationTime& animationTime, Core::JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, const float nowTime, const float timeOffset)noexcept;
		void ClipClose()noexcept;
		void Update(Core::JAnimationTime& animationTime, Core::JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, std::vector<DirectX::XMFLOAT4X4>& localTransform, float nowTime, float deltaTime)noexcept;  
	private:
		uint GetAnimationSampleJointIndex(const uint sampleIndex, const float localTime)noexcept;
		void UpdateUsingAvatar(Core::JAnimationTime& animationTime, Core::JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, std::vector<DirectX::XMFLOAT4X4>& localTransform)noexcept;
		bool IsMatchSkeleton()const noexcept;
	public:
		bool Copy(JObject* ori) final;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private:
		void StuffResource() final;
		void ClearResource() final;
		bool IsValid()const noexcept final;
		bool ReadFbxData(); 
	private:
		void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)final;
	private:
		Core::J_FILE_IO_RESULT CallStoreResource()final;
		static Core::J_FILE_IO_RESULT StoreObject(JAnimationClip* clip);
		static Core::J_FILE_IO_RESULT StoreMetadata(std::wofstream& stream, JAnimationClip* clip);
		static JAnimationClip* LoadObject(JDirectory* directory, const JResourcePathData& pathData);
		static Core::J_FILE_IO_RESULT LoadMetadata(std::wifstream& stream, const std::wstring& folderPath, AnimationClipMetadata& metadata);
		static void RegisterJFunc();
	private:
		JAnimationClip(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
		~JAnimationClip();
	};
}