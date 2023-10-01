#include"JEditorSceneImageInteraction.h"
#include"../Gui/JGui.h"  
#include"../../Object/Component/Camera/JCamera.h"    
#include"../../Object/Component/Transform/JTransform.h"
#include"../../Object/Resource/Scene/JScene.h" 
#include"../../Object/GameObject/JGameObject.h" 
#include"../../Core/Geometry/JRay.h"
#include"../../Core/Geometry/JBBox.h" 
#include"../../Core/Geometry/JDirectXCollisionEx.h" 
#include"../../Core/Math/JMathHelper.h"  
//Debug
//#include"../../Develop/Debug/JDevelopDebug.h"
using namespace DirectX;
namespace JinEngine::Editor
{
	namespace Private
	{
		static bool IsValidData(const JUserPtr<JScene>& scene, 
			const JUserPtr<JCamera>& cam, 
			const JVector2<float>& sceneImageWorldMinPoint,
			const bool checkMouseInWindow)
		{
			if (!scene.IsValid() || !cam.IsValid())
				return false;

			const JVector2<float> windowPos = JGui::GetWindowPos();
			const JVector2<float> windowSize = JGui::GetWindowSize();
			if (checkMouseInWindow)
			{
				if (!JGui::IsMouseInRect(sceneImageWorldMinPoint, windowSize))
					return false;
			}
			 
			return sceneImageWorldMinPoint.Contained(windowPos, windowSize);
		}
		static JVector2<float> ToNdcPos(JUserPtr<JCamera> cam,
			const JVector2<uint>& mousePos,
			const JVector2<float>& sceneImageWorldMinPoint)
		{
			//1.screen에 local mouse pos를 계산
			//2.local mouse pos를 현재 editor window 크기(scene image 크기)에 맞게 scale
			//3. scale된 pos를 world로 변환(screen -> ndc -> proj -> view -> world) 

			//Editor Window view port size = tab + menu + contents 
			//world mouse pos = 0 ~ app display size
			//localMousePos = 0 ~ scene image size
			const JVector2<float> localMousePos = mousePos - sceneImageWorldMinPoint;
			const JVector2<float> camViewSize = JVector2<float>(cam->GetFarViewWidth(), cam->GetFarViewHeight());
			const float widthRate = (float)camViewSize.x / JGui::GetWindowSize().x;
			const float heightRate = (float)camViewSize.y / JGui::GetWindowSize().y;

			//scale window size
			const JVector2<float> finalMousePos = JVector2<float>(localMousePos.x * widthRate, localMousePos.y * heightRate);
			//screen 0 ~ n
			//ndc -1 ~ 1
			//Xndc = 2Xs / w - 1
			//Yndc = -( 2Ys / w + 1)

			const float vx = ((2.0f * finalMousePos.x) / cam->GetFarViewWidth() - 1.0f);
			const float vy = ((-2.0f * finalMousePos.y) / cam->GetFarViewHeight() + 1.0f); 
			return JVector2<float>(vx, vy);
		}
		static JVector2<float> ToViewPos(JUserPtr<JCamera> cam,
			const JVector2<uint>& mousePos,
			const JVector2<float>& sceneImageScreenMinPoint,
			const float pointZValue = 1.0f)
		{
			//Xv = Xndc * r * tan(fov) * z = (Xndc * z)/ proj(0, 0)	..투영창으로 스케일링
			//Yv = Yndc * r * tan(fov) * z= (Yndc * z) / proj(1, 1)	..투영창으로 스케일링 
			//screen상에 점이므로 z 는 기본적으로 1.0이다
			const JVector2<float> ndcPos = ToNdcPos(cam, mousePos, sceneImageScreenMinPoint);
			const JMatrix4x4 proj = cam->GetProj4x4();
			return JVector2<float>((ndcPos.x * pointZValue) / proj(0, 0), (ndcPos.y * pointZValue) / proj(1, 1));
		}
	}
	JUserPtr<JGameObject> JEditorSceneImageInteraction::Intersect(JUserPtr<JScene> scene,
		JUserPtr<JCamera> cam,
		const J_ACCELERATOR_LAYER layer,
		const JVector2<float>& sceneImageWorldMinPoint) noexcept
	{
		if (!Private::IsValidData(scene, cam, sceneImageWorldMinPoint, true))
			return nullptr;

		//계산에서 구한 pos는 world변환 후 ray dir로 사용 ray pos는 cam eye를 world로 변환한값이다
		JVector2<float> viewPos = Private::ToViewPos(cam, JGui::GetMousePos(), sceneImageWorldMinPoint);
		const XMMATRIX invView = XMMatrixInverse(nullptr, cam->GetView());

		const XMVECTOR rayOri = XMVector3TransformCoord(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), invView);
		const XMVECTOR rayDir = XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(viewPos.x, viewPos.y, 1.0f, 0.0f), invView));

		return scene->IntersectFirst(Core::JRay{ rayOri, rayDir }, layer);
	}
	std::vector<JUserPtr<JGameObject>> JEditorSceneImageInteraction::Contain(JUserPtr<JScene> scene,
		JUserPtr<JCamera> cam,
		const J_ACCELERATOR_LAYER layer,
		const JVector2<float>& sceneImageWorldMinPoint,
		const JVector2<float>& minMousePos,
		const JVector2<float>& maxMousePos) noexcept
	{
		const bool isSameMosuePos = minMousePos == maxMousePos;
		if (!Private::IsValidData(scene, cam, sceneImageWorldMinPoint, isSameMosuePos))
			return std::vector<JUserPtr<JGameObject>>{};
		
		if (isSameMosuePos)
			return std::vector<JUserPtr<JGameObject>>{};

		//create local frustum by bbox min max
		JVector2<float> posV00 = Private::ToViewPos(cam, minMousePos, sceneImageWorldMinPoint);
		JVector2<float> posV01 = Private::ToViewPos(cam, maxMousePos, sceneImageWorldMinPoint);

		const XMMATRIX view = cam->GetView();
		const XMMATRIX invView = XMMatrixInverse(nullptr, view);

		const JVector3<float> rayV00 = XMVector3Normalize(XMVectorSet(posV00.x, posV00.y, 1.0f, 0.0f));
		const JVector3<float> rayV01 = XMVector3Normalize(XMVectorSet(posV01.x, posV01.y, 1.0f, 0.0f));
		const JVector3<float> minRayV = JVector3<float>::Min(rayV00, rayV01);
		const JVector3<float> maxRayV = JVector3<float>::Max(rayV00, rayV01);
 
		const XMMATRIX p = XMMatrixPerspectiveOffCenterLH (minRayV.x, maxRayV.x, minRayV.y, maxRayV.y, cam->GetNear(), cam->GetFar());
		const BoundingFrustum localFrustum(p);
 
		//to world frustum
		JMatrix4x4 worldM = cam->GetTransform()->GetWorldMatrix4x4();
		BoundingFrustum worldFrustum;
		localFrustum.Transform(worldFrustum, worldM.LoadXM());

		//rquest contain
		JAcceleratorContainInfo info(worldFrustum, layer); 
		info.allowIntersect = true;
		scene->Contain(info);  
		return info.resultObjVec;
	}
}