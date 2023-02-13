#pragma once
#include<deque>
#include<string>
#include<memory>
#include"../Func/Functor/JFunctor.h"
#include"../Handle/JDataHandleStructure.h"

namespace JinEngine
{
	namespace Core
	{
		class JTransitionTask
		{
		public:
			enum class ADDITONAL_PROCESS_TYPE
			{
				DO_PRE,
				DO_POST,
				UNDO_PRE,
				UNDO_POST
			};
		public:
			using ProcessBindVec = std::vector<std::unique_ptr<JBindHandleBase>>;
		private:
			ProcessBindVec preprocessDoVec;
			ProcessBindVec postprocessDoVec;
			ProcessBindVec preprocessUndoVec;
			ProcessBindVec postprocessUndoVec;
		private: 
			std::string doTaskName;
			std::string undoTaskName;
		public: 
			JTransitionTask(const std::string& doTaskName, const std::string& undoTaskName);
			virtual ~JTransitionTask();
		public:
			std::string GetDoTaskName()const noexcept;
			std::string GetUndoTaskName()const noexcept;
		public:
			void RegisterAddtionalProcess(const ADDITONAL_PROCESS_TYPE type, ProcessBindVec&& vec);
		public:
			virtual void Do() = 0;
			virtual void Undo() = 0;
			virtual void Clear() noexcept;
		protected:
			void Process(const ADDITONAL_PROCESS_TYPE type);
		public:
			virtual bool IsValid()const noexcept = 0;
		};
		 
		class JTransitionSetValueTask final : public JTransitionTask
		{
		private:
			std::unique_ptr<JBindHandleBase> doBindhandle;
			std::unique_ptr<JBindHandleBase> undoBindhandle;
		public: 
			JTransitionSetValueTask(const std::string& taskName,
				std::unique_ptr<JBindHandleBase>&& doBindhandle,
				std::unique_ptr<JBindHandleBase>&& undoBindhandle);
		public:
			void Do() final;
			void Undo() final;
			void Clear()noexcept final;
		public:
			bool IsValid()const noexcept;
		};

		template<typename DataStructure,  typename DoHandle, typename UndoHandle>
		class JTransitionCreationTask final : public JTransitionTask
		{
		private:
			std::unique_ptr<DoHandle> doBindhandle = nullptr;
			std::unique_ptr<UndoHandle> undoBindhandle = nullptr;
			DataStructure& dataStructure;
			JDataHandle dataHandle;
		public: 
			JTransitionCreationTask(const std::string& taskName,
				std::unique_ptr<DoHandle> doBindhandle,
				std::unique_ptr<UndoHandle> undoBindhandle,
				DataStructure& dataStructure)
				:JTransitionTask("Do" + taskName, "Undo"+ taskName),
				doBindhandle(std::move(doBindhandle)),
				undoBindhandle(std::move(undoBindhandle)),
				dataStructure(dataStructure),
				dataHandle(dataStructure.CreateInvalidHandle())
			{}
			~JTransitionCreationTask()
			{
				Clear();
			}
		public:
			void Do() final
			{
				Process(JTransitionTask::ADDITONAL_PROCESS_TYPE::DO_PRE);
				doBindhandle->Invoke<DataStructure&, JDataHandle&>(dataStructure, dataHandle);
				Process(JTransitionTask::ADDITONAL_PROCESS_TYPE::DO_POST);
			}
			void Undo() final
			{
				Process(JTransitionTask::ADDITONAL_PROCESS_TYPE::UNDO_PRE);
				undoBindhandle->Invoke<DataStructure&, JDataHandle&>(dataStructure, dataHandle);
				Process(JTransitionTask::ADDITONAL_PROCESS_TYPE::UNDO_POST);
			}
			void Clear()noexcept final
			{
				JTransitionTask::Clear();
				doBindhandle.reset();
				undoBindhandle.reset();
				if (dataHandle.IsValid())
					dataStructure.Remove(dataHandle);			 
			}
		public:
			bool IsValid()const noexcept
			{
				return doBindhandle != nullptr && undoBindhandle != nullptr;
			}
		};
		
		template<typename DataStructure, typename ...Param>
		struct JTransitionCreationHandleType
		{ 
		public:
			using Functor = Core::JFunctor<void, DataStructure&, Core::JDataHandle&, Param...>;
			using Bind = Core::JBindHandle<Functor, const Core::EmptyType&, const Core::EmptyType&, Param...>;
		};
 

		//Make transition create task bind unq
		template<typename BindType, typename Functor, typename ...Param>
		static std::unique_ptr<BindType> CTaskUptr(Functor& functor, Param... var)
		{
			return std::make_unique<BindType>(functor, empty, empty, std::forward<Param>(var)...);
		}

		class JTransition
		{
		public:
			static void Log(const std::string& str);
			static void Execute(std::unique_ptr<JTransitionTask> task);
			static void Undo();
			static void Redo();
			static void SetTaskCapacity(const uint8 value)noexcept;
			static void SetLogCapacity(const uint value)noexcept;
		public:
			static constexpr uint8 GetMaxTaskCapacity() noexcept
			{
				return 50;
			}
			static constexpr uint GetMaxLogCapacity() noexcept
			{
				return 50;
			}
		public:
			static const std::deque<std::string>& GetLog()noexcept;
		public:
			static void Initialize();
			static void Clear();
		};
	}
}