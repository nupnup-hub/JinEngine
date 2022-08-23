#pragma once
#include"../JComponent.h" 
#include"../../JFrameInterface.h"
#include"../../../Graphic/JGraphicTexture.h"
namespace JinEngine
{
	namespace Graphic
	{
		struct JLightConstants;
		struct JShadowMapConstants;
	} 

	class JLightInterface : public JComponent, public JFrameInterface<Graphic::JLightConstants, Graphic::JShadowMapConstants>, public Graphic::JGraphicTexture
	{
	protected:
		JLightInterface(const std::string& cTypeName, size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner);
	};
}