#pragma once
#include"../JObject.h"

namespace JinEngine
{
	class JComponent;
	class JScene;
	namespace Editor
	{
		class JEditorPageShareData;
	}

	class JGameObjectEditorInterface
	{
	private:
		friend class Editor::JEditorPageShareData;
	protected:
		virtual ~JGameObjectEditorInterface() = default;
	public:
		virtual JGameObjectEditorInterface* EditorInterface() = 0;
	private:
		virtual void SetSelectedByEditorTrigger(const bool value)noexcept = 0;
	};


	class JGameObjectInterface : public JObject,
		public Core::JTypeCashInterface<JComponent>,
		public JGameObjectEditorInterface
	{ 
	private:
		friend class JScene;
	protected:
		JGameObjectInterface(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag);
	private:
		virtual Core::J_FILE_IO_RESULT CallStoreGameObject(std::wofstream& stream) = 0;
	}; 
}