#include"JGuiBehaviorAdaptee.h"

#define COL32_R_SHIFT    0
#define COL32_G_SHIFT    8
#define COL32_B_SHIFT    16
#define COL32_A_SHIFT    24
#define COLFACTOR(f)  (f > 1.0f ? 1.0f : (f < 0.0f ? 0.0f : f))
#define COL32(R,G,B,A)    (((uint32)(A)<<COL32_A_SHIFT) | ((uint32)(B)<<COL32_B_SHIFT) | ((uint32)(G)<<COL32_G_SHIFT) | ((uint32)(R)<<COL32_R_SHIFT))

namespace JinEngine::Editor
{  
	uint32 JGuiBehaviorAdaptee::ConvertUColor(const JVector4<float>& color)const noexcept
	{ 
		return COL32(COLFACTOR(color.x) * 255,
			COLFACTOR(color.y) * 255,
			COLFACTOR(color.z) * 255,
			COLFACTOR(color.w) * 255);
	}
	bool JGuiBehaviorAdaptee::CanFocusByMouseRightClick()noexcept
	{
		return false;
	}
}