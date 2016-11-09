// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// Copyright (C) 2016 Butakov Oleg, Plaxin Gleb.
// Computational Methods @ Computational Mathematics & Cybernetics, MSU.
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

#pragma once
#include "PresentationEngine.hpp"

#pragma warning(push, 0)
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#pragma warning(pop)

#if SUPPORT_LOADING_FROM_FILE
#include <d3dx9.h>
#endif	// if SUPPORT_LOADING_FROM_FILE

namespace Presentation2
{
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //
	// Meshes.
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //

	// -----------------------
#if SUPPORT_LOADING_FROM_FILE
	template<typename TVertex, D3DPRIMITIVETYPE TPrimitiveType>
	ADINT void Mesh<TVertex, TPrimitiveType>::UpdateVertexBuffer(LPCWSTR const path, dxm::argb const color) const
	{
		assert(path != nullptr);

#if !_DEBUG
		OutputDebugStringA("Loading resource from file in release version.");
#endif	// if !_DEBUG

		std::vector<UINT> vertexIndices, texCoordIndices, normalIndices;
		std::vector<dxm::vec3> vertexBuffer;
		std::vector<dxm::vec2> texCoordBuffer;
		std::vector<dxm::vec3> normalBuffer;

		FILE* file = nullptr;
		Utils::RuntimeCheck(_wfopen_s(&file, path, L"r") == 0);

		/* Reading the file.. */
		for (char lineHeader[16] = {}; fscanf_s(file, "%s", lineHeader, dxm::countof(lineHeader)) != EOF;)
		{
			/* Reading first word of the line.. */
			if (lineHeader[0] == 'v' && lineHeader[1] == '\0')
			{
				/* .. It is a vertex coord. */
				dxm::vec3 vertex;
				Utils::RuntimeCheck(fscanf_s(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z) == 3);
				vertexBuffer.push_back(vertex);
			}
			else if (lineHeader[0] == 'v' && lineHeader[1] == 'n' && lineHeader[2] == '\0')
			{
				/* .. It is a vertex normal. */
				dxm::vec3 normal;
				Utils::RuntimeCheck(fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z) == 3);
				normalBuffer.push_back(normal);
			}
			else if (lineHeader[0] == 'v' && lineHeader[1] == 't' && lineHeader[2] == '\0')
			{
				/* .. It is a texture coord. */
				dxm::dvec3 texCoord;
				Utils::RuntimeCheck(fscanf_s(file, "%lf %lf %lf\n", &texCoord.x, &texCoord.y, &texCoord.z) == 3);
				texCoordBuffer.push_back(glm::vec2(texCoord.x, 1 - texCoord.y));
			}
			else if (lineHeader[0] == 'f' && lineHeader[1] == '\0')
			{
				/* .. It is a face. */
				vertexIndices.resize(vertexIndices.size() + 3);
				texCoordIndices.resize(texCoordIndices.size() + 3);
				normalIndices.resize(normalIndices.size() + 3);
				Utils::RuntimeCheck(fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n"
					, &vertexIndices[vertexIndices.size() - 3], &texCoordIndices[texCoordIndices.size() - 3], &normalIndices[normalIndices.size() - 3]
					, &vertexIndices[vertexIndices.size() - 2], &texCoordIndices[texCoordIndices.size() - 2], &normalIndices[normalIndices.size() - 2]
					, &vertexIndices[vertexIndices.size() - 1], &texCoordIndices[texCoordIndices.size() - 1], &normalIndices[normalIndices.size() - 1]) == 9);
			}
			else
			{
				/* .. It is something strange, just skipping. */
				char static restOfTheLine[256] = {};
				fgets(restOfTheLine, dxm::countof(restOfTheLine), file);
			}
		}

		_fclose_nolock(file);

		/* Unwrapping internal OBJ format.. */
		std::vector<Vertex> vertices;
		for (auto cnt = 0u; cnt < vertexIndices.size(); ++cnt)
		{
			auto const& vertex = vertexBuffer[vertexIndices[cnt] - 1];
			auto const& texCoord = texCoordBuffer[texCoordIndices[cnt] - 1];
			auto const& normal = normalBuffer[normalIndices[cnt] - 1];

			vertices.push_back({ UniversalCtor, vertex, normal, color, texCoord });
		}
		this->UpdateVertexBuffer(LoadFromMemory, vertices.data(), vertices.size());
	}
#endif	// if SUPPORT_LOADING_FROM_FILE

	// -----------------------
	template<typename TVertex, D3DPRIMITIVETYPE TPrimitiveType>
	ADINT void Mesh<TVertex, TPrimitiveType>::UpdateVertexBuffer(LoadFromMemory_t, Vertex const* const vertices, UINT const verticesCount) const
	{
		auto const verticesByteSize = verticesCount * sizeof(Vertex);
		if (m_VertexBuffer != nullptr)
		{
			/* Buffer already exists, but we can upload new data only if
			 * buffer size is equal or large than data size. */
			D3DVERTEXBUFFER_DESC vertexBufferDesc = {};
			Utils::RuntimeCheckH(m_VertexBuffer->GetDesc(&vertexBufferDesc));
			if (vertexBufferDesc.Size < verticesByteSize)
			{
				m_VertexBuffer->Release();
				m_VertexBuffer = nullptr;
			}
		}
		m_PrimitivesCount = verticesCount / (PrimitiveType == D3DPT_TRIANGLELIST ? 3 : 2);
		if (verticesCount != 0)
		{
			if (m_VertexBuffer == nullptr)
			{
				/* Buffer does not exist. We need to create the new of the
				 * corresponding size. */
				Utils::RuntimeCheckH(m_Device->CreateVertexBuffer(verticesByteSize, 0, Vertex::FVF, D3DPOOL_MANAGED, &m_VertexBuffer, nullptr));
			}
			assert(vertices != nullptr);
			assert(m_VertexBuffer != nullptr);

			void* verticesGpuData = nullptr;
			Utils::RuntimeCheckH(m_VertexBuffer->Lock(0, 0, &verticesGpuData, 0));
			::memcpy_s(verticesGpuData, verticesByteSize, vertices, verticesByteSize);
			Utils::RuntimeCheckH(m_VertexBuffer->Unlock());
		}
	}

	// -----------------------
	template<typename TMesh, BOOL TIsTransparent, BOOL TIsLit>
	ADAPI MeshRenderer<TMesh, TIsTransparent, TIsLit>::MeshRenderer(IDirect3DDevice9* const device, MeshPtr const& mesh)
		: m_Device(device), m_Texture(nullptr), m_Mesh(mesh)
	{
	}

	// -----------------------
#if SUPPORT_LOADING_FROM_FILE
	template<typename TMesh, BOOL TIsTransparent, BOOL TIsLit>
	ADAPI MeshRenderer<TMesh, TIsTransparent, TIsLit>::MeshRenderer(IDirect3DDevice9* const device, MeshPtr const& mesh, LPCWSTR const path)
		: MeshRenderer(device, mesh)
	{
		assert(path != nullptr);

#if !_DEBUG
		OutputDebugStringA("Loading a texture from file in a release build.");
#endif	// if !_DEBUG

		Utils::RuntimeCheckH(D3DXCreateTextureFromFileW(m_Device, path, &m_Texture));
	}
#endif	// if SUPPORT_LOADING_FROM_FILE
	
	// -----------------------
	template<typename TMesh, BOOL TIsTransparent, BOOL TIsLit>
	ADAPI MeshRenderer<TMesh, TIsTransparent, TIsLit>::MeshRenderer(IDirect3DDevice9* const device, MeshPtr const& mesh, LoadFromMemory_t, void const* const data, UINT const width, UINT const height)
		: MeshRenderer(device, mesh)
	{
		assert(data != nullptr);
		assert(width != 0 && height != 0);

		/* Creating sized texture.. */
		Utils::RuntimeCheckH(device->CreateTexture(width, height, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_Texture, nullptr));

		/* Uploading texture information to the GPU.. */
		IDirect3DSurface9* surface = nullptr;
		Utils::RuntimeCheckH(m_Texture->GetSurfaceLevel(0, &surface));
		D3DLOCKED_RECT surfaceRect = {};
		Utils::RuntimeCheckH(surface->LockRect(&surfaceRect, nullptr, D3DLOCK_DISCARD));
		auto const textureSize = width * height * sizeof(dxm::argb);
		::memcpy_s(surfaceRect.pBits, textureSize, data, textureSize);
		Utils::RuntimeCheckH(surface->UnlockRect());
		surface->Release();

		m_Texture->GenerateMipSubLevels();
	}

	// -----------------------
	template<typename TMesh, BOOL TIsTransparent, BOOL TIsLit>
	ADAPI void MeshRenderer<TMesh, TIsTransparent, TIsLit>::Render() const
	{
		/* Setting up the transformations. */
		auto const matrix = dxm::translate(Position)
			* dxm::toMat4(dxm::quat(Rotation))
			* dxm::translate(PositionOffset)
			* dxm::scale(Scale);
		Utils::RuntimeCheckH(m_Device->SetTransform(D3DTS_WORLD, dxm::ptr(matrix)));

		/* Setting up the transparency. */
		Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TIsTransparent));

		/* Setting up the lights (we have a signle light source on the scene). */
		Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_LIGHTING, TIsLit));
		Utils::RuntimeCheckH(m_Device->LightEnable(0, TIsLit));

		/* Setting up the textures (we have a signle texture per object). */
		Utils::RuntimeCheckH(m_Device->SetTexture(0, m_Texture));

		/* Setting up and rendering the mesh data. */
		Utils::RuntimeCheckH(m_Device->SetFVF(FVF));
		Utils::RuntimeCheckH(m_Device->SetStreamSource(0, m_Mesh->GetVertexBuffer(), 0, sizeof(Vertex)));
		Utils::RuntimeCheckH(m_Device->DrawPrimitive(PrimitiveType, 0, m_Mesh->GetPrimitivesCount()));
	}

}	// namespace Presentation2