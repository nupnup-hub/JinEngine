#pragma once
#include"../EditorWindow.h" 

namespace JinEngine
{ 
	class GraphicResourceWatcher : public EditorWindow
	{
	public:
		GraphicResourceWatcher(std::unique_ptr<EditorAttribute> attribute, const size_t ownerPageGuid);
		~GraphicResourceWatcher();
		GraphicResourceWatcher(const GraphicResourceWatcher& rhs) = delete;
		GraphicResourceWatcher& operator=(const GraphicResourceWatcher& rhs) = delete;
		
		void UpdateWindow(EditorUtility* editorUtility)override;
	};
}
