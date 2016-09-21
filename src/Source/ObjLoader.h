#pragma once
#pragma warning(push, 0)
#	include <glm/glm.hpp>
#pragma warning(pop)
#include <vector>
#include <d3d9.h>

typedef glm::vec3 Vector3;
typedef D3DCOLOR Color;

template<DWORD fvf>
struct BaseVertex
{
	DWORD static const FVF = fvf;
};	// struct BaseVertex

struct LineMeshVertex : BaseVertex<D3DFVF_XYZ | D3DFVF_DIFFUSE>
{
	Vector3 Position;
	Color   DiffuseColor;

	LineMeshVertex(Vector3 const& p, Color const c) : Position(p), DiffuseColor(c) {}
};	// struct LineVertex

struct TriangleMeshVertex : BaseVertex<D3DFVF_XYZ | D3DFVF_NORMAL /*| D3DFVF_DIFFUSE*/ | D3DFVF_TEX1>
{
	Vector3 Position;
	Vector3 Normal;
//	Color   DiffuseColor;
	glm::vec2 UV;

	TriangleMeshVertex(Vector3 const& p, Vector3 const& n, Color const c, glm::vec2 const& uv) : Position(p), Normal(n), /*DiffuseColor(c),*/ UV(uv) {}
};	// struct Vertex

template<typename PrimitiveVertex, D3DPRIMITIVETYPE primitiveType>
struct BaseMesh
{
public:
	std::vector<PrimitiveVertex> m_Vertices;
	LPDIRECT3DVERTEXBUFFER9 m_VerticesBuffer = nullptr;
	DWORD m_PrimitivesCount = 0;

public:
	void SetupVerticesBuffer(LPDIRECT3DDEVICE9 const device)
	{
		device->CreateVertexBuffer(m_Vertices.size() * sizeof(PrimitiveVertex), 0, PrimitiveVertex::FVF,
								   D3DPOOL_MANAGED, &m_VerticesBuffer, nullptr);
		
		VOID* verticesData;
		m_VerticesBuffer->Lock(0, 0, &verticesData, 0);
		memcpy_s(verticesData, m_Vertices.size() * sizeof(PrimitiveVertex), m_Vertices.data(),
				 m_Vertices.size() * sizeof(PrimitiveVertex));
		m_VerticesBuffer->Unlock();
		
		m_PrimitivesCount = m_Vertices.size() / (primitiveType == D3DPT_TRIANGLELIST ? 3 : 2);
	}

	void Render(LPDIRECT3DDEVICE9 const device)
	{
		device->SetFVF(PrimitiveVertex::FVF);
		device->SetStreamSource(0, m_VerticesBuffer, 0, sizeof(PrimitiveVertex));
		device->DrawPrimitive(primitiveType, 0, m_PrimitivesCount);
	}

};	// struct BaseMesh

typedef BaseMesh<LineMeshVertex, D3DPT_LINELIST> LineMesh;
typedef BaseMesh<TriangleMeshVertex, D3DPT_TRIANGLELIST> Mesh;

bool ImportozameshenieBJD(const char* path, Mesh& out_vertices);
