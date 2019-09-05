#include "Mesh.h"
using namespace DirectX;
Mesh::Mesh(Vertex* vertices, unsigned int numVertices, unsigned int* indices, int numIndices, ID3D11Device* device)
{
	this->numIndices = numIndices; //stroring the num of indices

	//setting up the vertex buffer description
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = numVertices*sizeof(Vertex);       // number of vertices in the buffer
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER; // Tells DirectX this is a vertex buffer
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	//holding the initial vertex data
	D3D11_SUBRESOURCE_DATA initialVertexData;
	initialVertexData.pSysMem = reinterpret_cast<void*>(vertices); //casting the vertex data to a void pointer

	//creating the vertex buffer data
	device->CreateBuffer(&vbd, &initialVertexData, &vertexBuffer);

	// Create the INDEX BUFFER description ------------------------------------
// - The description is created on the stack because we only need
//    it to create the buffer.  The description is then useless.
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = numIndices*sizeof(unsigned int);         // 3 = number of indices in the buffer
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER; // Tells DirectX this is an index buffer
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	// Create the proper struct to hold the initial index data
	// - This is how we put the initial data into the buffer
	D3D11_SUBRESOURCE_DATA initialIndexData;
	initialIndexData.pSysMem = reinterpret_cast<void*>(indices);

	// Actually create the buffer with the initial data
	// - Once we do this, we'll NEVER CHANGE THE BUFFER AGAIN
	device->CreateBuffer(&ibd, &initialIndexData, &indexBuffer);
}

Mesh::Mesh(std::string fileName, ID3D11Device* device)
{
	std::ifstream ifile(fileName.c_str());

	std::string line; //line that stores file data

	//check if the file exists
	if (ifile.is_open())
	{
		//making a list of the position, normals and uvs
		std::vector<XMFLOAT3> positions;
		std::vector<XMFLOAT3> normals;
		std::vector<XMFLOAT2> uvs;
		std::vector<XMFLOAT3> tangents;
		std::vector<XMFLOAT3> bitangents;

		//list of vertices and indices
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;


		//num of verts and indices
		UINT vertCount = 0;
		UINT indexCount = 0;

		while (std::getline(ifile, line))
		{
			std::vector<std::string> words; //this holds all the individual characters of the line

			size_t pos = 0;
			size_t curPos = 0;

			//splitting the string with the spacebar
			while (pos <= line.length())
			{
				//finding the space string
				//taking a substring from that point
				//storing that substring in the list
				pos = line.find(" ", curPos);
				std::string word = line.substr(curPos, (size_t)(pos - curPos));
				curPos = pos + 1;
				words.emplace_back(word);
			}

			if (line.find("v ") == 0)
			{
				//storing the position if the line starts with "v"
				positions.emplace_back(XMFLOAT3(std::stof(words[1]), std::stof(words[2]), std::stof(words[3])));
			}

			if (line.find("vn") == 0)
			{
				//storing the normals if the line starts with "vn"
				normals.emplace_back(XMFLOAT3(std::stof(words[1]), std::stof(words[2]), std::stof(words[3])));
			}

			if (line.find("vt") == 0)
			{
				//storing the textures if the line starts with "vt"
				uvs.emplace_back(XMFLOAT2(std::stof(words[1]), std::stof(words[2])));
			}

			if (line.find("f") == 0)
			{
				//storing the faces if the line starts with "f"

				//this the the list of the faces on this line
				std::vector<std::vector<std::string>> listOfFaces;

				listOfFaces.reserve(10);

				//looping through all the faces
				for (int i = 0; i < words.size() - 1; i++)
				{
					std::vector<std::string> face; //holds the individial data of each face
					face.reserve(10);
					pos = 0;
					curPos = 0;

					//splitting each vertex further to seperate it based on a '/' character
					while (pos <= words[i + 1].length())
					{
						(pos = words[i + 1].find("/", curPos));
						std::string word = words[i + 1].substr(curPos, pos - curPos);
						curPos = pos + 1;
						face.emplace_back(word);
					}

					//adding this face to the list of faces
					listOfFaces.emplace_back(face);
				}

				//first vertex
				Vertex v1;
				v1.Position = positions[std::stoi(listOfFaces[0][0]) - 1];
				v1.normal = normals[std::stoi(listOfFaces[0][2]) - 1];
				v1.uv = uvs[std::stoi(listOfFaces[0][1]) - 1];

				//second vertex
				Vertex v2;
				v2.Position = positions[std::stoi(listOfFaces[1][0]) - 1];
				v2.normal = normals[std::stoi(listOfFaces[1][2]) - 1];
				v2.uv = uvs[std::stoi(listOfFaces[1][1]) - 1];

				//third vertex
				Vertex v3;
				v3.Position = positions[std::stoi(listOfFaces[2][0]) - 1];
				v3.normal = normals[std::stoi(listOfFaces[2][2]) - 1];
				v3.uv = uvs[std::stoi(listOfFaces[2][1]) - 1];

				//since the texture space starts at top left, it its probably upside down
				v1.uv.y = 1 - v1.uv.y;
				v2.uv.y = 1 - v2.uv.y;
				v3.uv.y = 1 - v3.uv.y;

				//since the coordinates system are inverted we have to flip the normals and 
				//negate the z axis of position
				v1.normal.z *= -1;
				v2.normal.z *= -1;
				v3.normal.z *= -1;

				v1.Position.z *= -1;
				v2.Position.z *= -1;
				v3.Position.z *= -1;

				//we have to flip the winding order
				vertices.emplace_back(v1);
				vertices.emplace_back(v3);
				vertices.emplace_back(v2);

				//adding indices
				indices.emplace_back(vertCount); vertCount++;
				indices.emplace_back(vertCount); vertCount++;
				indices.emplace_back(vertCount); vertCount++;


				//if it has a potential 4th vertex, add it, but ignore n-gons
				if (listOfFaces.size() > 3)
				{
					//fourth vertex
					Vertex v4;
					v4.Position = positions[std::stoi(listOfFaces[3][0]) - 1];
					v4.normal = normals[std::stoi(listOfFaces[3][2]) - 1];
					v4.uv = uvs[std::stoi(listOfFaces[3][1]) - 1];

					//do the same handedness conversion
					v4.Position.z *= -1;
					v4.normal.z *= -1;
					v4.uv.y = 1 - v4.uv.y;

					//adding the triangle
					vertices.emplace_back(v1);
					vertices.emplace_back(v4);
					vertices.emplace_back(v3);

					//adding indices
					indices.emplace_back(vertCount); vertCount++;
					indices.emplace_back(vertCount); vertCount++;
					indices.emplace_back(vertCount); vertCount++;

				}
			}

		}

		numIndices = vertCount;

		//create the vertex and index buffer
		//vertexBuffer
		D3D11_BUFFER_DESC vbd;
		memset(&vbd, 0, sizeof(vbd));
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = vertCount * sizeof(Vertex);

		//creating a subresource
		D3D11_SUBRESOURCE_DATA initialVertexData;
		initialVertexData.pSysMem = reinterpret_cast<void*>(vertices.data()); //the vertex list

		device->CreateBuffer(&vbd, &initialVertexData, &vertexBuffer);

		//index buffer description
		D3D11_BUFFER_DESC ibd;
		memset(&ibd, 0, sizeof(ibd));
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = vertCount * sizeof(unsigned int);

		D3D11_SUBRESOURCE_DATA initialIndexData;
		initialIndexData.pSysMem = reinterpret_cast<void*>(indices.data());

		//creating index buffer
		device->CreateBuffer(&ibd, &initialIndexData, &indexBuffer);
		ifile.close();
	}
}

void Mesh::CalculateTangents(std::vector<Vertex>& vertices, std::vector<XMFLOAT3>& position, std::vector<XMFLOAT3>& normals,
	std::vector<XMFLOAT2>& uvs, std::vector<XMFLOAT3>& tangents, std::vector<XMFLOAT3>& bitangents, unsigned int vertCount)
{
	//compute the tangents and bitangents for each triangle
	for (size_t i = 0; i < vertCount; i+=3)
	{
		//getting the position, normal, and uv data for vertex
		XMFLOAT3 vert1 = vertices[i].Position;
		XMFLOAT3 vert2 = vertices[i+1].Position;
		XMFLOAT3 vert3 = vertices[i+2].Position;

		XMFLOAT2 uv1 = vertices[i].uv;
		XMFLOAT2 uv2 = vertices[i+1].uv;
		XMFLOAT2 uv3 = vertices[i+2].uv;

		//finding the two edges of the triangles
		auto tempEdge = XMLoadFloat3(&vert2)-XMLoadFloat3(&vert1);
		XMFLOAT3 edge1;
		XMStoreFloat3(&edge1, tempEdge);
		tempEdge = XMLoadFloat3(&vert3) - XMLoadFloat3(&vert1);
		XMFLOAT3 edge2;
		XMStoreFloat3(&edge2, tempEdge);

		//finding the difference in UVs
		XMFLOAT2 deltaUV1;
		XMStoreFloat2(&deltaUV1, XMLoadFloat2(&uv2) - XMLoadFloat2(&uv1));
		XMFLOAT2 deltaUV2;
		XMStoreFloat2(&deltaUV2, XMLoadFloat2(&uv3) - XMLoadFloat2(&uv1));

		//calculate the inverse of the delta uv matrix
		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
		//calculating the tangent of the triangle
		XMFLOAT3 tangent; 
		XMStoreFloat3(&tangent,XMLoadFloat3(&edge1) * deltaUV2.y - XMLoadFloat3(&edge2) * deltaUV1.y);

		//adding the tangents to the list, same for all three vertices
		tangents.emplace_back(tangent);
		tangents.emplace_back(tangent);
		tangents.emplace_back(tangent);

	}
}

Mesh::~Mesh()
{
	//releasing the vertex and index buffer
	if(vertexBuffer)
		vertexBuffer->Release();

	if(indexBuffer)
		indexBuffer->Release();
}

ID3D11Buffer* Mesh::GetVertexBuffer()
{
	return vertexBuffer;
}

ID3D11Buffer* Mesh::GetIndexBuffer()
{
	return indexBuffer;
}

unsigned int Mesh::GetIndexCount()
{
	return numIndices;
}

void Mesh::LoadOBJ(std::string fileName)
{

}
