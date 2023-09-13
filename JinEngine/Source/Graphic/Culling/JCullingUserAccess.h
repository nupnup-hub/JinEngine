#pragma once
#include"JCullingInterface.h"

namespace JinEngine
{
	namespace Graphic
	{
		class JCullingUserAccess
		{
		protected:
			JCullingUserAccess() = default;
			virtual ~JCullingUserAccess() = default;
		public:
			virtual const JCullingUserInterface CullingUserInterface()const noexcept = 0;
		public:  
			virtual bool AllowFrustumCulling()const noexcept = 0;
			virtual bool AllowHzbOcclusionCulling()const noexcept = 0;
			virtual bool AllowHdOcclusionCulling()const noexcept = 0;
			virtual bool AllowDisplayOccCullingDepthMap()const noexcept = 0;
		public:
			bool AllowOcclusionCulling()const noexcept;
		};
	}
}