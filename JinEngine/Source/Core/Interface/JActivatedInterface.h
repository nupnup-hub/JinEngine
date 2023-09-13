#pragma once

namespace JinEngine
{
	namespace Core
	{
		class JActivatedInterface
		{
		private:
			bool isActivated = false;
		public:
			virtual ~JActivatedInterface() = default;
		public:
			bool IsActivated() const noexcept;
		protected:
			/**
			* @return true
			*/
			virtual bool CanActivate() const noexcept;
			/**
			* @return true
			*/
			virtual bool CanDeActivate() const noexcept;
		protected:
			void Activate() noexcept;
			void DeActivate()noexcept;
		protected:
			virtual void DoActivate() noexcept;
			virtual void DoDeActivate() noexcept;
		};
	}
}