#pragma once  
namespace JinEngine
{
	namespace Graphic
	{ 		
		enum class J_GRAPHIC_BUFFER_TYPE
		{ 
			UPLOAD_COMMON,
			UPLOAD_CONSTANT,
			UNORDERED_ACCEESS,
			READ_BACK,
			OCC_PREDICT
		};
	}
}