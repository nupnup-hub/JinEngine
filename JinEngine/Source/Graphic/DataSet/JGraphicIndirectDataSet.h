#pragma once
#include"JGraphicDataSet.h"

namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicResourceInfo;
		struct JGraphicBlurTaskSettingSet
		{
		public:
			ResourceHandle fromRH;
			ResourceHandle toRH;
		public:
			const Core::JDataHandle* fromDH;
			const Core::JDataHandle* toDH;
		public:
			std::unique_ptr<JBlurDesc> desc;
			std::unique_ptr<JGraphicBlurComputeSet> dataSet;	//out
		public:
			const bool useDataHandle = false;
		public:
			JGraphicBlurTaskSettingSet(ResourceHandle from, ResourceHandle to, std::unique_ptr<JBlurDesc>&& desc);
			JGraphicBlurTaskSettingSet(const Core::JDataHandle* from, const Core::JDataHandle* to, std::unique_ptr<JBlurDesc>&& desc);
		};

		struct JGraphicMipmapGenerationSettingSet
		{
		public:
			const std::vector<Core::JDataHandle>& mipMapHandle;
			std::unique_ptr<JDownSampleDesc> desc;
			std::unique_ptr<JGraphicDownSampleComputeSet> dataSet;		//out
		public:
			JGraphicMipmapGenerationSettingSet(const std::vector<Core::JDataHandle>& mipMapHandle, std::unique_ptr<JDownSampleDesc>&& desc);
		};

		struct JGraphicConvetColorSettingSet
		{
		public:
			std::unique_ptr<JGraphicConvertColorComputeSet> dataSet;		//out
		public: 
			JGraphicResourceInfo* from;											//in
			JGraphicResourceInfo* to;										//in
		public:
			JConvertColorDesc desc;											//in
		public:
			JGraphicConvetColorSettingSet(JGraphicResourceInfo* from, JGraphicResourceInfo* to, const JConvertColorDesc& desc);
		};

	}
}