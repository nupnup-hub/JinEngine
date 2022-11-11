#pragma once 
#include"../../../../Utility/JVector.h" 

namespace JinEngine
{     
	class JCamera; 
	namespace Editor
	{
		class JEditorCameraControl
		{
		protected:
			JVector2<float> preMousePos;
		public:
			JEditorCameraControl();
			~JEditorCameraControl();
			JEditorCameraControl(const JEditorCameraControl& rhs) = delete;
			JEditorCameraControl& operator=(const JEditorCameraControl& rhs) = delete;

			void MouseDown(JCamera* sceneCamera, float x, float y);
			void MouseUp(JCamera* sceneCamera, float x, float y);
			void MouseMove(JCamera* sceneCamera, float x, float y);
			void KeyboardInput(JCamera* sceneCamera);
		};
	}
}