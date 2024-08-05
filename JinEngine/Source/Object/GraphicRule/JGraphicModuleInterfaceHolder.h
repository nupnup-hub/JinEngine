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
#include"JGraphicModuleInterface.h"
#include"../../Core/Singleton/JSingletonHolder.h"

namespace JinEngine
{
	namespace Rule
	{
		class JGraphicModuleInterfaceHolder
		{
		private:
			std::unique_ptr<JGraphicModuleInterface> moduleInterface;
		public:
			JGraphicModuleInterface* Get();
		public:
			void Set(std::unique_ptr<JGraphicModuleInterface>&& newModuleInterface);
		};

	}
	using JGraphicModuleInterfaceHolder = Core::JSingletonHolder<Rule::JGraphicModuleInterfaceHolder>;
	static Rule::JGraphicModuleInterface* GraphicModuleInterface()
	{
		return JGraphicModuleInterfaceHolder::Instance().Get();
	}
	static Rule::JGraphicModuleInterface* GMI()
	{
		return JGraphicModuleInterfaceHolder::Instance().Get();
	} 
}