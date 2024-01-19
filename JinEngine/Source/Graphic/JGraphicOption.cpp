#include"JGraphicOption.h"
namespace JinEngine
{
	namespace Graphic
	{
		uint JGraphicOption::GetClusterTotalCount()const noexcept
		{
			return GetClusterXCount() * GetClusterYCount() * GetClusterZCount();
		}
		uint JGraphicOption::GetClusterIndexCount()const noexcept
		{
			return GetClusterTotalCount() * GetLightPerCluster();
		}
		uint JGraphicOption::GetClusterXCount()const noexcept
		{
			return Constants::litClusterXRange[clusterXIndex];
		}
		uint JGraphicOption::GetClusterYCount()const noexcept
		{
			return Constants::litClusterYRange[clusterYIndex];
		}
		uint JGraphicOption::GetClusterZCount()const noexcept
		{
			return Constants::litClusterZRange[clusterZIndex];
		}
		uint JGraphicOption::GetLightPerCluster()const noexcept
		{
			return Constants::maxLightPerClusterRange[lightPerClusterIndex];
		}
		bool JGraphicOption::IsOcclusionActivated()const noexcept
		{
			return isOcclusionQueryActivated;
		} 
		bool JGraphicOption::CanUseSSAO()const noexcept
		{ 
			return allowDeferred && useSsao;
		} 
	}
}