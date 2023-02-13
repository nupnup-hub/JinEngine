#pragma once
#include"../JResourceObject.h"
#include"../JClearableInterface.h"  
#include"../../../Core/FSM/JFSMownerInterface.h"

namespace JinEngine
{
	namespace Core
	{
		class JFSMdiagram;
	}
	class JAnimationControllerInterface : public JResourceObject, 
		public JClearableInterface,
		public Core::JFSMdiagramOwnerInterface
	{
	protected:
		JAnimationControllerInterface(const JResourceObject::JResourceInitData& initdata);
	};
}