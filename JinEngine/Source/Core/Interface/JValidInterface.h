#pragma once

namespace JinEngine
{
	namespace Core
	{ 
		class JValidInterface
		{
		private:
			bool isValid = false;
		public:
			JValidInterface() = default;
			virtual ~JValidInterface();
		public:
			virtual bool IsValid()const noexcept;
			bool IsValidTriggerOn()const noexcept;
		protected:
			void SetValid(bool value);
		};
	}
}