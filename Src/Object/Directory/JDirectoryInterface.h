#pragma once
#include"../JObject.h"
namespace JinEngine
{
	class JResourceManagerImpl;
	namespace Editor
	{
		class JWindowDirectory;
	}

	class JDirectoryDestroyInterface
	{
	private:
		friend class JResourceManagerImpl;
	protected:
		virtual ~JDirectoryDestroyInterface() = default;
	public:
		virtual JDirectoryDestroyInterface* DestroyInterface() = 0;
	private:
		virtual void BeginForcedDestroy() = 0;
	};

	class JDirectoryOCInterface :public JObject,
		public JDirectoryDestroyInterface
	{
	private:
		friend class Editor::JWindowDirectory;
	protected:
		JDirectoryOCInterface(const std::string& name, const size_t guid, const J_OBJECT_FLAG flag);
	public:
		virtual JDirectoryOCInterface* OCInterface() = 0;
	private:
		virtual void OpenDirectory()noexcept = 0;
		virtual void CloseDirectory()noexcept = 0;
	};
}