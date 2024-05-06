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
#include"../Skeleton/JSkeletonAsset.h"

namespace JinEngine
{
	class JSkeletonAsset;
	class JAnimationUpdateData;
	class JAnimationClipPrivate;
	namespace Core
	{
		struct JAnimationData;
	}
	class JAnimationClip final : public JResourceObject
	{
		REGISTER_CLASS_IDENTIFIER_LINE_RESOURCE(JAnimationClip) 
	public: 
		class InitData final: public JResourceObject::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			std::unique_ptr<Core::JAnimationData> anidata;
		public:
			InitData(const uint8 formatIndex, 
				const JUserPtr<JDirectory>& directory,
				std::unique_ptr<Core::JAnimationData>&& anidata);
			InitData(const size_t guid,
				const uint8 formatIndex,
				const JUserPtr<JDirectory>& directory,
				std::unique_ptr<Core::JAnimationData>&& anidata);
			InitData(const std::wstring& name,
				const size_t guid,
				const J_OBJECT_FLAG flag,
				const uint8 formatIndex, 
				const JUserPtr<JDirectory>& directory,
				std::unique_ptr<Core::JAnimationData>&& anidata);
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
		void ClipEnter(JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber, const float timeOffset)noexcept;
		void ClipClose()noexcept; 
		void Update(JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private:
		JAnimationClip(InitData& initData);
		~JAnimationClip();
	};
}