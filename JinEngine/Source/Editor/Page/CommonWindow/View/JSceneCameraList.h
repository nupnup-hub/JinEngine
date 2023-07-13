#pragma once
#include"../../../../Core/JDataType.h"
#include"../../../../Core/Pointer/JOwnerPtr.h"
#include "../../../../Utility/JVector.h"
#include<string>

namespace JinEngine
{
	class JScene;
	class JCamera;
	namespace Editor
	{
		class JSceneCameraList
		{
		private:
			uint preSelected = 0;
			bool canDisplayEditorCam = false;
		public:
			void DisplayCameraList(const JUserPtr<JScene>& scene, const std::string& unqLabel, const JVector2<float> size)noexcept;
		public:
			uint GetSelecetdIndex()const noexcept;
			JUserPtr<JCamera> GetSelectedCam(const JUserPtr<JScene>& scene)const noexcept;
			void SetSelecetdIndex(const uint selected)noexcept;
		public:
			void Update(const JUserPtr<JScene>& scene)noexcept;
		};
	}
}