#pragma once

namespace JinEngine
{
	__interface IFrameResourceControl
	{
	public:
		bool IsDirtied()const noexcept;
		void SetDirty()noexcept;
		void OffDirty()noexcept;
		void MinusDirty()noexcept;
	};
}