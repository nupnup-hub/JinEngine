#pragma once
#include"../Graphic/JGraphicConstants.h"  
#include"../Core/JDataType.h"
#include"../Core/Guid/GuidCreator.h"
#include<vector>

namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicImpl;
		class JHZBOccCulling;
		struct JGraphicDrawTarget;
	}

	namespace FrameUpdate
	{
		using FrameBuffCount = int;
		static constexpr int nonBuff = 0;
		static constexpr int singleBuff = 1;
		static constexpr int dobuleBuff = 2;
	}
#pragma region IFramaeUpdate
	template<typename ...Param>
	class IFrameUpdateBase
	{ 
	private:
		friend Graphic::JGraphicImpl;
	protected:
		virtual ~IFrameUpdateBase() = default;
	private:
		virtual void UpdateFrame(Param... var) = 0;
	};

	/*
			void CallUpdateFrame(Param... var)
		{
			UpdateFrame(std::forward<Param>(var)...);
		}
	*/
	template<typename IFrameUpdateBase1>
	class IFrameUpdate1 : public IFrameUpdateBase1
	{
	private:
		using IFrameBase1 = IFrameUpdateBase1;
	private:
		friend Graphic::JGraphicImpl;
	private:
		void CallUpdateEnd()
		{
			UpdateEnd();
		}
		virtual void UpdateEnd() = 0;
	};
	template<typename IFrameUpdateBase1, typename IFrameUpdateBase2>
	class IFrameUpdate2 : public IFrameUpdateBase1,
		public IFrameUpdateBase2
	{
	private:
		using IFrameBase1 = IFrameUpdateBase1;
		using IFrameBase2 = IFrameUpdateBase2;
	private:
		friend Graphic::JGraphicImpl;
	private:
		void CallUpdateEnd()
		{
			UpdateEnd();
		}
		virtual void UpdateEnd() = 0;
	};

	template<typename IFrameUpdateBase1, typename IFrameUpdateBase2, typename IFrameUpdateBase3>
	class IFrameUpdate3 : public IFrameUpdateBase1,
		public IFrameUpdateBase2,
		public IFrameUpdateBase3
	{
	private:
		using IFrameBase1 = IFrameUpdateBase1;
		using IFrameBase2 = IFrameUpdateBase2;
		using IFrameBase3 = IFrameUpdateBase3;
	private:
		friend Graphic::JGraphicImpl;
	private:
		void CallUpdateEnd()
		{
			UpdateEnd();
		}
		virtual void UpdateEnd() = 0;
	};
#pragma endregion

#pragma region IFrameBuff
	class JFrameBuffUserInterface;
	class JFrameBuffManagerInterface;

	class IFrameBuff1
	{
	private:
		friend class JFrameBuffUserInterface;
		friend class JFrameBuffManagerInterface;
	public:
		virtual ~IFrameBuff1() = default;
	protected:
		virtual int GetFrameBuffOffset()const noexcept = 0;
		virtual void SetFrameBuffOffset(int value)noexcept = 0;
	};

	class IFrameBuff2
	{
	private:
		friend class JFrameBuffUserInterface;
		friend class JFrameBuffManagerInterface;
	public:
		virtual ~IFrameBuff2() = default;
	protected:
		virtual int GetFirstFrameBuffOffset()const noexcept = 0;
		virtual int GetSecondFrameBuffOffset()const noexcept = 0;
		virtual void SetFirstFrameBuffOffset(int value)noexcept = 0;
		virtual void SetSecondFrameBuffOffset(int value)noexcept = 0;
	};
	class JFrameBuff1 : public IFrameBuff1
	{
	private:
		int frameBuffOffset;
	protected:
		int GetFrameBuffOffset()const noexcept final;
		void SetFrameBuffOffset(int value)noexcept final;
	};
	class JFrameBuff2 : public IFrameBuff2
	{
	private:
		int frameBuffOffset00;
		int frameBuffOffset01;
	protected:
		int GetFirstFrameBuffOffset()const noexcept final;
		int GetSecondFrameBuffOffset()const noexcept final;
		void SetFirstFrameBuffOffset(int value)noexcept final;
		void SetSecondFrameBuffOffset(int value)noexcept final;
	};

	class JFrameBuffUserInterface
	{
	public:
		virtual ~JFrameBuffUserInterface() = default;
	protected:
		int CallGetFrameBuffOffset(IFrameBuff1& iFrameBuff)const noexcept;
	protected:
		int CallGetFirstFrameBuffOffset(IFrameBuff2& iFrameBuff)const noexcept;
		int CallGetSecondFrameBuffOffset(IFrameBuff2& iFrameBuff)const noexcept;
	};

	class JFrameBuffManagerInterface : public JFrameBuffUserInterface
	{
	protected:
		void CallSetFrameBuffOffset(IFrameBuff1& iFrameBuff, int value)const noexcept;
	protected:
		void CallSetFirstFrameBuffOffset(IFrameBuff2& iFrameBuff, int value)const noexcept;
		void CallSetSecondFrameBuffOffset(IFrameBuff2& iFrameBuff, int value)const noexcept;
	};
#pragma endregion

#pragma region IFrameDirty
	class IFrameDirtyTrigger
	{
	protected:
		virtual ~IFrameDirtyTrigger() = default;
	protected:
		virtual void SetFrameDirty()noexcept = 0;
	};

	class JFrameDirtyTrigger : public IFrameDirtyTrigger
	{
	protected:
		void SetFrameDirty()noexcept override;
	};

	class IFrameDirty
	{
	private:
		friend Graphic::JGraphicImpl;
		friend Graphic::JHZBOccCulling;
		friend Graphic::JGraphicDrawTarget;
	protected:
		virtual ~IFrameDirty() = default;
	protected:
		virtual bool IsFrameDirted()const noexcept = 0;
		virtual void SetFrameDirty()noexcept = 0;
		virtual void MinusFrameDirty()noexcept = 0;
		virtual void OffFrameDirty()noexcept = 0;
	protected:
		//Debug
		virtual int GetFrameDirty()noexcept = 0;
	};

	class JFrameDirty : public IFrameDirty
	{
	private:
		int frameDirty;
	protected:
		bool IsFrameDirted()const noexcept final;
		void SetFrameDirty()noexcept override;
		void MinusFrameDirty()noexcept final;
		void OffFrameDirty()noexcept final;
	protected:
		int GetFrameDirty()noexcept final;
	};
	template<typename T> class IFrameDirtyObserver;
	template<typename T> class JFrameDirtyObserver;

	class JFrameDirtyListener : public JFrameDirty
	{
	private:
		template<typename T> friend class JFrameDirtyObserver;
	private:
		const size_t guid = Core::MakeGuid();
	protected:
		template<typename T>
		void RegisterFrameDirtyListener(IFrameDirtyObserver<T>& observer)
		{
			observer.AddFrameDirtyListener(*this);
		}
		template<typename T>
		void DeRegisterFrameDirtyListener(IFrameDirtyObserver<T>& observer)
		{
			observer.RemoveFrameDirtyListener(*this);
		}
	};

	template<typename FrameDirty>
	class IFrameDirtyObserver : public FrameDirty
	{
	private:
		friend class JFrameDirtyListener;
	protected:
		virtual void AddFrameDirtyListener(JFrameDirtyListener& newListener)noexcept = 0;
		virtual void RemoveFrameDirtyListener(JFrameDirtyListener& oldListener)noexcept = 0;
	};

	template<typename FrameDirty>
	class JFrameDirtyObserver : public IFrameDirtyObserver<FrameDirty>
	{
	private:
		std::vector<JFrameDirtyListener*> listener;
	protected:
		void SetFrameDirty()noexcept final
		{
			IFrameDirtyObserver<FrameDirty>::SetFrameDirty();
			const uint listenerCount = (uint)listener.size();
			for (uint i = 0; i < listenerCount; ++i)
				listener[i]->SetFrameDirty();
		}
	protected:
		void AddFrameDirtyListener(JFrameDirtyListener& newListener)noexcept final
		{
			listener.push_back(&newListener);
		}
		void RemoveFrameDirtyListener(JFrameDirtyListener& oldListener)noexcept final
		{
			uint listenerCount = (uint)listener.size();
			for (uint i = 0; i < listenerCount; ++i)
			{
				if (listener[i]->guid == oldListener.guid)
				{
					listener.erase(listener.begin() + i);
					break;
				}
			}
		}
	};
#pragma endregion

	template<typename IFrameUpdate, typename IFrameDirty, FrameUpdate::FrameBuffCount>
	class JFrameUpdate;

	template<typename FrameUpdate, typename FrameDirty>
	class JFrameUpdate<FrameUpdate, FrameDirty, 2> : public FrameUpdate,
		public FrameDirty,
		public JFrameBuff2
	{
	protected:
		void UpdateEnd()override
		{
			FrameDirty::MinusFrameDirty();
		}
	};
	template<typename FrameUpdate, typename FrameDirty>
	class JFrameUpdate<FrameUpdate, FrameDirty, 1> : public FrameUpdate,
		public FrameDirty,
		public JFrameBuff1
	{
	protected:
		void UpdateEnd()override
		{
			FrameDirty::MinusFrameDirty();
		}
	};

	template<typename FrameUpdate, typename FrameDirty>
	class JFrameUpdate<FrameUpdate, FrameDirty, 0> : public FrameUpdate,
		public FrameDirty
	{
	protected:
		void UpdateEnd()override
		{
			FrameDirty::MinusFrameDirty();
		}
	};
}