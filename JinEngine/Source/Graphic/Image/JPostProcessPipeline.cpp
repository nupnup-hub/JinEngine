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


#include"JPostProcessPipeline.h"
#include"JToneMapping.h"
#include"JConvertColor.h"
#include"JBloom.h"
#include"JAntialise.h"
#include"JPostProcessExposure.h"
#include"JPostProcessHistogram.h"
#include"../JGraphicOption.h"
#include"../JGraphicUpdateHelper.h"
#include"../GraphicResource/JGraphicResourceManager.h"
#include"../GraphicResource/JGraphicResourceShareData.h"
#include"../../Object/Component/Camera/JCamera.h" 
#include"../../Object/GameObject/JGameObject.h"  
namespace JinEngine::Graphic
{ 
	enum POST_PROCESSING_TYPE
	{
		NONE = 0,
		TONE_MAPPING = 1 << 0,
		BLOOM = 1 << 1,
		FXAA = 1 << 2,
		EXPOSURE= 1 << 3
	};
	void JPostProcessPipeline::ApplyPostProcess(JPostProcessComputeSet* computeSet, const JDrawHelper& helper, const bool isUpdatedThisFrame)
	{ 
		if (computeSet == nullptr || !helper.allowPostProcess)
			return;
		 
		if (!isUpdatedThisFrame && helper.option.postProcess.exposureType == J_EXPOSURE_TYPE::NONE)
			return;
		  
		auto gInfo = computeSet->gm->GetInfo(helper.cam.Get(), J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
		computeSet->imageShareData = computeSet->shareData->GetResourceDependencyData(J_GRAPHIC_TASK_TYPE::CONTROLL_POST_PROCESS_PIPELINE, gInfo);
		if (computeSet->imageShareData == nullptr)
			return;
		   
		POST_PROCESSING_TYPE appliedType = POST_PROCESSING_TYPE::NONE;
		computeSet->imageShareData->UpdateBegin();
		computeSet->ppSet->convertColor->ApplyToLinearColor(computeSet, helper);

		if (helper.option.postProcess.useBloom)
		{
			computeSet->ppSet->bloom->ApplyBloom(computeSet, helper);
			appliedType = Core::AddSQValueEnum(appliedType, POST_PROCESSING_TYPE::BLOOM);
		}
		else if (helper.option.postProcess.exposureType == J_EXPOSURE_TYPE::AUTO)
			computeSet->ppSet->exposure->ExtractExposure(computeSet, helper);

		if (helper.option.postProcess.useToneMapping)
		{
			computeSet->ppSet->tm->ApplyToneMapping(computeSet, helper);
			appliedType = Core::AddSQValueEnum(appliedType, POST_PROCESSING_TYPE::TONE_MAPPING);
		}
		if(helper.option.postProcess.useFxaa)
		{
			computeSet->ppSet->aa->ApplyFxaa(computeSet, helper);
			appliedType = Core::AddSQValueEnum(appliedType, POST_PROCESSING_TYPE::FXAA);
		}

		if (appliedType != POST_PROCESSING_TYPE::NONE)
		{
			computeSet->ppSet->convertColor->ApplyToDisplayColor(computeSet, helper);
			if (helper.option.postProcess.exposureType == J_EXPOSURE_TYPE::AUTO)
			{
				computeSet->ppSet->histogram->CreateHistogram(computeSet, helper);
				computeSet->ppSet->exposure->UpdateExposure(computeSet, helper);
				appliedType = Core::AddSQValueEnum(appliedType, POST_PROCESSING_TYPE::EXPOSURE);
			}
			if (helper.option.postProcess.useHistogramDebug)
				computeSet->ppSet->histogram->DrawHistogram(computeSet, helper);
		}
		computeSet->imageShareData->UpdateEnd();
	}
}