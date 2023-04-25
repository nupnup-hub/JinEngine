#pragma once
#include"../Graphic/JGraphicConstants.h"  
#include"../Core/Empty/EmptyType.h"
#include"../Core/JDataType.h"
#include"../Core/Guid/GuidCreator.h"
#include<vector>

namespace JinEngine
{
	namespace FrameUpdate
	{
		using FrameBuffCount = int;
		static constexpr int nonBuff = 0;
		static constexpr int singleBuff = 1;
		static constexpr int dobuleBuff = 2;
	}
#pragma region IFramaeUpdate
	template<typename ...Param>
	class JFrameUpdateBase
	{  
	protected:
		virtual ~JFrameUpdateBase() = default;
	private:
		virtual void UpdateFrame(Param... var)noexcept = 0;
	};

	/*
			void CallUpdateFrame(Param... var)
		{
			UpdateFrame(std::forward<Param>(var)...);
		}
	*/
	template<typename JFrameUpdateBase1>
	class JFrameUpdate1 : public JFrameUpdateBase1
	{
	private:
		using JFrameBase1 = JFrameUpdateBase1; 
	private:
		virtual void UpdateEnd() = 0;
	};
	template<typename JFrameUpdateBase1, typename JFrameUpdateBase2>
	class JFrameUpdate2 : public JFrameUpdateBase1,
		public JFrameUpdateBase2
	{
	private:
		using JFrameBase1 = JFrameUpdateBase1;
		using JFrameBase2 = JFrameUpdateBase2; 
	private:
		virtual void UpdateEnd() = 0;
	};

	template<typename JFrameUpdateBase1, typename JFrameUpdateBase2, typename JFrameUpdateBase3>
	class JFrameUpdate3 : public JFrameUpdateBase1,
		public JFrameUpdateBase2,
		public JFrameUpdateBase3
	{
	private:
		using JFrameBase1 = JFrameUpdateBase1;
		using JFrameBase2 = JFrameUpdateBase2;
		using JFrameBase3 = JFrameUpdateBase3; 
	private:
		virtual void UpdateEnd() = 0;
	};
#pragma endregion

#pragma region IFrameBuff 

	class JFrameBuff1 
	{
	private:
		uint frameBuffOffset;
	public:
		uint GetFrameBuffOffset()const noexcept;
		void SetFrameBuffOffset(const uint value)noexcept;
	};
	class JFrameBuff2
	{
	private:
		uint frameBuffOffset00;
		uint frameBuffOffset01;
	public:
		uint GetFirstFrameBuffOffset()const noexcept;
		uint GetSecondFrameBuffOffset()const noexcept;
		void SetFirstFrameBuffOffset(const uint value)noexcept;
		void SetSecondFrameBuffOffset(const uint value)noexcept;
	};
#pragma endregion

#pragma region JFrameDirtyBase
	class JFrameDirtyTriggerBase
	{
	protected:
		virtual ~JFrameDirtyTriggerBase() = default;
	protected:
		virtual void SetFrameDirty()noexcept = 0;
	};
	 
	//just call empty func
	class JFrameDirtyTrigger : public JFrameDirtyTriggerBase
	{
	protected:
		void SetFrameDirty()noexcept override;
	};

	class JFrameDirtyBase
	{ 
	protected:
		virtual ~JFrameDirtyBase() = default;
	public:
		virtual bool IsFrameDirted()const noexcept = 0;
	};

	class JFrameDirty : public JFrameDirtyBase
	{
	private:
		int frameDirty;
	public:
		bool IsFrameDirted()const noexcept final;
		int GetFrameDirty()noexcept;
		virtual void SetFrameDirty()noexcept;
		void MinusFrameDirty()noexcept;
		void OffFrameDirty()noexcept ;
	};

	class JFrameDirtyListener : public JFrameDirty
	{ 
	public:
		const size_t guid = Core::MakeGuid();
	};

	template<typename FrameDirty>
	class JFrameDirtyChain : public FrameDirty
	{
	private:
		std::vector<JFrameDirtyListener*> listener;
	protected:
		void SetFrameDirty()noexcept final
		{ 
			FrameDirty::SetFrameDirty();
			const uint listenerCount = (uint)listener.size();
			for (uint i = 0; i < listenerCount; ++i)
				listener[i]->SetFrameDirty();
		}
	protected:
		void AddFrameDirtyListener(JFrameDirtyListener* newListener)noexcept
		{
			listener.push_back(newListener);
		}
		void RemoveFrameDirtyListener(JFrameDirtyListener* oldListener)noexcept
		{
			uint listenerCount = (uint)listener.size();
			for (uint i = 0; i < listenerCount; ++i)
			{
				if (listener[i]->guid == oldListener->guid)
				{
					listener.erase(listener.begin() + i);
					break;
				}
			}
		}
	};
#pragma endregion

	template<typename IFrameUpdate, typename JFrameDirtyBase, FrameUpdate::FrameBuffCount>
	class JFrameUpdate;

	//use update and dirty and 2buff
	template<typename FrameUpdate, typename FrameDirty>
	class JFrameUpdate<FrameUpdate, FrameDirty, 2> : public FrameUpdate,
		public FrameDirty,
		public JFrameBuff2
	{
	public:
		void UpdateEnd()override
		{
			FrameDirty::MinusFrameDirty();
		}
	};

	//use update and dirty and 1buff
	template<typename FrameUpdate, typename FrameDirty>
	class JFrameUpdate<FrameUpdate, FrameDirty, 1> : public FrameUpdate,
		public FrameDirty,
		public JFrameBuff1
	{
	public:
		void UpdateEnd()override
		{
			FrameDirty::MinusFrameDirty();
		}
	};

	//use update and dirty
	template<typename FrameUpdate, typename FrameDirty>
	class JFrameUpdate<FrameUpdate, FrameDirty, 0> : public FrameUpdate,
		public FrameDirty
	{
	public:
		void UpdateEnd()override
		{
			FrameDirty::MinusFrameDirty();
		}
	};

	//use update and 2buff
	template<typename FrameUpdate>
	class JFrameUpdate<FrameUpdate, Core::EmptyType, 2> : public FrameUpdate, public JFrameBuff2
	{
	public:
		void UpdateEnd()override
		{}
	};

	//use update and 1buff
	template<typename FrameUpdate>
	class JFrameUpdate<FrameUpdate, Core::EmptyType, 1> : public FrameUpdate, public JFrameBuff1
	{
	public:
		void UpdateEnd()override
		{}
	};

	//only use update
	template<typename FrameUpdate>
	class JFrameUpdate<FrameUpdate, Core::EmptyType, 0> : public FrameUpdate
	{
	public:
		void UpdateEnd()override
		{}
	};
}