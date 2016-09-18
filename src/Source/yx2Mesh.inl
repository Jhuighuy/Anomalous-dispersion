// Copyright (C) 2016 Butakov Oleg, Plaxin Gleb.
// Computational Methods @ Computational Maathematics & Cybernetics, MSU.

#pragma once

namespace yx2
{
namespace engine
{
	/**
	 * \brief Initializes a mesh.
	 * \param[in] runtime Engine runtime instance.
	 * \param[in] vertices Mesh vertices.
	 * \param[in] verticesCount Mesh vertices count.
	 */
	// ***********************************************************************************************
	template<typename TVertex>
	YX2_API BasicMesh<TVertex>::BasicMesh(Runtime* const runtime, Vertex const* const vertices, DWORD const verticesCount)
		: RuntimeReferencee(runtime)
		, m_VertexBufferIsDirty(true), m_VertexBuffer(nullptr), m_Vertices(verticesCount, verticesCount + verticesCount)
	{
		GetVertexBuffer();
	}

	/**
	 * \brief Destroyes the mesh object.
	 */
	// ***********************************************************************************************
	template <typename TVertex>
	YX2_API BasicMesh<TVertex>::~BasicMesh()
	{
		m_VertexBuffer->Release();
		m_VertexBuffer = nullptr;
	}

	// ***********************************************************************************************
	template <typename TVertex>
	YX2_API auto BasicMesh<TVertex>::GetVertexBuffer() const
	{
		if (m_VertexBufferIsDirty)
		{
			/* It seems that something has modified the vertex buffer.
			 * We need to reupload everything back to the GPU. */
			UINT const dataSize = GetVerticesCount() * sizeof(Vertex);
			if (dataSize == 0)
			{
				throw std::runtime_error("Vertices data is empty.");
			}

			if (m_VertexBuffer != nullptr)
			{
				/* Buffer already exists, but we can upload new data only if
				 * buffer size is equal or large than data size. */
				D3DVERTEXBUFFER_DESC vertexBufferDesc = {};
				if (FAILED(m_VertexBuffer->GetDesc(&vertexBufferDesc))
					|| vertexBufferDesc.Size < dataSize)
				{
					m_VertexBuffer->Release();
					m_VertexBuffer = nullptr;
				}
			}

			if (m_VertexBuffer == nullptr)
			{
				/* Buffer does not exist. We need to create the new of the 
				 * corresponding size. */
				m_Device->CreateVertexBuffer(dataSize, 0, FVF, D3DPOOL_MANAGED, m_VertexBuffer, nullptr);
				if (m_VertexBuffer == nullptr)
				{
					throw std::runtime_error("Failed to create a vertex buffer.");
				}
			}

			assert(m_VertexBuffer != nullptr);

			/* And finally uploading the data. */
			void* data = nullptr;
			if (FAILED(m_VertexBuffer->Lock(0, 0, &data, 0)))
			{
				throw std::runtime_error("Failed to lock a vertex buffer.");
			}
			memcpy_s(data, dataSize, m_Vertices.data(), dataSize);
			if (FAILED(m_VertexBuffer->Unlock()))
			{
				throw std::runtime_error("Failed to unlock a vertex buffer.");
			}

			m_VertexBufferIsDirty = false;
		}
		return m_VertexBuffer;
	}

	template <typename TVertex>
	YX2_API auto BasicMesh<TVertex>::GetVertices() const
	{
		return m_Vertices.data();
	}

	template <typename TVertex>
	YX2_API auto BasicMesh<TVertex>::GetVerticesCount() const
	{
		return m_Vertices.size();
	}

	template <typename TVertex>
	YX2_API auto const& BasicMesh<TVertex>::GetVertex(size_t const at) const
	{
		return m_Vertices[at];
	}

	template <typename TVertex>
	YX2_API void BasicMesh<TVertex>::SetVertex(size_t const at, Vertex const& vertex) const
	{
		m_Vertices[at] = vertex;
		m_VertexBufferIsDirty = true;
	}

} // namespace engine
} // namespace yx2
