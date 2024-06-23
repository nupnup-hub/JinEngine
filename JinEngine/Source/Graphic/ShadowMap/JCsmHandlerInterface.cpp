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


#include"JCsmHandlerInterface.h"
#include"JCsmTargetInfo.h"
#include"../JGraphicPrivate.h"
#include"../../Core/Guid/JGuidCreator.h"
#include"../../Core/Utility/JCommonUtility.h" 
 
using namespace DirectX;
namespace JinEngine::Graphic
{
	struct Triangle
	{
		XMVECTOR pt[3];
		bool culled;
	};

	namespace
	{
		/**
		* reference by directx-sdk-samples-main\CascadedShadowMaps11
		*/

		static constexpr bool canMoveLightTexelSize = true;
		static constexpr bool isFitToScene = true;
		static float CalFixedSplit(const float camNear, const float camFar, const uint splitCount, const uint n)noexcept
		{
			return camNear + (camFar - camNear) * (n / float(splitCount));
		}
		static float CalLogSplit(const float camNear, const float camFar, const uint splitCount, const uint n)noexcept
		{
			return camNear * pow(camFar / camNear, n / float(splitCount));
		}
		//n is 1 ~ splitCount
		static float CalSplitLength(const float blendRate,
			const float camNear,
			const float camFar,
			const uint splitCount,
			const uint n)noexcept
		{
			return blendRate * CalLogSplit(camNear, camFar, splitCount, n) +
				(1 - blendRate) * CalFixedSplit(camNear, camFar, splitCount, n);
		}
		static void ComputeFrustumPoint(const BoundingFrustum& camFrustum, XMVECTOR(&wCorner)[8])
		{
			static const XMVECTORU32 vGrabX = { 0xFFFFFFFF,0x00000000,0x00000000,0x00000000 };
			static const XMVECTORU32 vGrabY = { 0x00000000,0xFFFFFFFF,0x00000000,0x00000000 };

			XMVECTORF32 vRightTop = { camFrustum.RightSlope,camFrustum.TopSlope,1.0f,1.0f };
			XMVECTORF32 vLeftBottom = { camFrustum.LeftSlope,camFrustum.BottomSlope,1.0f,1.0f };
			XMVECTORF32 vNear = { camFrustum.Near,camFrustum.Near,camFrustum.Near,1.0f };
			XMVECTORF32 vFar = { camFrustum.Far,camFrustum.Far,camFrustum.Far,1.0f };
			XMVECTOR vRightTopNear = XMVectorMultiply(vRightTop, vNear);
			XMVECTOR vRightTopFar = XMVectorMultiply(vRightTop, vFar);
			XMVECTOR vLeftBottomNear = XMVectorMultiply(vLeftBottom, vNear);
			XMVECTOR vLeftBottomFar = XMVectorMultiply(vLeftBottom, vFar);

			wCorner[0] = vRightTopNear;
			wCorner[1] = XMVectorSelect(vRightTopNear, vLeftBottomNear, vGrabX);
			wCorner[2] = vLeftBottomNear;
			wCorner[3] = XMVectorSelect(vRightTopNear, vLeftBottomNear, vGrabY);

			wCorner[4] = vRightTopFar;
			wCorner[5] = XMVectorSelect(vRightTopFar, vLeftBottomFar, vGrabX);
			wCorner[6] = vLeftBottomFar;
			wCorner[7] = XMVectorSelect(vRightTopFar, vLeftBottomFar, vGrabY);
		}
		static void ComputeNearFar(const XMVECTOR(&sceneBBoxCornerLV)[8],
			const XMVECTOR lightCamOrthoMin,
			const XMVECTOR lightCamOrthoMax,
			_Out_ float& outFrustumNear,
			_Out_ float& outFrustumFar)noexcept
		{
			//compute use sceneBBox and lightBBox(min, max V)

			// Initialize the near and far planes
			//outFrustumNear = JVectorBase::PositiveInf<float>();
			//outFrustumFar = JVectorBase::NegativeInf<float>();

			outFrustumNear = JVectorBase::PositiveInf<float>();
			outFrustumFar = JVectorBase::NegativeInf<float>();

			Triangle triangleList[16];
			INT iTriangleCnt = 1;

			triangleList[0].pt[0] = sceneBBoxCornerLV[0];
			triangleList[0].pt[1] = sceneBBoxCornerLV[1];
			triangleList[0].pt[2] = sceneBBoxCornerLV[2];
			triangleList[0].culled = false;

			// These are the indices used to tesselate an AABB into a list of triangles.
			static const INT iAABBTriIndexes[] =
			{
				0,1,2,  1,2,3,
				4,5,6,  5,6,7,
				0,2,4,  2,4,6,
				1,3,5,  3,5,7,
				0,1,4,  1,4,5,
				2,3,6,  3,6,7
			};

			INT iPointPassesCollision[3];

			// At a high level: 
			// 1. Iterate over all 12 triangles of the AABB.  
			// 2. Clip the triangles against each plane. Create new triangles as needed.
			// 3. Find the min and max z values as the near and far plane.

			//This is easier because the triangles are in camera spacing making the collisions tests simple comparisions.

			float fLightCameraOrthographicMinX = XMVectorGetX(lightCamOrthoMin);
			float fLightCameraOrthographicMaxX = XMVectorGetX(lightCamOrthoMax);
			float fLightCameraOrthographicMinY = XMVectorGetY(lightCamOrthoMin);
			float fLightCameraOrthographicMaxY = XMVectorGetY(lightCamOrthoMax);

			for (INT AABBTriIter = 0; AABBTriIter < 12; ++AABBTriIter)
			{
				triangleList[0].pt[0] = sceneBBoxCornerLV[iAABBTriIndexes[AABBTriIter * 3 + 0]];
				triangleList[0].pt[1] = sceneBBoxCornerLV[iAABBTriIndexes[AABBTriIter * 3 + 1]];
				triangleList[0].pt[2] = sceneBBoxCornerLV[iAABBTriIndexes[AABBTriIter * 3 + 2]];
				iTriangleCnt = 1;
				triangleList[0].culled = FALSE;

				// Clip each invidual triangle against the 4 frustums.  When ever a triangle is clipped into new triangles, 
				//add them to the list.
				for (INT frustumPlaneIter = 0; frustumPlaneIter < 4; ++frustumPlaneIter)
				{
					FLOAT fEdge;
					INT iComponent;

					if (frustumPlaneIter == 0)
					{
						fEdge = fLightCameraOrthographicMinX; // todo make float temp
						iComponent = 0;
					}
					else if (frustumPlaneIter == 1)
					{
						fEdge = fLightCameraOrthographicMaxX;
						iComponent = 0;
					}
					else if (frustumPlaneIter == 2)
					{
						fEdge = fLightCameraOrthographicMinY;
						iComponent = 1;
					}
					else
					{
						fEdge = fLightCameraOrthographicMaxY;
						iComponent = 1;
					}

					for (INT triIter = 0; triIter < iTriangleCnt; ++triIter)
					{
						// We don't delete triangles, so we skip those that have been culled.
						if (!triangleList[triIter].culled)
						{
							INT iInsideVertCount = 0;
							XMVECTOR tempOrder;
							// Test against the correct frustum plane.
							// This could be written more compactly, but it would be harder to understand.

							if (frustumPlaneIter == 0)
							{
								for (INT triPtIter = 0; triPtIter < 3; ++triPtIter)
								{
									if (XMVectorGetX(triangleList[triIter].pt[triPtIter]) >
										XMVectorGetX(lightCamOrthoMin))
									{
										iPointPassesCollision[triPtIter] = 1;
									}
									else
									{
										iPointPassesCollision[triPtIter] = 0;
									}
									iInsideVertCount += iPointPassesCollision[triPtIter];
								}
							}
							else if (frustumPlaneIter == 1)
							{
								for (INT triPtIter = 0; triPtIter < 3; ++triPtIter)
								{
									if (XMVectorGetX(triangleList[triIter].pt[triPtIter]) <
										XMVectorGetX(lightCamOrthoMax))
									{
										iPointPassesCollision[triPtIter] = 1;
									}
									else
									{
										iPointPassesCollision[triPtIter] = 0;
									}
									iInsideVertCount += iPointPassesCollision[triPtIter];
								}
							}
							else if (frustumPlaneIter == 2)
							{
								for (INT triPtIter = 0; triPtIter < 3; ++triPtIter)
								{
									if (XMVectorGetY(triangleList[triIter].pt[triPtIter]) >
										XMVectorGetY(lightCamOrthoMin))
									{
										iPointPassesCollision[triPtIter] = 1;
									}
									else
									{
										iPointPassesCollision[triPtIter] = 0;
									}
									iInsideVertCount += iPointPassesCollision[triPtIter];
								}
							}
							else
							{
								for (INT triPtIter = 0; triPtIter < 3; ++triPtIter)
								{
									if (XMVectorGetY(triangleList[triIter].pt[triPtIter]) <
										XMVectorGetY(lightCamOrthoMax))
									{
										iPointPassesCollision[triPtIter] = 1;
									}
									else
									{
										iPointPassesCollision[triPtIter] = 0;
									}
									iInsideVertCount += iPointPassesCollision[triPtIter];
								}
							}

							// Move the points that pass the frustum test to the begining of the array.
							if (iPointPassesCollision[1] && !iPointPassesCollision[0])
							{
								tempOrder = triangleList[triIter].pt[0];
								triangleList[triIter].pt[0] = triangleList[triIter].pt[1];
								triangleList[triIter].pt[1] = tempOrder;
								iPointPassesCollision[0] = TRUE;
								iPointPassesCollision[1] = FALSE;
							}
							if (iPointPassesCollision[2] && !iPointPassesCollision[1])
							{
								tempOrder = triangleList[triIter].pt[1];
								triangleList[triIter].pt[1] = triangleList[triIter].pt[2];
								triangleList[triIter].pt[2] = tempOrder;
								iPointPassesCollision[1] = TRUE;
								iPointPassesCollision[2] = FALSE;
							}
							if (iPointPassesCollision[1] && !iPointPassesCollision[0])
							{
								tempOrder = triangleList[triIter].pt[0];
								triangleList[triIter].pt[0] = triangleList[triIter].pt[1];
								triangleList[triIter].pt[1] = tempOrder;
								iPointPassesCollision[0] = TRUE;
								iPointPassesCollision[1] = FALSE;
							}

							if (iInsideVertCount == 0)
							{ // All points failed. We're done,  
								triangleList[triIter].culled = true;
							}
							else if (iInsideVertCount == 1)
							{// One point passed. Clip the triangle against the Frustum plane
								triangleList[triIter].culled = false;

								// 
								XMVECTOR vVert0ToVert1 = triangleList[triIter].pt[1] - triangleList[triIter].pt[0];
								XMVECTOR vVert0ToVert2 = triangleList[triIter].pt[2] - triangleList[triIter].pt[0];

								// Find the collision ratio.
								FLOAT fHitPointTimeRatio = fEdge - XMVectorGetByIndex(triangleList[triIter].pt[0], iComponent);
								// Calculate the distance along the vector as ratio of the hit ratio to the component.
								FLOAT fDistanceAlongVector01 = fHitPointTimeRatio / XMVectorGetByIndex(vVert0ToVert1, iComponent);
								FLOAT fDistanceAlongVector02 = fHitPointTimeRatio / XMVectorGetByIndex(vVert0ToVert2, iComponent);
								// Add the point plus a percentage of the vector.
								vVert0ToVert1 *= fDistanceAlongVector01;
								vVert0ToVert1 += triangleList[triIter].pt[0];
								vVert0ToVert2 *= fDistanceAlongVector02;
								vVert0ToVert2 += triangleList[triIter].pt[0];

								triangleList[triIter].pt[1] = vVert0ToVert2;
								triangleList[triIter].pt[2] = vVert0ToVert1;

							}
							else if (iInsideVertCount == 2)
							{ // 2 in  // tesselate into 2 triangles


								// Copy the triangle\(if it exists) after the current triangle out of
								// the way so we can override it with the new triangle we're inserting.
								triangleList[iTriangleCnt] = triangleList[triIter + 1];

								triangleList[triIter].culled = false;
								triangleList[triIter + 1].culled = false;

								// Get the vector from the outside point into the 2 inside points.
								XMVECTOR vVert2ToVert0 = triangleList[triIter].pt[0] - triangleList[triIter].pt[2];
								XMVECTOR vVert2ToVert1 = triangleList[triIter].pt[1] - triangleList[triIter].pt[2];

								// Get the hit point ratio.
								FLOAT fHitPointTime_2_0 = fEdge - XMVectorGetByIndex(triangleList[triIter].pt[2], iComponent);
								FLOAT fDistanceAlongVector_2_0 = fHitPointTime_2_0 / XMVectorGetByIndex(vVert2ToVert0, iComponent);
								// Calcaulte the new vert by adding the percentage of the vector plus point 2.
								vVert2ToVert0 *= fDistanceAlongVector_2_0;
								vVert2ToVert0 += triangleList[triIter].pt[2];

								// Add a new triangle.
								triangleList[triIter + 1].pt[0] = triangleList[triIter].pt[0];
								triangleList[triIter + 1].pt[1] = triangleList[triIter].pt[1];
								triangleList[triIter + 1].pt[2] = vVert2ToVert0;

								//Get the hit point ratio.
								FLOAT fHitPointTime_2_1 = fEdge - XMVectorGetByIndex(triangleList[triIter].pt[2], iComponent);
								FLOAT fDistanceAlongVector_2_1 = fHitPointTime_2_1 / XMVectorGetByIndex(vVert2ToVert1, iComponent);
								vVert2ToVert1 *= fDistanceAlongVector_2_1;
								vVert2ToVert1 += triangleList[triIter].pt[2];
								triangleList[triIter].pt[0] = triangleList[triIter + 1].pt[1];
								triangleList[triIter].pt[1] = triangleList[triIter + 1].pt[2];
								triangleList[triIter].pt[2] = vVert2ToVert1;
								// Cncrement triangle count and skip the triangle we just inserted.
								++iTriangleCnt;
								++triIter;


							}
							else
							{ // all in
								triangleList[triIter].culled = false;

							}
						}// end if !culled loop            
					}
				}
				for (INT index = 0; index < iTriangleCnt; ++index)
				{
					if (!triangleList[index].culled)
					{
						// Set the near and far plan and the min and max z values respectivly.
						for (int vertind = 0; vertind < 3; ++vertind)
						{
							float fTriangleCoordZ = XMVectorGetZ(triangleList[index].pt[vertind]);
							if (outFrustumNear > fTriangleCoordZ)
							{
								outFrustumNear = fTriangleCoordZ;
							}
							if (outFrustumFar < fTriangleCoordZ)
							{
								outFrustumFar = fTriangleCoordZ;
							}
						}
					}
				}
			}
		}
	}
	bool JCsmHandlerInterface::TargetData::IsValid()const noexcept
	{
		return info != nullptr;
	}
	JCsmOption JCsmHandlerInterface::GetCsmOption()const noexcept
	{
		return option;
	}
	uint JCsmHandlerInterface::GetCsmTargetCount()const noexcept
	{
		return (uint)target.size();
	}
	const JCsmHandlerInterface::ComputeResult& JCsmHandlerInterface::GetCsmComputeResult(const uint index)const noexcept
	{
		return target[index].result;
	}
	JCsmOption& JCsmHandlerInterface::GetCsmOptionRef()const noexcept
	{
		return option;
	}
	void JCsmHandlerInterface::SetCsmOption(const JCsmOption& newOption)noexcept
	{
		option = newOption;
	}
	void JCsmHandlerInterface::CsmUpdate(DirectX::XMMATRIX lightView,
		const DirectX::BoundingBox& sceneBBoxW,
		const size_t mapSize)
	{
		const uint count = GetCsmTargetCount();
		for (uint i = 0; i < count; ++i)
			CalculateShadowMap(lightView, sceneBBoxW, target[i].info->GetFrustum(), mapSize, i);
	}
	void JCsmHandlerInterface::CalculateShadowMap(const DirectX::XMMATRIX lightView,
		const BoundingBox& sceneBBoxW,
		const BoundingFrustum& camFrustumW,
		const size_t mapSize,
		const uint targetIndex)
	{
		if (targetIndex >= target.size())
			return;

		auto& result = target[targetIndex].result;
		result.subFrustumCount = 0;
		 
		//1. calculate split rate
		const uint splitCount = option.GetSplitCount();
		for (uint i = 0; i < splitCount; ++i)
		{
			//cam to far
			const float length = CalSplitLength(option.GetSplitBlendRate(), camFrustumW.Near, camFrustumW.Far, option.GetSplitCount(), i + 1);
			result.splitRate[i] = length / camFrustumW.Far;
			++result.subFrustumCount;
			if (length > option.GetShadowDistance())
				break;
		}

		//2. calculate frustum point(world)	
		XMFLOAT3 sceneBBoxCornerW[8];
		sceneBBoxW.GetCorners(sceneBBoxCornerW);

		float sceneBBoxNear = 100000.0f;
		float sceneBBoxFar = -100000.0f;

		XMVECTOR sceneBBoxCornerLV[8];
		for (uint i = 0; i < 8; ++i)
		{
			const XMVECTOR vConer = XMLoadFloat3(&sceneBBoxCornerW[i]);
			sceneBBoxCornerLV[i] = XMVector3Transform(vConer, lightView);
			float cornerZ = XMVectorGetZ(sceneBBoxCornerLV[i]);
			if (cornerZ < sceneBBoxNear)
				sceneBBoxNear = cornerZ;
			if (cornerZ > sceneBBoxFar)
				sceneBBoxFar = cornerZ;
		}

		XMVECTOR lightCamOrthoMin[JCsmOption::maxCountOfSplit];
		XMVECTOR lightCamOrthoMax[JCsmOption::maxCountOfSplit];

		//float outFrustumNear[JCsmOption::maxCountOfSplit];
		//float outFrustumFar[JCsmOption::maxCountOfSplit];

		float shadowDistance = option.GetShadowDistance();
		const XMMATRIX ndcToTextureSpaceM = JMatrix4x4::NdcToTextureSpace().LoadXM();
		//float preNear = camFrustumW.Near;

		for (uint i = 0; i < result.subFrustumCount; ++i)
		{
			//3. split frustum
			BoundingFrustum subFrustum = camFrustumW;
			//subFrustum.Near = preNear;
			subFrustum.Far = result.splitRate[i] * camFrustumW.Far;
			if (subFrustum.Far > shadowDistance)
				subFrustum.Far = shadowDistance;
			//preNear = subFrustum.Far;

			XMFLOAT3 wCorner[8];
			subFrustum.GetCorners(wCorner);

			lightCamOrthoMin[i] = JVector4<float>::PositiveInfV().ToXmV();
			lightCamOrthoMax[i] = JVector4<float>::NegativeInfV().ToXmV();
			XMVECTOR worldUnitsPerTexel = JVector4<float>::Zero().ToXmV();
			XMVECTOR multiplySetzwToZeroV = XMVectorSet(1.0f, 1.0f, 0.0f, 0.0f);

			//4. calculate light camera ortho min / max(frustum x, y)
			for (uint j = 0; j < 8; ++j)
			{
				XMVECTOR lightConer = XMVector3Transform(XMLoadFloat3(&wCorner[j]), lightView);
				lightCamOrthoMin[i] = XMVectorMin(lightConer, lightCamOrthoMin[i]);
				lightCamOrthoMax[i] = XMVectorMax(lightConer, lightCamOrthoMax[i]);
			}

			if (isFitToScene)
			{
				// Fit the ortho projection to the cascades far plane and a near plane of zero. 
				// Pad the projection to be the size of the diagonal of the Frustum partition. 
				// 
				// To do this, we pad the ortho transform so that it is always big enough to cover 
				// the entire camera view frustum. 

				XMVECTOR vDiagonal = XMLoadFloat3(&wCorner[0]) - XMLoadFloat3(&wCorner[6]);
				vDiagonal = XMVector3Length(vDiagonal);

				// The offset calculated will pad the ortho projection so that it is always the same size 
				// and big enough to cover the entire cascade interval.
				XMVECTOR vBoarderOffset = (vDiagonal - (lightCamOrthoMax[i] - lightCamOrthoMin[i])) * XMVectorSet(0.5f, 0.5f, 0.5f, 0.5f);
				// Set the Z and W components to zero.
				vBoarderOffset *= multiplySetzwToZeroV;

				// Add the offsets to the projection.
				lightCamOrthoMax[i] += vBoarderOffset;
				lightCamOrthoMin[i] -= vBoarderOffset;

				// The bound is the length of the diagonal of the frustum interval.
				FLOAT fCascadeBound = XMVectorGetX(vDiagonal);

				// The world units per texel are used to snap the shadow the orthographic projection
				// to texel sized increments.  This keeps the edges of the shadows from shimmering.
				FLOAT fWorldUnitsPerTexel = fCascadeBound / (float)mapSize;
				worldUnitsPerTexel = XMVectorSet(fWorldUnitsPerTexel, fWorldUnitsPerTexel, 0.0f, 0.0f);
			}

			//https://learn.microsoft.com/en-us/windows/win32/dxtecharts/common-techniques-to-improve-shadow-depth-maps		
			if (canMoveLightTexelSize)
			{
				// We snape the camera to 1 pixel increments so that moving the camera does not cause the shadows to jitter.
				// This is a matter of integer dividing by the world space size of a texel
				lightCamOrthoMin[i] /= worldUnitsPerTexel;
				lightCamOrthoMin[i] = XMVectorFloor(lightCamOrthoMin[i]);
				lightCamOrthoMin[i] *= worldUnitsPerTexel;

				lightCamOrthoMax[i] /= worldUnitsPerTexel;
				lightCamOrthoMax[i] = XMVectorFloor(lightCamOrthoMax[i]);
				lightCamOrthoMax[i] *= worldUnitsPerTexel;
			}

			float outFrustumNear = FLT_MAX;
			float outFrustumFar = -FLT_MAX;
			//if (i == 0)
			{
				//5. calculate frustum near far	 
				ComputeNearFar(sceneBBoxCornerLV, lightCamOrthoMin[i], lightCamOrthoMax[i], outFrustumNear, outFrustumFar);
			}
			//else
			//{
			//	outFrustumNear = sceneBBoxNear;
			//	outFrustumFar = sceneBBoxFar;
			//}
			const float minX = XMVectorGetX(lightCamOrthoMin[i]);
			const float maxX = XMVectorGetX(lightCamOrthoMax[i]);
			const float minY = XMVectorGetY(lightCamOrthoMin[i]);
			const float maxY = XMVectorGetY(lightCamOrthoMax[i]);
			const XMMATRIX shadowProjM = XMMatrixOrthographicOffCenterLH(minX, maxX, minY, maxY, outFrustumNear, outFrustumFar);
 
			//const XMMATRIX shadowTextureM = shadowProjM * ndcToTextureSpaceM;
			XMMATRIX shadowTextureM = shadowProjM;
			result.shadowProjM[i].StoreXM(shadowProjM);

			shadowTextureM = shadowProjM * ndcToTextureSpaceM;
			result.scale[i].x = XMVectorGetX(shadowTextureM.r[0]);
			result.scale[i].y = XMVectorGetY(shadowTextureM.r[1]);
			result.scale[i].z = XMVectorGetZ(shadowTextureM.r[2]);
			result.scale[i].w = 1.0f;
			result.posOffset[i] = shadowTextureM.r[3];
			result.posOffset[i].w = 0;
			//result.frustumSize[i] = JVector2F(abs(maxX - minX), abs(maxY - minY));
			result.fNear[i] = outFrustumNear;
			result.fFar[i] = outFrustumFar; 
		}
	}
	JUserPtr<JCsmTargetInfo> JCsmHandlerInterface::CreateTargetInfo()
	{
		auto newInfo = Core::JPtrUtil::MakeOwnerPtr<JCsmTargetInfo>(GetCsmHandlerGuid());

		TargetData tPrivate;
		tPrivate.info = std::move(newInfo);
		if (!tPrivate.IsValid())
			return nullptr;

		const uint infoIndex = (uint)target.size();
		target.push_back(std::move(tPrivate));
		return target[infoIndex].info;
	}
	bool JCsmHandlerInterface::DestroyTargetInfo(const int index)
	{
		if (index < 0)
			return false;

		target.erase(target.begin() + index);
		return true;
	}
	void JCsmHandlerInterface::DestroyAllTargetInfo()
	{
		target.clear();
	}
	bool JCsmHandlerInterface::RegisterCsmHandlerface()
	{
		return JGraphicPrivate::CsmInterface::RegisterHandler(this);
	}
	bool JCsmHandlerInterface::DeRegisterCsmHandlerface()
	{
		const bool res = JGraphicPrivate::CsmInterface::DeRegisterHandler(this);
		target.clear();
		return res;
	}
}