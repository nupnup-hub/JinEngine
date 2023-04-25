#pragma once

namespace JinEngine
{
	class JClearableInterface
	{
	public:
		virtual ~JClearableInterface() = default;
	private:
		virtual void StuffResource() = 0;
		virtual void ClearResource() = 0;
	};
}