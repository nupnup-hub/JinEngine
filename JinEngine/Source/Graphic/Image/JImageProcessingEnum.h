#pragma once
#include"../../Core/Reflection/JReflection.h"

namespace JinEngine
{
	namespace Graphic
	{
		enum class J_DOWN_SAMPLING_TYPE
		{
			BOX,
			GAUSIAAN,
			KAISER,
			COUNT
		};
		enum class J_BLUR_TYPE
		{
			BOX,
			GAUSIAAN,
			//BILATERAL,
			COUNT
		}; 
		REGISTER_ENUM_CLASS(J_SSAO_TYPE ,int, DEFAULT, HORIZON_BASED)
		REGISTER_ENUM_CLASS(J_KERNEL_SIZE, int, _3x3,
			_5x5,
			_7x7)//has order dependency

		REGISTER_ENUM_CLASS(J_SSAO_SAMPLE_TYPE, int,
			LOW,		 
			NORMAL,		 
			HIGH,
			ULTRA)// 1/8, 1/4, 1/2, 1

		enum class J_SSAO_BLUR_TYPE
		{
			BILATERAL, 
			COUNT
		};
 
		class JKenelType
		{
		public:
			template<J_KERNEL_SIZE type>
			static constexpr bool IsEven()noexcept
			{
				return false; 
			}
			template<J_KERNEL_SIZE type>
			static constexpr uint Size()noexcept
			{
				if constexpr (type == J_KERNEL_SIZE::_3x3)
					return 3;
				else if constexpr (type == J_KERNEL_SIZE::_5x5)
					return 5;
				else if constexpr (type == J_KERNEL_SIZE::_7x7)
					return 7;
				else
					return 0;
			}
			static constexpr uint MaxSize()noexcept
			{
				return Size<(J_KERNEL_SIZE)((uint)J_KERNEL_SIZE::COUNT - 1)>();
			}
		}; 
	}
}