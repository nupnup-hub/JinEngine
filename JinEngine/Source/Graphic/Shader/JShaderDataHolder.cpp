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


#include"JShaderDataHolder.h" 

namespace JinEngine::Graphic
{
	namespace Private
	{
		//shader setting을 일반화 해서 관리하는데는 어려움이 많다
		//각 task별로 사용하는 기능이 정해져있고 관리하는 parameter들이 상이하므로
		//graphic resource처럼 manager가 모든걸 관리하지 않고 data가 담긴 holder를 각 task들이 소유하며
		//생성 파괴를 스스로 관리한다.
		//추후 재설계를 하기 전까지는 여기서 필요한 공통data를 관리한다. 
	}

	JShaderDataHolder::JShaderDataHolder()
	{ 
	}
	JShaderDataHolder::~JShaderDataHolder()
	{ 
	} 
}