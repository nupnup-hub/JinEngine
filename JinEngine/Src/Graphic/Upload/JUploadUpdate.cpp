#include"JUploadUpdate.h"

namespace JinEngine
{
	namespace Graphic
	{
		void JUploadDirty::SetUploadDirty()noexcept
		{
			uploadDirty = true;
		}
		void JUploadDirty::OffUploadDirty()noexcept
		{
			uploadDirty = false;
		}
		bool JUploadDirty::IsUploadDirted()const noexcept
		{
			return uploadDirty;
		}
	}
}