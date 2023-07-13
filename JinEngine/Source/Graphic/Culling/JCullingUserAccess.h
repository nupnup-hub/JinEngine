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
		};
	}
}