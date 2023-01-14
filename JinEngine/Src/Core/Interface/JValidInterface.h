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
			virtual bool IsValid()const noexcept;
		protected:
			void SetValid(bool value);
		};
	}
}