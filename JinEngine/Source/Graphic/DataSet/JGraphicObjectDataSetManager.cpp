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

#include"JGraphicObjectDataSetManager.h"
#include"../DataSet/JGraphicTaskDataSet.h"
#include"../../Object/Component/Camera/JCamera.h"
#include"../../Object/GraphicRule/JGraphicModuleInterfaceHolder.h"

namespace JinEngine::Graphic
{
	namespace
	{
		void ApplyDeferred(const JGraphicOptionChangedSet& set, const ObjectDataSetVec& camVec)
		{
			if (set.preOption.rendering.allowDeferred == set.newOption.rendering.allowDeferred)
				return;

			const bool isAct = set.newOption.rendering.allowDeferred;	 
			const uint count = camVec.Count();
			for (uint i = 0; i < count; ++i)
			{
				JOwnerPtr<JGraphicObjectDataSetBase>& objSet = *camVec.Get(i);
				JCamera* cam = static_cast<JCamera*>(objSet->Object().Get());
				auto graphicData = cam->GetModuleManagedData();

				JGraphicResourceTypeSet typeSet(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
				const int resourceIndex = graphicData->GetGraphicResourceUserInterface()->GetResourceIndex(typeSet.resouce, typeSet.task);
				if (!graphicData->GetGraphicResourceUserInterface()->IsValidHandle(typeSet.resouce, resourceIndex))
					continue;

				if (isAct)
				{
					//typeSet.option = J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP;
					//if (graphicData->GetGraphicResourceUserInterface()->HasOption(typeSet.resouce, typeSet.option, resourceIndex))
					//	continue;

					JGraphicResourceCreationDesc desc(typeSet);
					typeSet.option = J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP;
					GMI()->CreateGraphicResourceOption(graphicData, desc);

					typeSet.option = J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY;
					GMI()->CreateGraphicResourceOption(graphicData, desc);

					typeSet.option = J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP;
					GMI()->CreateGraphicResourceOption(graphicData, desc);
				}
				else
				{
					typeSet.option = J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP;
					GMI()->DestroyGraphicResourceOption(graphicData, typeSet);

					typeSet.option = J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY;
					GMI()->DestroyGraphicResourceOption(graphicData, typeSet);

					typeSet.option = J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP;
					GMI()->DestroyGraphicResourceOption(graphicData, typeSet);
				}
				graphicData->GetFrameUpdateUserInterface()->SetFrameDirty();
			}
		}
	}
	bool JGraphicObjectDataSetManager::HasDependency(const JGraphicOption::TYPE type)const noexcept
	{
		if (type == JGraphicOption::TYPE::RENDERING)
			return true;
		else
			return false;
	}
	void JGraphicObjectDataSetManager::NotifyGraphicOptionChanged(const JGraphicOptionChangedSet& set)
	{
		if (set.preOption.rendering.allowDeferred != set.newOption.rendering.allowDeferred)
			ApplyDeferred(set, GetDataVec(J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA));
	}
}