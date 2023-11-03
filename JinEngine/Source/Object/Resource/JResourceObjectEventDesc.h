#pragma once
#include"../../Core/JCoreEssential.h"

namespace JinEngine
{
	struct JResourceEventDesc 
	{
	public:
		virtual ~JResourceEventDesc() = default;
	};
	struct JResourceUpdateEvDesc : public JResourceEventDesc
	{
	public:
		enum class USER_ACTION
		{
			PASS,
			UPDATE_USER_ONLY,
			UPDATE_USER_AND_REAR_OF_FRAME_BUFFER
		};
	public:
		const USER_ACTION action = USER_ACTION::PASS;
	public:
		JResourceUpdateEvDesc(const USER_ACTION action);
	};
}