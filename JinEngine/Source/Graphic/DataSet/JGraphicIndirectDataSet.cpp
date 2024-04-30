#include"JGraphicIndirectDataSet.h"

namespace JinEngine::Graphic
{
	JGraphicBlurTaskSettingSet::JGraphicBlurTaskSettingSet(ResourceHandle from, ResourceHandle to, std::unique_ptr<JBlurDesc>&& desc)
		:fromRH(from), toRH(to), fromDH(nullptr), toDH(nullptr), useDataHandle(false), desc(std::move(desc))
	{}
	JGraphicBlurTaskSettingSet::JGraphicBlurTaskSettingSet(const Core::JDataHandle* from, const Core::JDataHandle* to, std::unique_ptr<JBlurDesc>&& desc)
		: fromRH(nullptr), toRH(nullptr), fromDH(from), toDH(to), useDataHandle(true), desc(std::move(desc))
	{}

	JGraphicMipmapGenerationSettingSet::JGraphicMipmapGenerationSettingSet(const std::vector<Core::JDataHandle>& mipMapHandle, std::unique_ptr<JDownSampleDesc>&& desc)
		: mipMapHandle(mipMapHandle), desc(std::move(desc))
	{}

	JGraphicConvetColorSettingSet::JGraphicConvetColorSettingSet(JGraphicResourceInfo* from, JGraphicResourceInfo* to, const JConvertColorDesc& desc)
		: from(from), to(to), desc(desc)
	{}
}
