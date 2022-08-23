#pragma once
#include<deque>
#include<string>
#include<memory>
#include"../../Core/Func/Functor/JFunctor.h"

namespace JinEngine
{

	namespace Editor
	{
		struct JEditorTask
		{
		public: 
			std::unique_ptr<Core::JBindHandleBase> bindhanle;
			std::string taskName;
		public:
			template<typename OriFunctor, typename ...BindParam>
			JEditorTask(Core::JBindHandle<OriFunctor, BindParam...>& bindhanle, const std::string& taskName);
			~JEditorTask();
			JEditorTask(JEditorTask&& lhs);
			JEditorTask& operator=(JEditorTask&& rhs);
		public:
			void Clear();
		};

		struct JEditorTaskSet
		{
		public:
			JEditorTask doExe;
			JEditorTask undoExe;
			bool isValid = false;
		public:
			void Push(JEditorTask&& doExe, JEditorTask&& undoExe);
			void Clear();
		};

		class JEditorTransition
		{
		public:
			static void Log(const std::string& str);
			static void Execute(JEditorTask&& doExe, JEditorTask&& undoExe);
			static void Undo();
			static void Redo();
			static void SetTaskCapacity(const uint8 value)noexcept;
			static void SetLogCapacity(const uint value)noexcept;
		public:
			static const std::deque<std::string>& GetLog()noexcept;
		public:
			static void Initialize();
			static void Clear();
		};
	}
}