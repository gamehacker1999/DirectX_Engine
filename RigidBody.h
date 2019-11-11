#pragma once
#include<DirectXMath.h>
#include<vector>
#include<algorithm>
#include<memory>
using namespace DirectX;
class RigidBody
{
	XMFLOAT3 center; //center point in local space
	XMFLOAT3 minL; //minimum coordinate in local space (for OBB)
	XMFLOAT3 maxL; //maximum coordinate in local space (for OBB)
	XMFLOAT3 minG; //minimum coordinate in global space (for ARBB)
	XMFLOAT3 maxG; //maximum coordinate in global space (for ARBB)
	XMFLOAT3 halfWidth; //half the size of the Oriented Bounding Box
	XMFLOAT3 arbbSize;// size of the Axis (Re)Alligned Bounding Box
	float radius;

	XMFLOAT4X4 modelMatrix; //Matrix that will take us from local to world coordinate

public:
	RigidBody(std::vector<XMFLOAT3> points);
	void SetModelMatrix(XMFLOAT4X4 modelMatrix);

	//getters
	XMFLOAT3 GetMinLocal();
	XMFLOAT3 GetMaxLocal();
	XMFLOAT3 GetMinGlobal();
	XMFLOAT3 GetMaxGlobal();
	XMFLOAT4X4 GetModelMatrix();
	XMFLOAT3 GetCenterLocal();
	XMFLOAT3 GetCenterGlobal();
	float GetRadius();
	bool BoundingSphereCheck(std::shared_ptr<RigidBody> other);

	//collision detection
	bool SATCollision(std::shared_ptr<RigidBody> other);
	bool IsOverlapping(XMFLOAT3 normal,std::vector<XMFLOAT3> thisPoints, std::vector<XMFLOAT3> otherPoints);
};

