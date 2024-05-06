/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#pragma once
#include"../Core/Reflection/JReflection.h"

namespace JinEngine
{ 
	/**
	* OBJECT_FLAG_UNDESTROYABLE
	* ex) root -> ch00 -> ch01 ���迡 Ʈ���� �����ҽ�
	* root(UNDESTROYABLE) -> ch00(UNDESTROYABLE).... �� ����������
	* root(UNDESTROYABLE) -> ch00 -> ch01(UNDESTROYABLE)�� �Ұ���
	* �߰��� UNDESTROYABLE�÷��׸� ������������ ������Ʈ�� ������ ch00�� �θ� ����ä�� ������ root���ƴϱ⿡ ������ �߻�.
------------------------------fixed------------------------------
	* Forced Destroy�� �߰��ϸ鼭 ���� ��ü�� UNDESTROYABLE�� ���������ʰԵǾ���.
	*/

	/**
	* OBJECT_FLAG_HIDDEN : gui�� object�� ǥ�õ����ʴ´�.
	* OBJECT_FLAG_UNDESTROYABLE : �������̿ܿ� �ı��Ҽ�����.
	* OBJECT_FLAG_UNEDITABLE : editor�� ���� ������ �Ұ����ϴ�. 
	*/
	REGISTER_ENUM(J_OBJECT_FLAG, int, OBJECT_FLAG_NONE = 0,
		OBJECT_FLAG_AUTO_GENERATED = 1 << 0,
		OBJECT_FLAG_HIDDEN = 1 << 1,
		OBJECT_FLAG_UNEDITABLE = 1 << 2,
		OBJECT_FLAG_UNDESTROYABLE = 1 << 3,
		OBJECT_FLAG_DO_NOT_SAVE = 1 << 4,
		OBJECT_FLAG_UNCOPYABLE = 1 << 5,
		OBJECT_FLAG_ONLY_USED_IN_EDITOR = 1 << 6, 
		OBJECT_FLAG_RESTRICT_CONTROL_IDENTIFICABLE = 1 << 7,
		OBJECT_FLAG_EDITOR_OBJECT = OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_DO_NOT_SAVE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_ONLY_USED_IN_EDITOR | OBJECT_FLAG_RESTRICT_CONTROL_IDENTIFICABLE,
		OBJECT_FLAG_UNIQUE_EDITOR_OBJECT = OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_DO_NOT_SAVE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_UNCOPYABLE | OBJECT_FLAG_ONLY_USED_IN_EDITOR | OBJECT_FLAG_RESTRICT_CONTROL_IDENTIFICABLE)
}