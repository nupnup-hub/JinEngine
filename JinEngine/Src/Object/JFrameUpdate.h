#pragma once
#include"../Graphic/FrameResource/JFrameResourceConstant.h"  
#include"../Core/JDataType.h"
#include"../Core/Guid/GuidCreator.h"
#include<vector>

namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicImpl;
		struct JGraphicDrawTarget;
	}

	namespace FrameUpdate
	{
		using HasFrameBuff = bool; 
		static constexpr bool hasFrameBuff = true;
		static constexpr bool hasNotFrameBuff = false;
	}
#pragma region IFramaeUpdate
	template<typename ...Param>
	class IFrameUpdate
	{
	private:
		friend Graphic::JGraphicImpl;
	protected:
		virtual ~IFrameUpdate() = default;
	private:
		bool CallUpdateFrame(Param... var)
		{
			return UpdateFrame(std::forward<Param>(var)...);
		}
		void CallUpdateEnd()
		{
			UpdateEnd();
		}
		virtual bool UpdateFrame(Param... var) = 0;
		virtual void UpdateEnd() = 0;
	};

#pragma endregion

#pragma region IFrameBuff
	class JFrameBuffUserInterface;
	class JFrameBuffManagerInterface;
	class IFrameBuff
	{
	private:
		friend class JFrameBuffUserInterface;
		friend class JFrameBuffManagerInterface;
	public:
		virtual ~IFrameBuff() = default;
	protected:
		virtual int GetFrameBuffOffset()const noexcept = 0;
		virtual void SetFrameBuffOffset(int value)noexcept = 0;
	};
	class JFrameBuff : public IFrameBuff
	{
	private:
		int frameBuffOffset;
	protected:
		int GetFrameBuffOffset()const noexcept final;
		void SetFrameBuffOffset(int value)noexcept final;
	};
	class JFrameBuffUserInterface
	{
	public:
		virtual ~JFrameBuffUserInterface() = default;
	protected:
		int CallGetFrameBuffOffset(IFrameBuff& iFrameBuff)const noexcept;
	};
	class JFrameBuffManagerInterface : public JFrameBuffUserInterface
	{
	protected:
		void CallSetFrameBuffOffset(IFrameBuff& iFrameBuff, int value)const noexcept;
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

	template<typename IFrameUpdate, typename IFrameDirty, FrameUpdate::HasFrameBuff>
	class JFrameUpdate;

	template<typename FrameUpdate, typename FrameDirty>
	class JFrameUpdate<FrameUpdate, FrameDirty, true> : public FrameUpdate,
		public FrameDirty,
		public JFrameBuff
	{
	protected:
		void UpdateEnd()override
		{
			FrameDirty::MinusFrameDirty();
		}
	};

	template<typename FrameUpdate, typename FrameDirty>
	class JFrameUpdate<FrameUpdate, FrameDirty, false> : public FrameUpdate,
		public FrameDirty
	{
	protected:
		void UpdateEnd()override
		{
			FrameDirty::MinusFrameDirty();
		}
	};
}