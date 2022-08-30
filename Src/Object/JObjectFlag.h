#pragma once

namespace JinEngine
{
	enum J_OBJECT_FLAG
	{
		OBJECT_FLAG_NONE = 0,
		OBJECT_FLAG_AUTO_GENERATED = 1 << 0,
		OBJECT_FLAG_HIDDEN = 1 << 1,
		OBJECT_FLAG_UNEDITABLE = 1 << 2,
		//������Ʈ �ı����� �÷���
		//ex) root -> ch00 -> ch01 ���迡 Ʈ���� �����ҽ�
		//root(UNDESTROYABLE) -> ch00(UNDESTROYABLE).... �� ����������
		//root(UNDESTROYABLE) -> ch00 -> ch01(UNDESTROYABLE)�� �Ұ���
		//�߰��� UNDESTROYABLE�÷��׸� ������������ ������Ʈ�� ������ ch00�� �θ� ����ä�� ������ root���ƴϱ⿡ ������ �߻�.
		OBJECT_FLAG_UNDESTROYABLE = 1 << 3,
		OBJECT_FLAG_DO_NOT_SAVE = 1 << 4,
		OBJECT_FLAG_UNCOPYABLE = 1 << 5,
		OBJECT_FLAG_CAPTURE_OBJECT = 1 << 6,


		OBJECT_FLAG_EDITOR_OBJECT = OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_DO_NOT_SAVE | OBJECT_FLAG_HIDDEN
	};
}