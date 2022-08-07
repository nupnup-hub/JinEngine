#pragma once

namespace JinEngine
{
	class JObjectInterface
	{
	protected:
		virtual bool IsActivated() const noexcept = 0;
		virtual void Activate() noexcept = 0;
		virtual void DeActivate()noexcept = 0;
	protected:
		virtual ~JObjectInterface() = default;
	};
}