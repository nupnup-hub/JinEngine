#pragma once
#include"../JComponent.h" 
#include"../../JFrameUpdate.h"
#include"../../../Graphic/JGraphicTexture.h"
namespace JinEngine
{
	namespace Graphic
	{
		struct JLightConstants;
		struct JShadowMapLightConstants;
		struct JShadowMapConstants;
	} 
	 
	class JLightInterface : public JComponent,  
		public JFrameUpdate<IFrameUpdate<Graphic::JLightConstants&, 
		Graphic::JShadowMapLightConstants&,
		Graphic::JShadowMapConstants&>, JFrameDirtyListener, false>,
		public Graphic::JGraphicTexture
	{
	protected:
		JLightInterface(const std::string& cTypeName, size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner);
	};
}