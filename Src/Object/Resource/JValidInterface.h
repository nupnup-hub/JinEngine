#pragma once

namespace JinEngine
{
	class JResourceUserInterface;
	class JValidInterface
	{
	private:
		friend class JResourceUserInterface;
	private:
		bool isValid = false;
	protected:
		void SetValid(bool value); 
	protected:
		virtual bool IsValid()const noexcept;
	};
}