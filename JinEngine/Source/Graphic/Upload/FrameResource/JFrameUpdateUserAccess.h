#pragma once
#include"../../../Core/Pointer/JOwnerPtr.h"

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
		private:
			JUserPtr<Core::JIdentifier> iden;
			JFrameDirtyBase* dirtyBase;
		public:
			JFrameUpdateUserAccess(JUserPtr<Core::JIdentifier> iden, JFrameDirtyBase* dirtyBase);
		public:
			bool IsFrameDirted()const noexcept;
			bool IsValid()const noexcept;
		};

		class JFrameUpdateUserAccessInterface
		{
		protected:
			~JFrameUpdateUserAccessInterface() = default;
		public:
			virtual JFrameUpdateUserAccess FrameUserInterface() noexcept = 0;
		};
	}
}