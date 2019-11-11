#include "RigidBody.h"

RigidBody::RigidBody(std::vector<XMFLOAT3> points)
{
	//Count the points of the incoming list
	size_t uVertexCount = points.size();

	XMStoreFloat4x4(&modelMatrix, XMMatrixIdentity());

	//If there are none just return, we have no information to create the BS from
	if (uVertexCount == 0)
		return;

	//Max and min as the first vector of the list
	maxL = minL = points[0];

	//Get the max and min out of the list
	for (unsigned int i = 1; i < uVertexCount; ++i)
	{
		if (maxL.x < points[i].x) maxL.x = points[i].x;
		else if (minL.x > points[i].x) minL.x = points[i].x;

		if (maxL.y < points[i].y) maxL.y = points[i].y;
		else if (minL.y > points[i].y) minL.y = points[i].y;

		if (maxL.z < points[i].z) maxL.z = points[i].z;
		else if (minL.z > points[i].z) minL.z = points[i].z;
	}

	//with model matrix being the identity, local and global are the same
	minG = minL;
	maxG = maxL;

	//with the max and the min we calculate the center
	XMStoreFloat3(&center, (XMLoadFloat3(&maxL) + XMLoadFloat3(&minL)) / 2);

	//we calculate the distance between min and max vectors
	XMStoreFloat3(&halfWidth, (XMLoadFloat3(&maxL) + XMLoadFloat3(&minL)) / 2);

	//Get the distance between the center and either the min or the max
	XMVECTOR vector1 = XMLoadFloat3(&center);
	XMVECTOR vector2 = XMLoadFloat3(&minL);
	XMVECTOR vectorSub = XMVectorSubtract(vector1, vector2);
	XMVECTOR length = XMVector3Length(vectorSub);
	XMStoreFloat(&radius, length);
}

void RigidBody::SetModelMatrix(XMFLOAT4X4 modelMatrix)
{
	//Assign the model matrix
	this->modelMatrix = modelMatrix;

	//Calculate the 8 corners of the cube
	XMFLOAT3 v3Corner[8];
	//Back square
	v3Corner[0] = minL;
	v3Corner[1] = XMFLOAT3(maxL.x, minL.y, minL.z);
	v3Corner[2] = XMFLOAT3(minL.x, maxL.y, minL.z);
	v3Corner[3] = XMFLOAT3(maxL.x, maxL.y, minL.z);

	//Front square
	v3Corner[4] = XMFLOAT3(minL.x, minL.y, maxL.z);
	v3Corner[5] = XMFLOAT3(maxL.x, minL.y, maxL.z);
	v3Corner[6] = XMFLOAT3(minL.x, maxL.y, maxL.z);
	v3Corner[7] = maxL;

	//Place them in world space
	for (size_t uIndex = 0; uIndex < 8; ++uIndex)
	{
		//v3Corner[uIndex] = XMFLOAT3(m_m4ToWorld * vector4(v3Corner[uIndex], 1.0f));
		XMFLOAT4 vecF = XMFLOAT4(v3Corner[uIndex].x, v3Corner[uIndex].y, v3Corner[uIndex].z, 1.0f);
		XMVECTOR vec = XMLoadFloat4(&vecF);
		vec = XMVector4Transform(vec,XMLoadFloat4x4(&modelMatrix));

		XMStoreFloat3(&v3Corner[uIndex], vec);
	}

	//Identify the max and min as the first corner
	maxG = minG = v3Corner[0];

	//get the new max and min for the global box
	for (size_t i = 1; i < 8; ++i)
	{
		if (maxG.x < v3Corner[i].x) maxG.x = v3Corner[i].x;
		else if (minG.x > v3Corner[i].x) minG.x = v3Corner[i].x;

		if (maxG.y < v3Corner[i].y) maxG.y = v3Corner[i].y;
		else if (minG.y > v3Corner[i].y) minG.y = v3Corner[i].y;

		if (maxG.z < v3Corner[i].z) maxG.z = v3Corner[i].z;
		else if (minG.z > v3Corner[i].z) minG.z = v3Corner[i].z;
	}

	//we calculate the distance between min and max vectors
	//arbbSize = maxG - minG;
	XMStoreFloat3(&arbbSize, XMLoadFloat3(&maxG) - XMLoadFloat3(&minG));
}

XMFLOAT3 RigidBody::GetMinLocal()
{
	return minL;
}

XMFLOAT3 RigidBody::GetMaxLocal()
{
	return maxL;
}

XMFLOAT3 RigidBody::GetMinGlobal()
{
	return minG;
}

XMFLOAT3 RigidBody::GetMaxGlobal()
{
	return maxG;
}

XMFLOAT4X4 RigidBody::GetModelMatrix()
{
	return modelMatrix;
}

XMFLOAT3 RigidBody::GetCenterLocal()
{
	return center;
}

XMFLOAT3 RigidBody::GetCenterGlobal()
{
	XMFLOAT3 globalCenter;

	XMStoreFloat3(&globalCenter, XMVector4Transform(XMVectorSet(center.x, center.y, center.z, 1.0f), XMLoadFloat4x4(&modelMatrix)));
	return globalCenter;
}

float RigidBody::GetRadius()
{
	return radius;
}

bool RigidBody::BoundingSphereCheck(std::shared_ptr<RigidBody> other)
{
	XMVECTOR vector1 = XMLoadFloat3(&GetCenterGlobal());
	XMVECTOR vector2 = XMLoadFloat3(&other->GetCenterGlobal());
	XMVECTOR vectorSub = XMVectorSubtract(vector1, vector2);
	XMVECTOR length = XMVector3Length(vectorSub);
	XMFLOAT3 dist;
	XMStoreFloat3(&dist, length);

	if (dist.x < this->GetRadius() + other->GetRadius())
	{
		return true;
	}

	return false;

}

bool RigidBody::SATCollision(std::shared_ptr<RigidBody> other)
{
	
	if (this == other.get())
	{
		return false;
	}

	if (!BoundingSphereCheck(other))
	{
		return false;
	}
	
	//corners of the first rigid body
	std::vector<XMFLOAT3> OBBPoints;

	//back bottom left
	XMFLOAT3 backBottomLeft1 = minL;
	OBBPoints.emplace_back(backBottomLeft1);
	//front up right
	XMFLOAT3 frontUpRight1 = maxL;
	OBBPoints.emplace_back(frontUpRight1);
	//back bottom right point
	XMFLOAT3 backBottomRight1 = XMFLOAT3(maxL.x, minL.y, minL.z);
	OBBPoints.emplace_back(backBottomRight1);
	//back up right point
	XMFLOAT3 backUpRight1 = XMFLOAT3(maxL.x, maxL.y, minL.z);
	OBBPoints.emplace_back(backUpRight1);
	//back up left point
	XMFLOAT3 backUpLeft1 = XMFLOAT3(minL.x, maxL.y, minL.z);
	OBBPoints.emplace_back(backUpLeft1);
	//front bottom left
	XMFLOAT3 frontBottomLeft1 = XMFLOAT3(minL.x, minL.y, maxL.z);
	OBBPoints.emplace_back(frontBottomLeft1);
	//front bottom right point
	XMFLOAT3 frontBottomRight1 = XMFLOAT3(maxL.x, minL.y, maxL.z);
	OBBPoints.emplace_back(frontBottomRight1);
	//front up left point
	XMFLOAT3 frontUpLeft1 = XMFLOAT3(minL.x, maxL.y, maxL.z);
	OBBPoints.emplace_back(frontUpLeft1);

	//calculating the global position of every single corner point
	for (size_t i = 0; i < OBBPoints.size(); i++)
	{
		//OBBPoints[i] = XMFLOAT3(GetModelMatrix() * vector4(OBBPoints[i], 1.0f));
		XMStoreFloat3(&OBBPoints[i],
			XMVector4Transform(XMVectorSet(OBBPoints[i].x, OBBPoints[i].y, OBBPoints[i].z, 1.0f), XMLoadFloat4x4(&modelMatrix)));
	}

	//corners of the second rigid body
	XMFLOAT3 v3MinLOther = other->GetMinLocal();
	XMFLOAT3 v3MaxLOther = other->GetMaxLocal();

	//corners of the first rigid body
	std::vector<XMFLOAT3> OtherOBBPoints;

	XMFLOAT3 backBottomLeft2 = v3MinLOther;
	OtherOBBPoints.emplace_back(backBottomLeft2);
	//front up right
	XMFLOAT3 frontUpRight2 = v3MaxLOther;
	OtherOBBPoints.emplace_back(frontUpRight2);
	//back bottom right point
	XMFLOAT3 backBottomRight2 = XMFLOAT3(v3MaxLOther.x, v3MinLOther.y, v3MinLOther.z);
	OtherOBBPoints.emplace_back(backBottomRight2);
	//back up right point
	XMFLOAT3 backUpRight2 = XMFLOAT3(v3MaxLOther.x, v3MaxLOther.y, v3MinLOther.z);
	OtherOBBPoints.emplace_back(backUpRight2);
	//back up left point
	XMFLOAT3 backUpLeft2 = XMFLOAT3(v3MinLOther.x, v3MaxLOther.y, v3MinLOther.z);
	OtherOBBPoints.emplace_back(backUpLeft2);
	//front bottom left
	XMFLOAT3 frontBottomLeft2 = XMFLOAT3(v3MinLOther.x, v3MinLOther.y, v3MaxLOther.z);
	OtherOBBPoints.emplace_back(frontBottomLeft2);
	//front bottom right point
	XMFLOAT3 frontBottomRight2 = XMFLOAT3(v3MaxLOther.x, v3MinLOther.y, v3MaxLOther.z);
	OtherOBBPoints.emplace_back(frontBottomRight2);
	//front up left point
	XMFLOAT3 frontUpLeft2 = XMFLOAT3(v3MinLOther.x, v3MaxLOther.y, v3MaxLOther.z);
	OtherOBBPoints.emplace_back(frontUpLeft2);

	//calculating the global space of every single corner point
	for (size_t i = 0; i < OtherOBBPoints.size(); i++)
	{
		XMStoreFloat3(&OtherOBBPoints[i],
			XMVector4Transform(XMVectorSet(OtherOBBPoints[i].x, OtherOBBPoints[i].y, OtherOBBPoints[i].z, 1.0f), XMLoadFloat4x4(&other->GetModelMatrix())));
	}

	//list to hold the axis of seperation
	std::vector<XMFLOAT3> normalList;

	//normal of x axis of this body
	XMFLOAT3 A0;
	XMStoreFloat3(&A0,
		XMVector4Transform(XMVectorSet(1.f,0,0,0),XMLoadFloat4x4(&modelMatrix)));
	normalList.emplace_back(A0);
	//normal of y axis of this body
	XMFLOAT3 A1;
	XMStoreFloat3(&A1,
		XMVector4Transform(XMVectorSet(0.f, 1.0f, 0, 0), XMLoadFloat4x4(&modelMatrix)));
	normalList.emplace_back(A1);
	//normal of the z axis of this body
	XMFLOAT3 A2;
	XMStoreFloat3(&A2,
		XMVector4Transform(XMVectorSet(0.f, 0, 1.0f, 0), XMLoadFloat4x4(&modelMatrix)));
	normalList.emplace_back(A2);

	//normal of x axis of other body
	XMFLOAT3 B0;
	XMStoreFloat3(&B0,
		XMVector4Transform(XMVectorSet(1.f, 0, 0.0f, 0), XMLoadFloat4x4(&other->GetModelMatrix())));
	normalList.emplace_back(B0);
	//normal of y axis of other body
	XMFLOAT3 B1;
	XMStoreFloat3(&B1,
		XMVector4Transform(XMVectorSet(0.f, 1.f, 0.0f, 0), XMLoadFloat4x4(&other->GetModelMatrix())));
	normalList.emplace_back(B1);
	//normal of the z axis of other body
	XMFLOAT3 B2;
	XMStoreFloat3(&B2,
		XMVector4Transform(XMVectorSet(0.f, 0, 1.0f, 0), XMLoadFloat4x4(&other->GetModelMatrix())));
	normalList.emplace_back(B2);

	for (size_t i = 0; i < normalList.size(); i++)
	{
		XMStoreFloat3(&normalList[i], XMVector3Normalize(XMLoadFloat3(&normalList[i])));
	}
	
	//9 cross product axes
	//the logic of these axes is as follows
	//for each normal axis in this rigid body, find a cross product of that axis
	//with each axis on the other rigid body

	XMFLOAT3 A0CrossB0;
	//glm::cross(A0, B0);
	XMStoreFloat3(&A0CrossB0, XMVector3Cross(XMLoadFloat3(&A0), XMLoadFloat3(&B0)));
	normalList.emplace_back(A0CrossB0);

	XMFLOAT3 A0CrossB1;// = glm::cross(A0, B1);
	XMStoreFloat3(&A0CrossB1, XMVector3Cross(XMLoadFloat3(&A0), XMLoadFloat3(&B1)));
	normalList.emplace_back(A0CrossB1);

	XMFLOAT3 A0CrossB2;// = glm::cross(A0, B2);
	XMStoreFloat3(&A0CrossB2, XMVector3Cross(XMLoadFloat3(&A0), XMLoadFloat3(&B2)));
	normalList.emplace_back(A0CrossB2);

	XMFLOAT3 A1CrossB0;// = glm::cross(A1, B0);
	XMStoreFloat3(&A1CrossB0, XMVector3Cross(XMLoadFloat3(&A1), XMLoadFloat3(&B0)));
	normalList.emplace_back(A1CrossB0);

	XMFLOAT3 A1CrossB1;// = glm::cross(A1, B1);
	XMStoreFloat3(&A1CrossB1, XMVector3Cross(XMLoadFloat3(&A1), XMLoadFloat3(&B1)));
	normalList.emplace_back(A1CrossB1);

	XMFLOAT3 A1CrossB2;// = glm::cross(A1, B2);
	XMStoreFloat3(&A1CrossB2, XMVector3Cross(XMLoadFloat3(&A1), XMLoadFloat3(&B2)));
	normalList.emplace_back(A1CrossB2);

	XMFLOAT3 A2CrossB0;// = glm::cross(A2, B0);
	XMStoreFloat3(&A2CrossB0, XMVector3Cross(XMLoadFloat3(&A2), XMLoadFloat3(&B0)));
	normalList.emplace_back(A2CrossB0);

	XMFLOAT3 A2CrossB1;// = glm::cross(A2, B1);
	XMStoreFloat3(&A2CrossB1, XMVector3Cross(XMLoadFloat3(&A2), XMLoadFloat3(&B1)));
	normalList.emplace_back(A2CrossB1);

	XMFLOAT3 A2CrossB2;// = glm::cross(A2, B2);
	XMStoreFloat3(&A2CrossB2, XMVector3Cross(XMLoadFloat3(&A2), XMLoadFloat3(&B2)));
	normalList.emplace_back(A2CrossB2);

	bool result = true;

	for (size_t i = 0; i < normalList.size(); i++)
	{
		XMStoreFloat3(&normalList[i], XMVector3Normalize(XMLoadFloat3(&normalList[i])));
	}

	//looping through the normals and checking if there is overlap on any of the normal
	for (int i = 1; i < normalList.size() + 1; i++)
	{
		//if there is no overlap on even one axis, it means that there is no collision
		if (IsOverlapping(normalList[(size_t)i - 1], OBBPoints, OtherOBBPoints) == false)
		{
			//result is set to a value not equal to 0;
			result = false;
			break;
		}
	}

	//there is no axis test that separates this two objects
	return result;
}

bool RigidBody::IsOverlapping(XMFLOAT3 axis,std::vector<XMFLOAT3> thisPoints,std::vector<XMFLOAT3> otherPoints)
{
	//if the cross product is a zero vector then assume there is an overlap
	if (axis.x==0&&axis.y==0&&axis.z==0)
	{
		return true;
	}

	bool overlap = false;

	//vector to hold the dot products of the this rigid body's points to the given axis
	std::vector<float> dots1;

	XMFLOAT4 length;
	XMStoreFloat4(&length, XMVector3Length(XMLoadFloat3(&axis)));

	//adding the dot products to a vector
	for (int i = 0; i < thisPoints.size(); i++)
	{
		XMFLOAT3 dot;
		XMStoreFloat3(&dot, XMVector3Dot(XMLoadFloat3(&axis), XMLoadFloat3(&thisPoints[i])));
		dots1.emplace_back(dot.x/length.x);
	}

	//vector to hold the dot products of the other rigid body's points to the given axis
	std::vector<float> dots2;

	//adding the dot products of to a vector
	for (int i = 0; i < otherPoints.size(); i++)
	{
		XMFLOAT3 dot;
		XMStoreFloat3(&dot, XMVector3Dot(XMLoadFloat3(&axis), XMLoadFloat3(&otherPoints[i])));
		dots2.emplace_back(dot.x/length.x);
	}

	//holding the min and max from the first set of dot products
	float min1 = *std::min_element(dots1.begin(), dots1.end());
	float max1 = *std::max_element(dots1.begin(), dots1.end());

	//holding the min and max from the first set of dot products
	float min2 = *std::min_element(dots2.begin(), dots2.end());
	float max2 = *std::max_element(dots2.begin(), dots2.end());

	//checking if there is an overlap between the dot products of both objects
	if (min2 < max1 && min1 < max2)
	{
		overlap = true;
	}

	//returning whether objects are overlapping or not
	return overlap;
}
