#pragma once
#include"../JObject.h"

namespace JinEngine
{
	class JGameObjectInterface : public JObject
	{ 
		friend class JScene;
	protected:
		JGameObjectInterface(const std::string& name, const size_t guid, const JOBJECT_FLAG flag);
	private:
		virtual Core::J_FILE_IO_RESULT CallStoreGameObject(std::wofstream& stream) = 0;
	}; 
}