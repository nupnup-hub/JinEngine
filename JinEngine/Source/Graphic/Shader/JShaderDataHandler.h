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
#include"JShaderDataHolder.h" 
#include"../Device/JGraphicDeviceUser.h" 
#include"../../Core/Pointer/JOwnerPtr.h" 

namespace JinEngine
{
	class JShader; 
	/**
	* User가 JObject를 통해서 조절가능한 Shader생성하는 책임을 가지는 class는
	* Graphic, Compute type에 따라 밑에 class를 반드시 상속한다. 
	*/
	namespace Graphic
	{ 
		struct JGraphicShaderCompileSet;
		//private shader
		class JGraphicDeviceShaderHandler
		{
		public:
			virtual ~JGraphicDeviceShaderHandler() = default;
		public:
			virtual void RecompileShader(const JGraphicShaderCompileSet& dataSet) = 0;
		};

		//public shader(user)
		//현재 scene drawing은 모두 JSceneDraw에 파생클래스에서 행해진다.
		//그러므로 사실상 JGraphicShaderDataHandler는 JSceneDraw만 상속받아서 사용한다.
		//추후에 다른 종류에 GraphicShader를 사용해서 Scene을 Draw할 경우가 생기면 JGraphicImpl에서
		//Shader관련 Event(compile, recompile, change pso)등을 체계적으로 관리해야할것이다.
		//지금은 JSceneDraw로 직접 함수를 호출해 접근하는 방식을 사용한다.
		class JGraphicShaderDataHandler : public JGraphicDeviceShaderHandler
		{
		public: 
			virtual JOwnerPtr<JShaderDataHolder> CreateShader(const JGraphicShaderCompileSet& dataSet, JGraphicShaderInitData initData) = 0;
		protected:
			void RecompileUserShader(JShader* shader);
		};
 
		// ComputeDrawClass마다 다루는 Compute Shader갯수와 종류 Recompile조건도 다르므로
		// NotifyChangeGraphicShaderMacro로 일괄적으로 알려서 조건에 따라 처리하거나
		// Event발생시 직접 필요로 하는 곳에 알려서 처리할수있다.
		// 현재는 직접 필요로 하는 곳에 알려서 처리하나
		// 추후에 따로 관리하는 기반을 만들 필요성이 생길수있다. 
		class JComputeShaderDataHandler : public JGraphicDeviceShaderHandler
		{
		public:
			virtual JOwnerPtr<JShaderDataHolder> CreateShader(const JGraphicShaderCompileSet& dataSet, JComputeShaderInitData initData) = 0;
		protected:
			void RecompileUserShader(JShader* shader);
		};
	}
}