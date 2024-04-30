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