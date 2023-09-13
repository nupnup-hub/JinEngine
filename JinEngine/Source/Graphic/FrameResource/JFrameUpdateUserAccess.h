#pragma once
#include"../../Core/Pointer/JOwnerPtr.h"

namespace JinEngine
{
	namespace Core
	{
		class JIdentifier;
	}
	namespace Graphic
	{
		class JFrameDirtyBase;
		class JFrameUpdateUserAccess
		{
		protected:
			~JFrameUpdateUserAccess() = default;
		public:
			virtual bool IsFrameDirted()const noexcept = 0; 
		};
	}
}