#pragma once 
#include<vector>
#include<functional>
 
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
 