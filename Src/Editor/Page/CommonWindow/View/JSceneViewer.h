#pragma once
#include"../../JEditorWindow.h"
#include"../../../../Utility/JDelegate.h"    
#include"../../../../Utility/JVector.h"    

namespace JinEngine
{ 
	class JCamera;
	namespace Editor
	{
		class JEditorCameraControl;
		class JSceneViewer : public JEditorWindow
		{
		private:
			JCamera* selectedCamera;
			std::unique_ptr<JEditorCameraControl> editorCamCtrl;

			//�����ʿ�
			//��Ʈ���������ڵ�
			bool onAnimation = false;
		public:
			JSceneViewer(std::unique_ptr<JEditorAttribute> attribute, const size_t ownerPageGuid);
			~JSceneViewer();
			JSceneViewer(const JSceneViewer& rhs) = delete;
			JSceneViewer& operator=(const JSceneViewer& rhs) = delete;
			bool Activate(JEditorUtility* editorUtility) final;
			bool DeActivate(JEditorUtility* editorUtility) final;
			void StoreEditorWindow(std::wofstream& stream)final;
			void LoadEditorWindow(std::wifstream& stream)final;
			void UpdateWindow(JEditorUtility* editorUtility)override;
		};
	}
}
