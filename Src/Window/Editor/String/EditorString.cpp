#include"EditorString.h" 
#include"../../../Application/JApplicationVariable.h"

namespace JinEngine
{
	bool EditorString::AddString(const size_t key, const std::vector<std::string>& strVec)
	{
		if (strVec.size() != (int)Core::J_LANGUAGE_TYPE::COUNT)
			return false;

		if (strMap.find(key) != strMap.end())
			return false;
 
		strMap.emplace(key, strVec);
		return true;
	}
	const std::string EditorString::GetString(const size_t key)
	{
		auto data = strMap.find(key);
		if (data != strMap.end())
			return data->second[(int)JApplicationVariable::GetEngineLanguageType()];
		else
			return "String is not found";
	}
}