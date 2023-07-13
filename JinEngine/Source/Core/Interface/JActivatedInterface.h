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
			bool IsActivated() const noexcept;
		protected:
			void Activate() noexcept;
			void DeActivate()noexcept;
		protected:
			virtual void DoActivate() noexcept;
			virtual void DoDeActivate() noexcept;
		};
	}
}