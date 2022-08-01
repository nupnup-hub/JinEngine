#pragma once
#include"Editor.h" 

namespace JinEngine 
{ 
	class JScene;
	class JCamera;
	class EditorAttribute;
	class EditorUtility;

	class EditorWindow : public Editor
	{
	protected:  
		const size_t ownerPageGuid;
	public:
		EditorWindow(std::unique_ptr<EditorAttribute> attribute, const size_t ownerPageGuid);
		virtual~EditorWindow();
		EditorWindow(const EditorWindow& rhs) = delete;
		EditorWindow& operator=(const EditorWindow& rhs) = delete;

		virtual void EnterWindow(EditorUtility* editorUtility);
		virtual void UpdateWindow(EditorUtility* editorUtility);
		void UpdateDocking(EditorUtility* editorUtility);
		virtual void CloseWindow(); 
		virtual void StoreEditorWindow(std::wofstream& stream);
		virtual void LoadEditorWindow(std::wifstream& stream);

		size_t GetOwnerPageGuid()const noexcept; 
	protected:
		void SetButtonSelectColor()noexcept;
		void SetButtonDefaultColor()noexcept;
		void SetTreeNodeSelectColor()noexcept;
		void SetTreeNodeDefaultColor()noexcept;
	};
}