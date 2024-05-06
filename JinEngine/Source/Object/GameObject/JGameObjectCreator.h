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
#include"../Component/RenderItem/JRenderLayer.h"
#include"../Component/RenderItem/JRenderItemAcceleratorMask.h"
#include"../Component/Light/JLightType.h"
#include"../Resource/Mesh/JDefaultShapeType.h"
#include"../Resource/Material/JDefaultMaterialType.h" 
#include"../JObjectFlag.h" 

namespace JinEngine
{
	class JGameObject;
	class JMeshGeometry;
	class JScene;
	class JGameObjectCreatorInterface
	{
	public:
		static std::wstring GetDefaultLitName(const J_LIGHT_TYPE type)noexcept;
	public:
		static JUserPtr<JGameObject> CreateRoot(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JScene>& ownerScene);
		static JUserPtr<JGameObject> CreateShape(JUserPtr<JGameObject> parent, const J_OBJECT_FLAG flag, const J_DEFAULT_SHAPE shape);
		static JUserPtr<JGameObject> CreateShape(JUserPtr<JGameObject> parent, const size_t guid, const J_OBJECT_FLAG flag, const J_DEFAULT_SHAPE shape);
		static JUserPtr<JGameObject> CreateShape(JUserPtr<JGameObject> parent, const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, const J_DEFAULT_SHAPE shape);
		static JUserPtr<JGameObject> CreateModel(JUserPtr<JGameObject> parent, const J_OBJECT_FLAG flag, const JUserPtr<JMeshGeometry>& mesh);
		static JUserPtr<JGameObject> CreateModel(JUserPtr<JGameObject> parent, const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JMeshGeometry>& mesh);
		static JUserPtr<JGameObject> CreateSky(JUserPtr<JGameObject> parent, const J_OBJECT_FLAG flag, const std::wstring name = L"Skymap"); 
		//if allowCulling(true) set hdOcc
		static JUserPtr<JGameObject> CreateCamera(JUserPtr<JGameObject> parent, const J_OBJECT_FLAG flag, const bool allowCulling);
		static JUserPtr<JGameObject> CreateCamera(JUserPtr<JGameObject> parent, const size_t guid, const J_OBJECT_FLAG flag, const bool allowCulling);
		static JUserPtr<JGameObject> CreateCamera(JUserPtr<JGameObject> parent, const std::wstring name, const size_t guid, const J_OBJECT_FLAG flag, const bool allowCulling);
		static JUserPtr<JGameObject> CreateLight(JUserPtr<JGameObject> parent, const J_OBJECT_FLAG flag, const J_LIGHT_TYPE type);
		static JUserPtr<JGameObject> CreateLight(JUserPtr<JGameObject> parent, const size_t guid, const J_OBJECT_FLAG flag, const J_LIGHT_TYPE type);
		static JUserPtr<JGameObject> CreateLight(JUserPtr<JGameObject> parent, const std::wstring name, const size_t guid, const J_OBJECT_FLAG flag, const J_LIGHT_TYPE type);
	public:
		static JUserPtr<JGameObject> CreateDebugRoot(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JScene>& ownerScene);
		static JUserPtr<JGameObject> CreateDebugCamera(JUserPtr<JGameObject> parent, const J_OBJECT_FLAG flag, const std::wstring name = L"DebugCamera");
		static JUserPtr<JGameObject> CreateDebugShape(JUserPtr<JGameObject> parent,
			const std::wstring& name,
			const J_OBJECT_FLAG flag,
			const J_DEFAULT_SHAPE meshType,
			const J_DEFAULT_MATERIAL matType, 
			const bool isDebugUI = false,
			const bool isLine = false,
			const J_RENDERITEM_ACCELERATOR_MASK acceleratorMask = ACCELERATOR_NOT_ALLOW_ALL);
		static JUserPtr<JGameObject> CreateDebugLineShape(JUserPtr<JGameObject> parent,
			const J_OBJECT_FLAG flag,
			const J_DEFAULT_SHAPE meshType,
			const J_DEFAULT_MATERIAL matType,
			const bool isDebugUI);
	};

	using JGCI = JGameObjectCreatorInterface;
}