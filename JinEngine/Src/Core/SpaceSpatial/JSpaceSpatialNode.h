#pragma once
#include"../Pointer/JOwnerPtr.h"

namespace JinEngine
{
	class JRenderItem;
	namespace Graphic
	{
		class JCullingUserInterface;
	}
	namespace Core
	{
		class JSpaceSpatialNode
		{
		protected:
			void SetCulling(Graphic::JCullingUserInterface& cullUser, const JUserPtr<JRenderItem>& rItem)const noexcept;
			void OffCulling(Graphic::JCullingUserInterface& cullUser, const JUserPtr<JRenderItem>& rItem)const noexcept;
		};
	}
}