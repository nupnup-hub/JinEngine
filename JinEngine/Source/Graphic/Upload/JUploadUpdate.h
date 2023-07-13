#pragma once
namespace JinEngine
{
	namespace Graphic
	{		
		//주로 FrameeResource가아닌 자원이 사용되는경우 할당된다
		//ex) shadowMap, occlusion 

		class JUploadDirty
		{
		private:
			bool uploadDirty = false;
		public:
			void SetUploadDirty()noexcept;
			void OffUploadDirty()noexcept;
		public:
			bool IsUploadDirted()const noexcept;
		};

		template<typename ...Param>
		class JUploadUpdateBase
		{
		public:
			virtual void UpdateUpload(Param... var)noexcept = 0;
		};

		template<typename UploadUpdateBase>
		class JUploadUpdateHolder : public UploadUpdateBase
		{
		public:
			virtual void UpdateUploadEnd()noexcept = 0;
		};

		template<typename UploadUpdateHolder, const bool useDirty>
		class JUploadUpdate : public UploadUpdateHolder, public JUploadDirty
		{
		public:
			void UpdateUploadEnd()noexcept override
			{
				OffUploadDirty();
			}
		};

		template<typename UploadUpdateHolder>
		class JUploadUpdate<UploadUpdateHolder, false> : public UploadUpdateHolder
		{
		public:
			void UpdateUploadEnd()noexcept override {}
		};
	}
}