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


#include"JGraphicInfo.h"
#include"../Core/File/JFileIOHelper.h"
#include"../Core/File/JFileConstant.h"
#include"../Application/Project/JApplicationProject.h"

namespace JinEngine::Graphic
{ 
	uint JGraphicInfo::FrameResourceInfo::GetLocalLightCapacity()const noexcept
	{
		return upPLightCapacity + upSLightCapacity + upRLightCapacity;
	} 
	void JGraphicInfo::Load()
	{
		JFileIOTool tool;
		const std::wstring path = Core::JFileConstant::MakeFilePath(JApplicationProject::ConfigPath(), L"GraphicInfo.txt");
		if (!tool.Begin(path, JFileIOTool::TYPE::JSON, JFileIOTool::BEGIN_OPTION_JSON_TRY_LOAD_DATA))
			return;

		tool.PushExistStack("--FrameInfo--");
		/*
		JFileIOHelper::LoadAtomicData(tool, frame.upObjCount, "UploadObjCount:");
		JFileIOHelper::LoadAtomicData(tool, frame.upBoundingObjCount, "UploadBoundingObjCount:");
		JFileIOHelper::LoadAtomicData(tool, frame.upHzbObjCount, "UploadHzbObjCount:");
		JFileIOHelper::LoadAtomicData(tool, frame.upAniCount, "UploadAniCount:");
		JFileIOHelper::LoadAtomicData(tool, frame.upScenePassCount, "UploadScenePassCount:");
		JFileIOHelper::LoadAtomicData(tool, frame.upCameraCount, "UploadCameraCount:");
		JFileIOHelper::LoadAtomicData(tool, frame.upDLightCount, "UploadDirectionalLightCount:");
		JFileIOHelper::LoadAtomicData(tool, frame.upPLightCount, "UploadPointLightCount:");
		JFileIOHelper::LoadAtomicData(tool, frame.upSLightCount, "UploadSpotLightCount:");
		JFileIOHelper::LoadAtomicData(tool, frame.upRLightCount, "UploadRectLightCount:");
		JFileIOHelper::LoadAtomicData(tool, frame.upCsmCount, "UploadCsmCount:");
		JFileIOHelper::LoadAtomicData(tool, frame.upCubeShadowMapCount, "UploadCubeShadowMapCount:");
		JFileIOHelper::LoadAtomicData(tool, frame.upNormalShadowMapCount, "UploadNormalShadowMapCount:");
		JFileIOHelper::LoadAtomicData(tool, frame.upMaterialCount, "UploadMaterialCount:");
		*/
  
		JFileIOHelper::LoadAtomicData(tool, frame.upObjCapacity, "UploadObjCapacity:");
		JFileIOHelper::LoadAtomicData(tool, frame.upBoundingObjCapacity, "UploadBoundingObjCapacity:");
		JFileIOHelper::LoadAtomicData(tool, frame.upHzbObjCapacity, "UploadHzbObjCapacity:");
		JFileIOHelper::LoadAtomicData(tool, frame.upAniCapacity, "UploadAniCapacity:");
		JFileIOHelper::LoadAtomicData(tool, frame.upScenePassCapacity, "UploadScenePassCapacity:");
		JFileIOHelper::LoadAtomicData(tool, frame.upCameraCapacity, "UploadCameraCapacity:");
		JFileIOHelper::LoadAtomicData(tool, frame.upDLightCapacity, "UploadDirectionalLightCapacity:");
		JFileIOHelper::LoadAtomicData(tool, frame.upPLightCapacity, "UploadPointLightCapacity:");
		JFileIOHelper::LoadAtomicData(tool, frame.upSLightCapacity, "UploadSpotLightCapacity:");
		JFileIOHelper::LoadAtomicData(tool, frame.upRLightCapacity, "UploadRectLightCapacity:"); 
		JFileIOHelper::LoadAtomicData(tool, frame.upMaterialCapacity, "UploadMaterialCapacity:");
		
		tool.PopStack();

		tool.PushExistStack("--ResourceInfo--");
		JFileIOHelper::LoadAtomicData(tool, resource.binding2DTextureCapacity, "Bind2DTextureCapacity:");
		JFileIOHelper::LoadAtomicData(tool, resource.bindingCubeMapCapacity, "BindCubeMapCapacity:");
		JFileIOHelper::LoadAtomicData(tool, resource.bindingShadowTextureCapacity, "BindShadowTextureCapacity:");
		JFileIOHelper::LoadAtomicData(tool, resource.bindingShadowTextureArrayCapacity, "BindShadowTextureArrayCapacity:");
		JFileIOHelper::LoadAtomicData(tool, resource.bindingShadowTextureCubeCapacity, "BindShadowTextureCubeCapacity:");
		tool.PopStack();
		tool.Close();  
	}
	void JGraphicInfo::Store()
	{
		JFileIOTool tool;
		const std::wstring path = Core::JFileConstant::MakeFilePath(JApplicationProject::ConfigPath(), L"GraphicInfo.txt");
		if (!tool.Begin(path, JFileIOTool::TYPE::JSON))
			return;

		tool.PushMapMember("--FrameInfo--");
		/*
		JFileIOHelper::StoreAtomicData(tool, frame.upObjCount, "UploadObjCount:");
		JFileIOHelper::StoreAtomicData(tool, frame.upBoundingObjCount, "UploadBoundingObjCount:");
		JFileIOHelper::StoreAtomicData(tool, frame.upHzbObjCount, "UploadHzbObjCount:");
		JFileIOHelper::StoreAtomicData(tool, frame.upAniCount, "UploadAniCount:");
		JFileIOHelper::StoreAtomicData(tool, frame.upScenePassCount, "UploadScenePassCount:");
		JFileIOHelper::StoreAtomicData(tool, frame.upCameraCount, "UploadCameraCount:");
		JFileIOHelper::StoreAtomicData(tool, frame.upDLightCount, "UploadDirectionalLightCount:");
		JFileIOHelper::StoreAtomicData(tool, frame.upPLightCount, "UploadPointLightCount:");
		JFileIOHelper::StoreAtomicData(tool, frame.upSLightCount, "UploadSpotLightCount:");
		JFileIOHelper::StoreAtomicData(tool, frame.upRLightCount, "UploadRectLightCount:");
		JFileIOHelper::StoreAtomicData(tool, frame.upCsmCount, "UploadCsmCount:");
		JFileIOHelper::StoreAtomicData(tool, frame.upCubeShadowMapCount, "UploadCubeShadowMapCount:");
		JFileIOHelper::StoreAtomicData(tool, frame.upNormalShadowMapCount, "UploadNormalShadowMapCount:");
		JFileIOHelper::StoreAtomicData(tool, frame.upMaterialCount, "UploadMaterialCount:");
		*/

		JFileIOHelper::StoreAtomicData(tool, frame.upObjCapacity, "UploadObjCapacity:");
		JFileIOHelper::StoreAtomicData(tool, frame.upBoundingObjCapacity, "UploadBoundingObjCapacity:");
		JFileIOHelper::StoreAtomicData(tool, frame.upHzbObjCapacity, "UploadHzbObjCapacity:");
		JFileIOHelper::StoreAtomicData(tool, frame.upAniCapacity, "UploadAniCapacity:");
		JFileIOHelper::StoreAtomicData(tool, frame.upScenePassCapacity, "UploadScenePassCapacity:");
		JFileIOHelper::StoreAtomicData(tool, frame.upCameraCapacity, "UploadCameraCapacity:");
		JFileIOHelper::StoreAtomicData(tool, frame.upDLightCapacity, "UploadDirectionalLightCapacity:");
		JFileIOHelper::StoreAtomicData(tool, frame.upPLightCapacity, "UploadPointLightCapacity:");
		JFileIOHelper::StoreAtomicData(tool, frame.upSLightCapacity, "UploadSpotLightCapacity:");
		JFileIOHelper::StoreAtomicData(tool, frame.upRLightCapacity, "UploadRectLightCapacity:");
		JFileIOHelper::StoreAtomicData(tool, frame.upMaterialCapacity, "UploadMaterialCapacity:");
		tool.PopStack();

		tool.PushMapMember("--ResourceInfo--");
		JFileIOHelper::StoreAtomicData(tool, resource.binding2DTextureCapacity, "Bind2DTextureCapacity:");
		JFileIOHelper::StoreAtomicData(tool, resource.bindingCubeMapCapacity, "BindCubeMapCapacity:");
		JFileIOHelper::StoreAtomicData(tool, resource.bindingShadowTextureCapacity, "BindShadowTextureCapacity:");
		JFileIOHelper::StoreAtomicData(tool, resource.bindingShadowTextureArrayCapacity, "BindShadowTextureArrayCapacity:");
		JFileIOHelper::StoreAtomicData(tool, resource.bindingShadowTextureCubeCapacity, "BindShadowTextureCubeCapacity:");
		tool.PopStack();
		tool.Close(JFileIOTool::CLOSE_OPTION_JSON_STORE_DATA);
	}
}