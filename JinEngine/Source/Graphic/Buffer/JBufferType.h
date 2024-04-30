#pragma once  
namespace JinEngine
{
	namespace Graphic
	{ 		
		enum class J_GRAPHIC_BUFFER_TYPE
		{ 
			//----GPU
			UPLOAD_BUFFER,		
			UPLOAD_CONSTANT,
			UNORDERED_ACCEESS,
			READ_BACK,
			OCC_PREDICT,
			COMMON, 
			//----CPU
			CPU
		};
	}
}