#pragma once

namespace JinEngine
{
	class JClearableInterface
	{
	private:
		virtual void StuffResource() = 0;
		virtual void ClearResource() = 0;
	};
}