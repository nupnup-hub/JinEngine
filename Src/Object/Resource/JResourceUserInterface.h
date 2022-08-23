#pragma once
#include"../../Core/Event/JEventListener.h"
#include"JResourceEventType.h"

namespace JinEngine
{
	class JResourceObject;
	class JReferenceInterface;
	class JValidInterface; 

	class JResourceUserInterface : public Core::JEventListener<size_t, J_RESOURCE_EVENT_TYPE, JResourceObject*>
	{
	protected:
		void OnResourceReference(JReferenceInterface& jRobj);
		void OffResourceReference(JReferenceInterface& jRobj);
		int GetResourceReferenceCount(JReferenceInterface& jRobj);
		bool IsVaildResource(JValidInterface& jRobj);
	};
}