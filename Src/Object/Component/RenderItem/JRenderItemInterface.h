#pragma once
#include"../JComponent.h" 
#include"../../JFrameInterface.h"
#include"../../Resource/JResourceUserInterface.h"
#include"../../../Graphic/JGraphicBufInterface.h"
namespace JinEngine
{
	namespace Graphic
	{
		struct JObjectConstants;
	}

	class JRenderItemInterface : public JComponent,
		public JFrameInterface<Graphic::JObjectConstants> ,
		public JResourceUserInterface,
		public Graphic::JGraphicBufUserInterface
	{
	protected:
		JRenderItemInterface(const std::string& cTypeName, size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner);
	};
}