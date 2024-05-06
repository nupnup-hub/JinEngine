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
	* User�� JObject�� ���ؼ� ���������� Shader�����ϴ� å���� ������ class��
	* Graphic, Compute type�� ���� �ؿ� class�� �ݵ�� ����Ѵ�. 
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
		//���� scene drawing�� ��� JSceneDraw�� �Ļ�Ŭ�������� ��������.
		//�׷��Ƿ� ��ǻ� JGraphicShaderDataHandler�� JSceneDraw�� ��ӹ޾Ƽ� ����Ѵ�.
		//���Ŀ� �ٸ� ������ GraphicShader�� ����ؼ� Scene�� Draw�� ��찡 ����� JGraphicImpl����
		//Shader���� Event(compile, recompile, change pso)���� ü�������� �����ؾ��Ұ��̴�.
		//������ JSceneDraw�� ���� �Լ��� ȣ���� �����ϴ� ����� ����Ѵ�.
		class JGraphicShaderDataHandler : public JGraphicDeviceShaderHandler
		{
		public: 
			virtual JOwnerPtr<JShaderDataHolder> CreateShader(const JGraphicShaderCompileSet& dataSet, JGraphicShaderInitData initData) = 0;
		protected:
			void RecompileUserShader(JShader* shader);
		};
 
		// ComputeDrawClass���� �ٷ�� Compute Shader������ ���� Recompile���ǵ� �ٸ��Ƿ�
		// NotifyChangeGraphicShaderMacro�� �ϰ������� �˷��� ���ǿ� ���� ó���ϰų�
		// Event�߻��� ���� �ʿ�� �ϴ� ���� �˷��� ó���Ҽ��ִ�.
		// ����� ���� �ʿ�� �ϴ� ���� �˷��� ó���ϳ�
		// ���Ŀ� ���� �����ϴ� ����� ���� �ʿ伺�� ������ִ�. 
		class JComputeShaderDataHandler : public JGraphicDeviceShaderHandler
		{
		public:
			virtual JOwnerPtr<JShaderDataHolder> CreateShader(const JGraphicShaderCompileSet& dataSet, JComputeShaderInitData initData) = 0;
		protected:
			void RecompileUserShader(JShader* shader);
		};
	}
}