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