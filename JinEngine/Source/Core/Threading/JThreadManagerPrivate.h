#pragma once
#include"../Func/Functor/JFunctor.h"
#include<memory>
namespace JinEngine
{
	namespace Graphic
	{
		class JGraphic;
	}
	namespace Application
	{
		class JApplication;;
	}
	namespace Core
	{
		struct JThreadInitInfo;
		class JThreadManagerPrivate
		{
		public:
			class ApplicationInterface
			{
			private:
				friend class Application::JApplication;
			private:
				static void Initialize();
				static void Clear();
			private:
				static void Update(); 
			};
			class GraphicInterface
			{
			private:
				friend class Graphic::JGraphic;
			private:
				static size_t CreateUpdateThread(const JThreadInitInfo& initInfo, std::unique_ptr<JBindHandleBase>&& bind);
				static size_t CreateDrawThread(const JThreadInitInfo& initInfo, std::unique_ptr<JBindHandleBase>&& bind);
			};
		};
	}
}