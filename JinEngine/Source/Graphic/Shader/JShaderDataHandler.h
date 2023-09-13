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
		 
		//현재 scene drawing은 모두 JSceneDraw에 파생클래스에서 행해진다.
		//그러므로 사실상 JGraphicShaderDataHandler는 JSceneDraw만 상속받아서 사용한다.
		//추후에 다른 종류에 GraphicShader를 사용해서 Scene을 Draw할 경우가 생기면 JGraphicImpl에서
		//Shader관련 Event(compile, recompile, change pso)등을 체계적으로 관리해야할것이다.
		//지금은 JSceneDraw로 직접 함수를 호출해 접근하는 방식을 사용한다.
		class JGraphicShaderDataHandler
		{
		public:
			virtual void NotifyChangeGraphicShaderMacro(JGraphicDevice* device, const JGraphicInfo& info) = 0;
			virtual JOwnerPtr<JGraphicShaderDataHolderBase> CreateGraphicShader(JGraphicDevice* device, JGraphicResourceManager* gResourceM, const JGraphicShaderInitData& initDara) = 0;
		public:
			void RecompileShader(JShader* shader); 
		};

		// ComputeDrawClass마다 다루는 Compute Shader갯수와 종류 Recompile조건도 다르므로
		// NotifyChangeGraphicShaderMacro로 일괄적으로 알려서 조건에 따라 처리하거나
		// Event발생시 직접 필요로 하는 곳에 알려서 처리할수있다.
		// 현재는 직접 필요로 하는 곳에 알려서 처리하나
		// 추후에 따로 관리하는 기반을 만들 필요성이 생길수있다.
		class JComputeShaderDataHandler
		{
		public: 
			virtual JOwnerPtr<JComputeShaderDataHolderBase> CreateComputeShader(JGraphicDevice* device, JGraphicResourceManager* gResourceM, const JComputeShaderInitData& initDara) = 0;
		public:
			void RecompileShader(JShader* shader); 
		};
	}
}