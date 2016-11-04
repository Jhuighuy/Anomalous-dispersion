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

namespace Presentation2
{
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //
	// Mesh setup.
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //

	// -----------------------
	template<typename TVertex, D3DPRIMITIVETYPE TPrimitiveType>
	ADINT void Mesh<TVertex, TPrimitiveType>::Update(Vertex const* const vertices, UINT const verticesCount) const
	{
		assert(!((vertices == nullptr) ^ (verticesCount == 0)));

		auto const verticesSize = verticesCount * sizeof(Vertex);
		if (m_VertexBuffer != nullptr)
		{
			/* Buffer already exists, but we can upload new data only if
			* buffer size is equal or large than data size. */
			D3DVERTEXBUFFER_DESC vertexBufferDesc = {};
			Utils::RuntimeCheckH(m_VertexBuffer->GetDesc(&vertexBufferDesc));
			if (vertexBufferDesc.Size < verticesSize)
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
				Utils::RuntimeCheckH(m_Device->CreateVertexBuffer(verticesSize, 0, Vertex::FVF, D3DPOOL_MANAGED, &m_VertexBuffer, nullptr));
			}
			assert(vertices != nullptr);
			assert(m_VertexBuffer != nullptr);

			void* verticesGpuData = nullptr;
			Utils::RuntimeCheckH(m_VertexBuffer->Lock(0, 0, &verticesGpuData, 0));
			::memcpy_s(verticesGpuData, verticesSize, vertices, verticesSize);
			Utils::RuntimeCheckH(m_VertexBuffer->Unlock());
		}
	}

	// -----------------------
	template<typename TVertex, D3DPRIMITIVETYPE TPrimitiveType>
	ADAPI void MutableMesh<TVertex, TPrimitiveType>::Render() const
	{
		if (m_Lock.try_lock())
		{
			/* Nothing locks the vertex buffer, we can safely render. */
			Utils::RuntimeCheckH(this->m_Device->SetFVF(Base::FVF));
			Utils::RuntimeCheckH(this->m_Device->SetStreamSource(0, GetVertexBuffer(), 0, sizeof(Vertex)));
			Utils::RuntimeCheckH(this->m_Device->DrawPrimitive(PrimitiveType, 0, GetPrimitivesCount()));
			m_Lock.unlock();
		}
	}

	// -----------------------
	template<typename TMesh, BOOL TIsTransparent, BOOL TIsLit>
	ADAPI void MeshRenderer<TMesh, TIsTransparent, TIsLit>::Render() const
	{
		/* Setting up the transformations. */
		auto const matrix = dxm::translate(Position)
			* dxm::toMat4(dxm::quat(dxm::vec3(Rotation.x, Rotation.y, Rotation.z)))
			* dxm::scale(Scale);
		Utils::RuntimeCheckH(m_Device->SetTransform(D3DTS_WORLD, dxm::ptr(matrix)));

		/* Setting up the transparency. */
		Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TIsTransparent));

		/* Setting up the lights (we have a signle light source on the scene). */
		Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_LIGHTING, TIsLit));
		Utils::RuntimeCheckH(m_Device->LightEnable(0, TIsLit));

		/* Setting up the textures and shaders (we have a signle texture and shared per object). */
		Utils::RuntimeCheckH(m_Device->SetTexture(0, Texture));
		Utils::RuntimeCheckH(m_Device->SetPixelShader(PixelShader));

		/* Renderin the mesh data. */
		m_Mesh->Render();
	}

}	// namespace Presentation2
