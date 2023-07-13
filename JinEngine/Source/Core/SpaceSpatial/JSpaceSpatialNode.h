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
		protected:

		};

		struct JIntersectInfo
		{
		public:
			int index;
			float dist;
			bool isIntersect;
		public:
			static bool CompareAsc(const JIntersectInfo& a, const JIntersectInfo& b);
			static bool CompareDesc(const JIntersectInfo& a, const JIntersectInfo& b);
		};
	}
}