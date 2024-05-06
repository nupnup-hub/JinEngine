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
#include"../JObjectFlag.h"
#include"../Component/RenderItem/JRenderLayer.h"
#include"../Component/RenderItem/JRenderItemPrimitive.h"
#include"../Component/RenderItem/JRenderItemAcceleratorMask.h"
#include"../Component/Light/JLightType.h"
#include"../Component/JComponentType.h" 

namespace JinEngine
{
	class JComponent;
	class JMeshGeometry;
	class JMaterial;
	class JCamera;
	class JLight;
	class JRenderItem;
	class JGameObject;

	class JComponentCreatorInterface
	{
	public:
		static JUserPtr<JCamera> CreateCamera(const JUserPtr<JGameObject>& owner);
		static JUserPtr<JLight> CreateLight(const JUserPtr<JGameObject>& owner, J_LIGHT_TYPE type);
		static JUserPtr<JRenderItem> CreateRenderItem(const JUserPtr<JGameObject>& owner,
			const JUserPtr<JMeshGeometry>& mesh, 
			const J_RENDER_PRIMITIVE primitiveType = J_RENDER_PRIMITIVE::TRIANGLE,
			const J_RENDER_LAYER renderLayer = J_RENDER_LAYER::OPAQUE_OBJECT,
			const J_RENDERITEM_ACCELERATOR_MASK acceleratorMask = ACCELERATOR_ALLOW_ALL);
		static JUserPtr<JRenderItem>CreateRenderItem(const JUserPtr<JGameObject>& owner,
			const JUserPtr<JMeshGeometry>& mesh,
			std::vector<JUserPtr<JMaterial>>& mat,
			const J_RENDER_PRIMITIVE primitiveType = J_RENDER_PRIMITIVE::TRIANGLE,
			const J_RENDER_LAYER renderLayer = J_RENDER_LAYER::OPAQUE_OBJECT,
			const J_RENDERITEM_ACCELERATOR_MASK acceleratorMask = ACCELERATOR_ALLOW_ALL); 
		static JUserPtr<JComponent> CreateComponent(const Core::JTypeInfo&  typeInfo, const JUserPtr<JGameObject>& owner);
	};

	using JCCI = JComponentCreatorInterface;
}