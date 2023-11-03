#pragma once
#include"JResourceObject.h"
#include"JResourceObjectEventType.h"
#include"../../Core/Event/JEventListener.h"

namespace JinEngine
{
	class JResourceObject;
	class JReferenceInterface;
 
	using JResourceListener = typename JResourceEventManager::Listener;
	class JResourceObjectUserInterface : public JResourceListener
	{
	protected:
		//Call scene object is activated
		void CallOnResourceReference(JReferenceInterface* jRobj);
		//Call scene object is deactivated
		void CallOffResourceReference(JReferenceInterface* jRobj);
		int CallGetResourceReferenceCount(JReferenceInterface* jRobj);
	};
}