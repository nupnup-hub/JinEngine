#pragma once
#include"../JResourceObject.h"
#include"../JClearableInterface.h" 
#include"../../../Core/FSM/JFSMownerInterface.h"

namespace JinEngine
{
	class JAnimationControllerInterface : public JResourceObject, 
		public JClearableInterface,
		public Core::IJFSMdiagramOwner,
		public Core::IJFSMconditionStorageOwner
	{
	protected:
		JAnimationControllerInterface(const JResourceObject::JResourceInitData& initdata);
	};
}