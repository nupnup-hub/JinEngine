#pragma once
#include"../JObject.h"
namespace JinEngine
{
	class JResourceManagerImpl;
	namespace Editor
	{
		class JWindowDirectory;
	}

	class JDirectoryOCInterface
	{
	private:
		friend class Editor::JWindowDirectory;
		friend class JResourceManagerImpl;
	protected:
		virtual ~JDirectoryOCInterface() = default;
	public:
		virtual JDirectoryOCInterface* OCInterface() = 0;
	private:
		virtual void OpenDirectory()noexcept = 0;
		virtual void CloseDirectory()noexcept = 0;
	};

	class JDirectoryInterface :public JObject,
		public JDirectoryOCInterface
	{
	private:
		friend class JResourceManagerImpl;
	protected:
		JDirectoryInterface(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag);
	};
}