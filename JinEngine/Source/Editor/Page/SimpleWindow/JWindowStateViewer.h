#pragma once 
#include"JEditorSimpleWindow.h" 

namespace JinEngine
{
	namespace Editor
	{
		class JEditorWindow;
		class JWindowStateViewer : public JEditorSimpleWindow
		{
		private:
			std::vector<JEditorWindow*> wndVec; 
		public:
			void Initialize(const std::vector<JEditorWindow*>& newWndVec); 
		public:
			void Update();
		};
	}
}