#pragma once
#include<vector>  
#include"JAnimationData.h"
#include"JAnimationClipInterface.h"
#include"../Skeleton/JSkeletonAsset.h"

namespace JinEngine
{
	class JSkeletonAsset;
	namespace Core
	{
		struct JAnimationTime;
		class JAnimationUpdateData;
		struct JFbxAnimationData;
		class JFbxFileLoaderImpl;
	}

	class JAnimationClip final : public JAnimationClipInterface
	{
		REGISTER_CLASS(JAnimationClip)
	public:
		struct JAnimationClipInitData : JResourceInitData
		{
		public:
			Core::JOwnerPtr<JAnimationData> anidata;
		public:
			JAnimationClipInitData(const std::wstring& name,
				const size_t guid,
				const J_OBJECT_FLAG flag,
				JDirectory* directory,
				const std::wstring oridataPath,
				Core::JOwnerPtr<JAnimationData> anidata);
			JAnimationClipInitData(const std::wstring& name,
				const size_t guid,
				const J_OBJECT_FLAG flag,
				JDirectory* directory,
				const uint8 formatIndex);
			JAnimationClipInitData(const std::wstring& name,
				JDirectory* directory,
				const std::wstring oridataPath,
				Core::JOwnerPtr<JAnimationData> anidata);
		public:
			bool IsValidCreateData()final; 
		};
		using InitData = JAnimationClipInitData;
	private:
		struct JAnimationClipMetadata final : public JResourceMetaData
		{
		public:
			JSkeletonAsset* clipSkeletonAsset = nullptr; 
			float updateFramePerSecond;
			bool isLooping = false; 
		};
	private:
		friend class Core::JFbxFileLoaderImpl;
	private:
		//fixed
		std::vector<JAnimationSample>animationSample;
		size_t oriSkeletoHash;
		//
		REGISTER_PROPERTY_EX(clipSkeletonAsset, GetClipSkeletonAsset, SetClipSkeletonAsset, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::NONE, false))
		Core::JUserPtr<JSkeletonAsset> clipSkeletonAsset;
		uint32 clipLength;
		float clipFramePerSecond;
		REGISTER_PROPERTY_EX(updateFramePerSecond, GetUpdateFPS, SetUpdateFPS, GUI_SLIDER(30, 120))
		float updateFramePerSecond; 
		REGISTER_PROPERTY_EX(isLooping, IsLoop, SetLoop, GUI_CHECKBOX())
		bool isLooping = false;
		bool matchClipSkeleton = false;
	public:  
		Core::JUserPtr<JSkeletonAsset> GetClipSkeletonAsset()noexcept;
		float GetUpdateFPS()const noexcept;
		bool IsLoop()const noexcept;
		uint GetSampleCount()const noexcept;
		uint GetSampleKeyCount(const uint sampleIndex)const noexcept;
		J_RESOURCE_TYPE GetResourceType()const noexcept final;
		static constexpr J_RESOURCE_TYPE GetStaticResourceType()noexcept
		{
			return J_RESOURCE_TYPE::ANIMATION_CLIP;
		}
		std::wstring GetFormat()const noexcept final;
		static std::vector<std::wstring> GetAvailableFormat()noexcept;

		void SetClipSkeletonAsset(Core::JUserPtr<JSkeletonAsset> newClipSkeletonAsset)noexcept;
		void SetUpdateFPS(float value)noexcept;
		void SetLoop(bool value)noexcept;
	public:
		bool IsSameSkeleton(JSkeletonAsset* srcSkeletonAsset)noexcept;
		void ClipEnter(Core::JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber, const float timeOffset)noexcept;
		void ClipClose()noexcept; 
		void Update(Core::JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept;
	private:
		uint GetAnimationSampleJointIndex(const uint sampleIndex, const float localTime)noexcept;
		void UpdateUsingAvatar(Core::JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept;
		bool IsMatchSkeleton()const noexcept;
	private:
		void DoCopy(JObject* ori) final;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private:
		void StuffResource() final;
		void ClearResource() final;
		bool IsValid()const noexcept final;
		bool WriteClipData();
		bool ReadClipData();  
	private:
		void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)final;
	private:
		bool ImportAnimationClip(const JAnimationData& jfbxAniData);
	private:
		Core::J_FILE_IO_RESULT CallStoreResource()final;
		static Core::J_FILE_IO_RESULT StoreObject(JAnimationClip* clip); 
		static Core::J_FILE_IO_RESULT StoreMetadata(std::wofstream& stream, JAnimationClip* clip);
		static JAnimationClip* LoadObject(JDirectory* directory, const Core::JAssetFileLoadPathData& pathData); 
		static Core::J_FILE_IO_RESULT LoadMetadata(std::wifstream& stream, JAnimationClipMetadata& metadata);
		static void RegisterJFunc();
	private:
		JAnimationClip(const JAnimationClipInitData& initdata);
		~JAnimationClip();
	};
}