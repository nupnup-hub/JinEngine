#pragma once
#include"../JObject.h"

namespace JinEngine
{
	class JComponentInterface : public JObject
	{
	private:
		friend class JGameObject;
	protected:
		JComponentInterface(const std::string& classTypeName, const size_t guid, JOBJECT_FLAG flag);
	private:
		virtual Core::J_FILE_IO_RESULT CallStoreComponent(std::wofstream& stream) = 0;
	};
}