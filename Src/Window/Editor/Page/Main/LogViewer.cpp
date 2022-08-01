#include"LogViewer.h"
#include"../EditorAttribute.h"
#include"../Debug/DebugHelper.h" 
#include"../../Utility/EditorUtility.h" 
#include"../../../../Utility/JCommonUtility.h"

namespace JinEngine
{
	LogViewer::LogViewer(std::unique_ptr<EditorAttribute> attribute, const size_t ownerPageGuid)
		:EditorWindow(std::move(attribute), ownerPageGuid)
	{

	}
	void LogViewer::Initialize(EditorUtility* editorUtility)noexcept
	{
		//editorUtility->editorsEvent.connect(this, &LogViewer::OnEditorEvent);
	}
	void LogViewer::UpdateWindow(EditorUtility* editorUtility)
	{
		EditorWindow::UpdateWindow(editorUtility);
		BuildLogViewer(editorUtility);
	}
	void LogViewer::BuildLogViewer(EditorUtility* editorUtility)
	{
		DebugHelper::PrintMessage(); 
	}
}