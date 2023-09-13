#pragma once 
#include"../Gui/JGuiType.h"
#include"../../Core/Math/JVector.h"
 
namespace JinEngine
{     
	class JCamera; 
	namespace Editor
	{
		class JEditorCameraControl
		{
		private:
			JVector2<float> preMousePos;
			//mulitiple cam pos delta
			float movementFactor = 1.0f;
		public:
			JEditorCameraControl();
			~JEditorCameraControl();
			JEditorCameraControl(const JEditorCameraControl& rhs) = delete;
			JEditorCameraControl& operator=(const JEditorCameraControl& rhs) = delete;
		public:
			void SetMousePos(const JVector2F mousePos);
		public:
			void Update(JCamera* sceneCamera, float x, float y, const J_GUI_FOCUS_FLAG_ wndFocusFlag);
		private:
			void MouseDown(JCamera* sceneCamera, float x, float y);
			void MouseUp(JCamera* sceneCamera, float x, float y);
			void MouseMove(JCamera* sceneCamera, float x, float y);
			void KeyboardInput(JCamera* sceneCamera);
		public:
			void AddMovementFactor(const float delta);
		};
	}
}