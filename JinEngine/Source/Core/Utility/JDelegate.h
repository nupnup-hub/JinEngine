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
#include"../JCoreEssential.h"
#include<functional>
 
namespace JinEngine
{
	namespace Core
	{
		template <typename T>
		class JDelegate;

		template<typename RET, typename ...PARAMS>
		class JDelegate<RET(PARAMS...)>
		{
		public:
			template<typename T>
			void connect(T* t, RET(T::* method)(PARAMS...))
			{
				functionVec.push_back([=](PARAMS ... as) { (t->*method)(as...); });
			}
			void connect(std::function<RET(PARAMS...)> func)
			{
				functionVec.push_back(func);
			}
			void Invoke(PARAMS... args)
			{
				for (auto& data : functionVec)
					data(args...);
			}
			void DisConnect(int index)
			{
				functionVec.erase(functionVec.begin() + index);
			}
			bool IsEmpty()
			{
				return true ? false : functionVec.size() == 0;
			}
		protected:
			std::vector<std::function<RET(PARAMS...)>>functionVec;
		};

	}
}