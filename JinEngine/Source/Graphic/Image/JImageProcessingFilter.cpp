#include"JImageProcessingFilter.h"
#include<cmath>

namespace JinEngine::Graphic
{
	float JFilter::Box(const int x, const int y, const int size)
	{
		return 1.0f / size;
	}
	float JFilter::Gaus(const int x, const int y, const float sig)
	{
		static constexpr float PI = 3.1415926535f;
		return std::exp(-(x * x + y * y) / (2.0f * sig * sig)) / (2.0f * PI * sig * sig);
	}
	/**
	* @param beta controll sharpness
	* @param N =  kenel(row or column) count - 1
	* @brief 2-D window function => w(n1, n2) = w1(n1)w2(n2)
	*/
	float JFilter::Kaiser(const int x, const int y, const float beta, const int N)
	{
		//https://en.wikipedia.org/wiki/Kaiser_window, https://en.wikipedia.org/wiki/Two-dimensional_window_design#Kaiser_window 
		float topX = std::cyl_bessel_i(0, beta * sqrt(1.0f - pow((x - (N / 2.0f)) / (N / 2.0f), 2)));
		float topY = std::cyl_bessel_i(0, beta * sqrt(1.0f - pow((y - (N / 2.0f)) / (N / 2.0f), 2)));
		float bottom = std::cyl_bessel_i(0, beta);
		return (topX * topY) / (bottom * bottom);
	}
}