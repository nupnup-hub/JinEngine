#pragma once
#include"JEditorInputBuffHelper.h" 
#include"../../Object/Directory/JDirectory.h" 

namespace JinEngine
{
	namespace Editor
	{
		class JEditorAddressBar
		{ 
		private: 
			JUserPtr<JDirectory> initDir;
			JUserPtr<JDirectory> opendDirectory; 
		private:
			JUserPtr<JDirectory> lastOpenList; 
		public:
			JEditorAddressBar(const JUserPtr<JDirectory>& initDir);
		public:
			/**
			* @return changed openDirectory
			*/
			bool Update();
		public:
			JDirectory* GetOpendDirectoryRaw()const noexcept;
			JUserPtr<JDirectory> GetOpendDirectory()const noexcept; 
		public: 
			void SetOpendDirectory(const JUserPtr<JDirectory>& dir);
		};
	}
}