#pragma once
#include"JOcclusionConstants.h" 
#include"../../Core/Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Graphic
	{ 
		/**
		* Caution!
		* Constants set�� 256 byte������ Gpu�� upload�Ǹ�
		* �̴� Cpu Constants structure size�� �ٸ��������Ƿ�
		* ������ data�� CopyData�ҽÿ��� Gpu�� upload�Ǵ� ũ��� ����
		* structure�� �Ҵ��ϰų�(����), �ϳ��� CopyData�� �ؾ��Ѵ�(����� �ʹ�����)
		*/
		template<uint setCount>
		struct JFrameConstantsSet
		{ 
		public:
			uint frameIndex[setCount];
		public:
			bool isUpdated[setCount];
			bool updateStart = false;
		public:
			virtual ~JFrameConstantsSet() = default;
		public:
			virtual void Begin()
			{
				JCUtil::InsertValue(isUpdated, false, std::make_index_sequence<setCount>());
				updateStart = false;
			}
		};
	}
}