#pragma once
#include"../Device/JGraphicDeviceUser.h"
#include"../../Core/Pointer/JOwnerPtr.h"
#include"../../Object/Resource/Shader/JShaderDataHolderBase.h"

namespace JinEngine
{
	class JShader;
	class JGraphicShaderDataHolderBase;
	class JComputeShaderDataHolderBase;
	namespace Graphic
	{
		class JGraphicDevice;
		class JGraphicResourceManager;
		struct JGraphicInfo;
		 
		//���� scene drawing�� ��� JSceneDraw�� �Ļ�Ŭ�������� ��������.
		//�׷��Ƿ� ��ǻ� JGraphicShaderDataHandler�� JSceneDraw�� ��ӹ޾Ƽ� ����Ѵ�.
		//���Ŀ� �ٸ� ������ GraphicShader�� ����ؼ� Scene�� Draw�� ��찡 ����� JGraphicImpl����
		//Shader���� Event(compile, recompile, change pso)���� ü�������� �����ؾ��Ұ��̴�.
		//������ JSceneDraw�� ���� �Լ��� ȣ���� �����ϴ� ����� ����Ѵ�.
		class JGraphicShaderDataHandler
		{
		public:
			virtual void NotifyChangeGraphicShaderMacro(JGraphicDevice* device, const JGraphicInfo& info) = 0;
			virtual JOwnerPtr<JGraphicShaderDataHolderBase> CreateGraphicShader(JGraphicDevice* device, JGraphicResourceManager* gResourceM, const JGraphicShaderInitData& initDara) = 0;
		public:
			void RecompileShader(JShader* shader); 
		};

		// ComputeDrawClass���� �ٷ�� Compute Shader������ ���� Recompile���ǵ� �ٸ��Ƿ�
		// NotifyChangeGraphicShaderMacro�� �ϰ������� �˷��� ���ǿ� ���� ó���ϰų�
		// Event�߻��� ���� �ʿ�� �ϴ� ���� �˷��� ó���Ҽ��ִ�.
		// ����� ���� �ʿ�� �ϴ� ���� �˷��� ó���ϳ�
		// ���Ŀ� ���� �����ϴ� ����� ���� �ʿ伺�� ������ִ�.
		class JComputeShaderDataHandler
		{
		public: 
			virtual JOwnerPtr<JComputeShaderDataHolderBase> CreateComputeShader(JGraphicDevice* device, JGraphicResourceManager* gResourceM, const JComputeShaderInitData& initDara) = 0;
		public:
			void RecompileShader(JShader* shader); 
		};
	}
}