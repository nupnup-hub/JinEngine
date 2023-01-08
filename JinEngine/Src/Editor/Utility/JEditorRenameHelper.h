#pragma once
#include"../../Core/Pointer/JOwnerPtr.h"
#include"../../Core/Func/Functor/JFunctor.h"
#include<memory>
#include<string>

namespace JinEngine
{
	class JObject;
	namespace Editor
	{
		class JEditorInputBuffHelper;
		class JEditorRenameHelper
		{
		public:
			using RenameF = Core::JFunctor<void, const std::string, Core::JUserPtr<JObject>>;
			using ActivateF = Core::JFunctor<void, Core::JUserPtr<JObject>>;
		private:
			Core::JUserPtr<JObject> renameTar;
			std::unique_ptr <JEditorInputBuffHelper> renameBuff;
			std::unique_ptr<RenameF> renameF;
			std::unique_ptr<ActivateF> activateF;
		public:
			JEditorRenameHelper();
		public:
			void Clear()noexcept;
		public:
			void Update(const std::string& uniqueLabel, const bool doIdent);
		public:
			void Activate(Core::JUserPtr<JObject> newRenameTar)noexcept;
		public:
			ActivateF* GetActivateFunctor()const noexcept;
		public:
			bool IsActivated()const noexcept;
			bool IsRenameTar(const size_t guid)const noexcept;
		};
	}
}