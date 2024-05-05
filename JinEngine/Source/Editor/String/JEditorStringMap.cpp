#include"JEditorStringMap.h" 
#include"../../Application/Engine/JApplicationEngine.h"

namespace JinEngine
{
	namespace Editor
	{
		bool JEditorStringMap::AddString(const size_t key, const std::vector<std::string>& strVec)noexcept
		{
			if (strVec.size() != (int)Core::J_LANGUAGE_TYPE::COUNT)
				return false;

			if (strMap.find(key) != strMap.end())
				return false;

			strMap.emplace(key, strVec);
			return true;
		}
		const std::string JEditorStringMap::GetString(const size_t key)const noexcept
		{
			auto data = strMap.find(key);
			if (data != strMap.end())
				return data->second[(int)JApplicationEngine::GetLanguageType()];
			else
				return "";
		}
	}
}