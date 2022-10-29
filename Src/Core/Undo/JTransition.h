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
			virtual void Do() = 0;
			virtual void Undo() = 0;
			virtual void Clear() noexcept = 0;
		public:
			virtual bool IsValid()const noexcept = 0;
		};
		 
		class JTransitionSetValueTask final : public JTransitionTask
		{
		private:
			std::unique_ptr<Core::JBindHandleBase> doBindhanle;
			std::unique_ptr<Core::JBindHandleBase> undoBindhanle;
		public: 
			JTransitionSetValueTask(const std::string& taskName,
				std::unique_ptr<Core::JBindHandleBase> doBindhanle,
				std::unique_ptr<Core::JBindHandleBase> undoBindhanle);
		public:
			void Do() final;
			void Undo() final;
			void Clear()noexcept final;
		public:
			bool IsValid()const noexcept;
		};

		template<typename DataStructure,  typename doHandle, typename undoHandle>
		class JTransitionCreationTask final : public JTransitionTask
		{
		private:
			std::unique_ptr<doHandle> doBindhanle;
			std::unique_ptr<undoHandle> undoBindhanle;
			DataStructure& dataStructure;
			JDataHandle dataHandle;
		public: 
			JTransitionCreationTask(const std::string& taskName,
				std::unique_ptr<doHandle> doBindhanle,
				std::unique_ptr<undoHandle> undoBindhanle,
				DataStructure& dataStructure)
				:JTransitionTask("Do" + taskName, "Undo"+ taskName),
				doBindhanle(std::move(doBindhanle)),
				undoBindhanle(std::move(undoBindhanle)),
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
				doBindhanle->Invoke<DataStructure&, JDataHandle&>(dataStructure, dataHandle);
			}
			void Undo() final
			{
				undoBindhanle->Invoke<DataStructure&, JDataHandle&>(dataStructure, dataHandle);
			}
			void Clear()noexcept final
			{
				doBindhanle.reset();
				undoBindhanle.reset();
				if (dataHandle.IsValid())
					dataStructure.Remove(dataHandle);
			}
		public:
			bool IsValid()const noexcept
			{
				return doBindhanle != nullptr && undoBindhanle != nullptr;
			}
		};

		//Make transition create task bind unq
		template<typename BindType, typename Functor, typename ...Param>
		static std::unique_ptr<BindType> CTaskUptr(Functor& functor, Param... var)
		{
			return std::make_unique<BindType>(functor, Core::empty, Core::empty, std::forward<Param>(var)...);
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