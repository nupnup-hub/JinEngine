#pragma once
namespace JinEngine
{
	namespace Editor
	{ 
		// used by testing or debugging
		class JEditorSimpleWindow
		{
		private:
			bool isOpen = false;
		public:
			JEditorSimpleWindow() = default;
			virtual ~JEditorSimpleWindow() = default;
		public:
			bool* GetOpenPtr()noexcept;
		public:
			bool IsOpen()const noexcept;
		public:
			virtual void Update() = 0;
		};
	}
}