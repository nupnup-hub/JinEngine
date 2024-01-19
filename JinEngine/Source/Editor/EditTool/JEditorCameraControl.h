#pragma once 
#include"../Gui/JGuiType.h"
#include"../../Core/Math/JVector.h"
#include"../../Object/JObjectModifyInterface.h"

namespace JinEngine
{     
	class JCamera; 
	namespace Editor
	{
		class JEditorCameraControl : public JObjectModifyInterface
		{
		private:
			JVector2<float> preMousePos = JVector2<float>::Zero();
			//mulitiple cam pos delta
			float movementFactor = 1.0f;
		public:
			JEditorCameraControl();
			~JEditorCameraControl();
			JEditorCameraControl(const JEditorCameraControl& rhs) = delete;
			JEditorCameraControl& operator=(const JEditorCameraControl& rhs) = delete;
		public:
			float GetMovemnetFactor()const noexcept;
		public:
			void SetMousePos(const JVector2F mousePos);
			void SetMovemnetFactor(const float factor)noexcept;
		public:
			void Update(const JUserPtr<JCamera>& cam, float x, float y, const J_GUI_FOCUS_FLAG_ wndFocusFlag);
		private:
			void MouseDown(const JUserPtr<JCamera>& cam, float x, float y);
			void MouseUp(const JUserPtr<JCamera>& cam, float x, float y);
			void MouseMove(const JUserPtr<JCamera>& cam, float x, float y);
			void KeyboardInput(const JUserPtr<JCamera>& cam);
		public:
			void AddMovementFactor(const float delta);
		};
	}
}