#pragma once

namespace JinEngine
{
	namespace Graphic
	{
		enum class J_GRAPHIC_DEVICE_TYPE
		{
			DX11,	//�̱���
			DX12,	//�ַ� ����
			OPEN_GL, //�̱���
			COUNT
		};  

		class JGraphicDeviceType
		{
		public:
			static bool IsDxType(const J_GRAPHIC_DEVICE_TYPE type)
			{
				switch (type)
				{
				case JinEngine::Graphic::J_GRAPHIC_DEVICE_TYPE::DX11:
					return true;
				case JinEngine::Graphic::J_GRAPHIC_DEVICE_TYPE::DX12:
					return true;
				case JinEngine::Graphic::J_GRAPHIC_DEVICE_TYPE::OPEN_GL:
					return false;
				default:
					break;
				}
				return false;
			}
		};
	}
}