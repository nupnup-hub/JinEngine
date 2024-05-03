#include"JImageProcessingFilter.h"
#include<cmath>

namespace JinEngine::Graphic
{
	static constexpr float PI = 3.1415926535f;
	float JFilter::Box1D(const int x, const int size)
	{
		return 1.0f / size;
	}
	float JFilter::Box2D(const int x, const int y, const int size)
	{
		return 1.0f / size;
	}
	float JFilter::Gaus1D(const int x, const float sig)
	{
		return std::exp(-(x * x) / (2.0f * sig * sig)) / sqrt((2.0f * PI * sig * sig));
	}
	float JFilter::Gaus2D(const int x, const int y, const float sig)
	{
		return std::exp(-(x * x + y * y) / (2.0f * sig * sig)) / (2.0f * PI * sig * sig);
	} 
	float JFilter::Kaiser1D(const int x, const float beta, const int N)
	{
		float top = std::cyl_bessel_i(0, sqrt(1.0f - powf((2.0f * x) / (N - 1), 2)) * beta);
		float bottom = std::cyl_bessel_i(0, beta);
		return top / bottom;
	}
	/**
	* @param beta controll sharpness
	* @param N =  kenel(row or column) count - 1
	* @brief 2-D window function => w(n1, n2) = w1(n1)w2(n2)
	*/
	float JFilter::Kaiser2D(const int x, const int y, const float beta, const int N)
	{
		//unuse
		//https://en.wikipedia.org/wiki/Kaiser_window, https://en.wikipedia.org/wiki/Two-dimensional_window_design#Kaiser_window  
		float topX = std::cyl_bessel_i(0, sqrt(1.0f - powf((2.0f * x) / N, 2)) * beta);
		float topY = std::cyl_bessel_i(0, sqrt(1.0f - powf((2.0f * y) / N, 2)) * beta);
		//float topX = std::cyl_bessel_i(0, beta * sqrt(1.0f - pow((x - (N / 2.0f)) / (N / 2.0f), 2)));
		//float topY = std::cyl_bessel_i(0, beta * sqrt(1.0f - pow((y - (N / 2.0f)) / (N / 2.0f), 2)));
		float bottom = std::cyl_bessel_i(0, beta);
		return (topX * topY) / (bottom * bottom);
	}
}