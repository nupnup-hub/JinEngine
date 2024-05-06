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