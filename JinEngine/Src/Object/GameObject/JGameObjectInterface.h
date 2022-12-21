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
	public:
		virtual void SetSelectedByEditorTrigger(const bool value)noexcept = 0;
	};

	class JGameObjectCompInterface
	{
	private:
		friend class JComponent;
	protected:
		virtual ~JGameObjectCompInterface() = default;
	public:
		virtual JGameObjectCompInterface* CompInterface() = 0;
	private:
		virtual bool AddComponent(JComponent& jComp)noexcept = 0;
		virtual bool RemoveComponent(JComponent& jComp)noexcept = 0;
	};

	class JGameObjectInterface : public JObject,
		public JGameObjectCompInterface,
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