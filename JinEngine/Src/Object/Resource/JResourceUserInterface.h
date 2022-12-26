#pragma once
#include"../../Core/Event/JEventListener.h"
#include"JResourceEventType.h"

namespace JinEngine
{
	class JResourceObject;
	class JReferenceInterface;
 
	class JResourceUserInterface : public Core::JEventListener<size_t, J_RESOURCE_EVENT_TYPE, JResourceObject*>
	{
	protected:
		void CallOnResourceReference(JReferenceInterface* jRobj);
		void CallOffResourceReference(JReferenceInterface* jRobj);
		int CallGetResourceReferenceCount(JReferenceInterface& jRobj); 
	};
}