#pragma once

namespace JinEngine
{
	class JResourceObjectUserInterface;

	//Resource를 참조하고 있는 Scene 영역에 있는 Activated된 Object를 카운트
	//전부 비활성화된 Object일시(User custom.... ex) 카메라와 거리가 먼 경우, 숨겨진 Object일 경우) 
	//Resource주요 데이터를 메모리에서 내리기위해 사용
	class JReferenceInterface
	{
	private:
		friend class JResourceObjectUserInterface;
	private:
		int referenceCount = 0;
	protected:
		virtual ~JReferenceInterface() = default;
	protected:
		int GetReferenceCount()const noexcept;
		void OnReference()noexcept;
		void OffReference()noexcept;
	};
}