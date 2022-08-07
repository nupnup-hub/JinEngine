#pragma once

namespace JinEngine
{
	class JResourceUserInterface;
	class JValidInterface
	{
	private:
		friend class JResourceUserInterface;
	private:
		bool isValid;
	protected:
		void SetValid(bool value);
		virtual bool IsValidResource()const noexcept;
	};
}