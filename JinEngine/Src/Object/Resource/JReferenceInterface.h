#pragma once

namespace JinEngine
{
	class JResourceObjectUserInterface;

	//Resource�� �����ϰ� �ִ� Scene ������ �ִ� Activated�� Object�� ī��Ʈ
	//���� ��Ȱ��ȭ�� Object�Ͻ�(User custom.... ex) ī�޶�� �Ÿ��� �� ���, ������ Object�� ���) 
	//Resource�ֿ� �����͸� �޸𸮿��� ���������� ���
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