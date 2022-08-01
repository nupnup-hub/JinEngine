#pragma once
#include"../EditorWindow.h" 
 
namespace JinEngine
{ 
	class LogViewer : public EditorWindow
	{ 
	public:
		LogViewer(std::unique_ptr<EditorAttribute> attribute, const size_t ownerPageGuid);
		~LogViewer() = default;
		LogViewer(const LogViewer& rhs) = delete;
		LogViewer& operator=(const LogViewer& rhs) = delete;

		void Initialize(EditorUtility* editorUtility)noexcept;
		void UpdateWindow(EditorUtility* editorUtility)override;
	private:
		void BuildLogViewer(EditorUtility* editorUtility);
	};
}
