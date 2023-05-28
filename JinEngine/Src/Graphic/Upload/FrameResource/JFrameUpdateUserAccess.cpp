#include"JFrameUpdateUserAccess.h"
#include"JFrameUpdate.h"
#include"../../../Core/Identity/JIdentifier.h"

namespace JinEngine
{
	namespace Graphic
	{
		JFrameUpdateUserAccess::JFrameUpdateUserAccess(JUserPtr<Core::JIdentifier> iden, JFrameDirtyBase* dirtyBase)
			:iden(iden), dirtyBase(dirtyBase)
		{}
		bool JFrameUpdateUserAccess::IsFrameDirted()const noexcept
		{
			return IsValid() ? dirtyBase->IsFrameDirted() : false;
		}
		bool JFrameUpdateUserAccess::IsValid()const noexcept
		{
			return iden.IsValid();
		}
	}
}