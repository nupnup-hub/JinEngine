#pragma once 
#include"../../../../Utility/Vector.h" 

namespace JinEngine
{     
	class JCamera; 
	class EditorCameraControl
	{ 
	protected:
		Vector2<float> preMousePos;  
	public: 
		EditorCameraControl();
		~EditorCameraControl();
		EditorCameraControl(const EditorCameraControl& rhs) = delete;
		EditorCameraControl& operator=(const EditorCameraControl& rhs) = delete;

		void MouseDown(JCamera* sceneCamera, float x, float y);
		void MouseUp(JCamera* sceneCamera, float x, float y);
		void MouseMove(JCamera* sceneCamera, float x, float y);
		void KeyboardInput(JCamera* sceneCamera);
	};
}