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


#include"JPreviewDirectory.h" 
#include"../JScene.h"   
#include"../../JResourceManager.h"
#include"../../Texture/JTexture.h"
#include"../../Material/JMaterial.h"
#include"../../Material/JDefaultMaterialSetting.h"
#include"../../Mesh/JMeshGeometry.h" 
#include"../../../Directory/JDirectory.h"
#include"../../../Component/JComponentCreator.h"
#include"../../../Component/RenderItem/JRenderItem.h" 
#include"../../../GameObject/JGameObject.h" 
#include"../../../GameObject/JGameObjectCreator.h" 
#include"../../../../Core/Guid/JGuidCreator.h"
#include"../../../../Core/Identity/JIdenCreator.h"

#include "../../../../Application/Project/JApplicationProject.h"
namespace JinEngine
{
	JPreviewDirectory::JPreviewDirectory(_In_ JUserPtr<JDirectory> jDir, const J_PREVIEW_DIMENSION previewDimension, const J_PREVIEW_FLAG previewFlag)
		:JPreviewScene(jDir, previewDimension, previewFlag)
	{ 
		JUserPtr<JScene> newScene = JICI::Create<JScene>(jDir->GetName() + L"_PreviewScene",
			Core::MakeGuid(),
			OBJECT_FLAG_EDITOR_OBJECT,
			JScene::GetDefaultFormatIndex(),
			_JResourceManager::Instance().GetEditorResourceDirectory(),
			J_SCENE_USE_CASE_TYPE::TWO_DIMENSIONAL_PREVIEW);
		 
		SetScene(newScene); 
	}
	JPreviewDirectory::~JPreviewDirectory(){}
	bool JPreviewDirectory::Initialze()noexcept
	{
		if (MakeJDirectoryPreviewScene())
			return true;
		else
		{
			Clear();
			return false;
		}
	}
	bool JPreviewDirectory::MakeJDirectoryPreviewScene()noexcept
	{ 
		JUserPtr<JTexture> texture = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::DIRECTORY);
		if (texture == nullptr)
			return false;

		const std::wstring matName = GetJObject()->GetName() + L"PreviewMaterial";
		JUserPtr<JDirectory> dir = _JResourceManager::Instance().GetEditorResourceDirectory();

		J_OBJECT_FLAG flag = OBJECT_FLAG_EDITOR_OBJECT;
		JUserPtr<JMaterial> newTextureMat = JICI::Create<JMaterial>(matName, Core::MakeGuid(), flag, JMaterial::GetDefaultFormatIndex(), dir);
		JDefaultMaterialSetting::SetAlbedoMapOnly(newTextureMat, texture);
		 
		JUserPtr<JGameObject> shapeObj = JGCI::CreateShape(GetScene()->GetRootGameObject(), flag, J_DEFAULT_SHAPE::QUAD);
		JUserPtr<JRenderItem> renderItem = shapeObj->GetRenderItem();

		renderItem->SetMaterial(0, newTextureMat);

		AdjustSceneSettingData adjustData;
		adjustData.targetTransform = shapeObj->GetTransform();
		adjustData.targetRenderItem = renderItem;
		adjustData.useFixedPad = true;

		SetTextureMaterial(newTextureMat);
		SetUseQuadShapeTrigger(true);
		AdjustScene(adjustData);
		return true;
	}
}
