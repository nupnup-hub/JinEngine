#pragma once
#include"../../JEditorWindow.h"  
#include"../../../../Utility/JVector.h"
#include<memory>

namespace JinEngine
{     
	namespace Editor
	{
		class JEditorCameraControl;
		class JSceneEditor : public JEditorWindow
		{
		protected:
			std::unique_ptr<JEditorCameraControl> editorCamCtrl;
		public:
			JSceneEditor(std::unique_ptr<JEditorAttribute> attribute, const size_t ownerPageGuid);
			~JSceneEditor();
			JSceneEditor(const JSceneEditor& rhs) = delete;
			JSceneEditor& operator=(const JSceneEditor& rhs) = delete;
		};
	}
}