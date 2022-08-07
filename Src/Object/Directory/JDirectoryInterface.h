#pragma once
#include"../JObject.h"
namespace JinEngine
{
	class WindowDirectory;
	class JDirectoryInterface : public JObject
	{
	private:
		friend class WindowDirectory;
	protected:
		JDirectoryInterface(const std::string& name, const size_t guid, const JOBJECT_FLAG flag);
	public:
		virtual JDirectoryInterface* DirectoryInterface() = 0;
	private:
		virtual void OpenDirectory() = 0;
		virtual void CloseDirectory() = 0;
	};
}