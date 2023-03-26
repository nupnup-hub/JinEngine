#pragma once

namespace JinEngine
{
	class JResourceUserInterface;
	class JReferenceInterface
	{
	private:
		friend class JResourceUserInterface;
	private:
		int referenceCount = 0;
	protected:
		virtual ~JReferenceInterface() = default;
	protected:
		int GetReferenceCount()const noexcept;
		virtual void OnReference()noexcept;
		virtual void OffReference()noexcept;
	};
}