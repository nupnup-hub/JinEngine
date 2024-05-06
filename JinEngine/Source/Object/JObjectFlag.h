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
	* ex) root -> ch00 -> ch01 관계에 트리를 구성할시
	* root(UNDESTROYABLE) -> ch00(UNDESTROYABLE).... 은 가능하지만
	* root(UNDESTROYABLE) -> ch00 -> ch01(UNDESTROYABLE)은 불가능
	* 중간에 UNDESTROYABLE플래그를 소유하지않은 오브젝트가 있을시 ch00은 부모가 없는채로 남지만 root가아니기에 에러가 발생.
------------------------------fixed------------------------------
	* Forced Destroy를 추가하면서 하위 객체에 UNDESTROYABLE은 문제되지않게되었다.
	*/

	/**
	* OBJECT_FLAG_HIDDEN : gui에 object가 표시되지않는다.
	* OBJECT_FLAG_UNDESTROYABLE : 관리자이외에 파괴할수없다.
	* OBJECT_FLAG_UNEDITABLE : editor를 통한 편집이 불가능하다. 
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