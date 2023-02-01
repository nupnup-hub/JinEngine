#pragma once
#include"../../Core/Pointer/JOwnerPtr.h"
#include"../../Core/Func/Functor/JFunctor.h"
#include"../../Utility/JVector.h"
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
		private:
			Core::JUserPtr<JObject> renameTar;
			std::unique_ptr <JEditorInputBuffHelper> renameBuff;
			std::unique_ptr<RenameF> renameF;
		private:
			std::string uniqueLabel; 
		public:
			JEditorRenameHelper();
		public:
			void Clear()noexcept;
		public:
			void Update(const bool doIdent);
			void UpdateMultiline(const JVector2<float>& size, const bool doIdent);
		private:
			void DoUpdate(const bool doIdent, const JVector2<float>& size = JVector2<float>(0, 0), const bool isMultiline = false);
		public:
			void Activate(Core::JUserPtr<JObject> newRenameTar)noexcept;
		public:
			bool IsActivated()const noexcept;
			bool IsRenameTar(const size_t guid)const noexcept;
		public:
			void SetBuffSize(const uint newBuffSize)noexcept;
		};
	}
}