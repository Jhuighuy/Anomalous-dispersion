// Copyright (C) 2016 Butakov Oleg, Plaxin Gleb.
// Computational Methods @ Computational Maathematics & Cybernetics, MSU.

#pragma once
#include "yx2Engine.h"

namespace yx2
{
namespace engine
{
	// ***********************************************************************************************
	// Meshes management.
	// ***********************************************************************************************

	namespace fvfs
	{
		template <DWORD TFVF, D3DPRIMITIVETYPE TPrimitiveType>
		struct BasicVertex
		{
			static auto const FVF = TFVF;
			static auto const PrimitiveType = TPrimitiveType;
		}; // struct BasicVertex

		struct LineVertex final : public fvfs::BasicVertex<D3DFVF_XYZ | D3DFVF_DIFFUSE, D3DPT_LINELIST>
		{
			vec3 XYZ;
			color32 Diffuse;
		}; // struct LineVertex

		struct TriangleVertex final
			: public fvfs::BasicVertex<D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE, D3DPT_TRIANGLELIST>
		{
			vec3 XYZ;
			vec3 Normal;
			color32 Diffuse;
		}; // struct TriangleVertex

	}	  // namespace fvfs

	using fvfs::LineVertex;
	using fvfs::TriangleVertex;

	// ***********************************************************************************************
	// BasicMesh<T> class.
	// ***********************************************************************************************

	/**
	 * \brief Template container for vertices.
	 * \tparam TVertex Type of a vertex.
	 */
	template <typename TVertex>
	class YX2_API BasicMesh final : public RuntimeReferencee
	{
	public:
		using Vertex = TVertex;
		static auto const FVF = TVertex::FVF;
		static auto const PrimitiveType = TVertex::PrimitiveType;
		static auto const VerticesPerPrimitive = PrimitiveType == D3DPT_TRIANGLELIST ? 3 : 2;

	private:
		mutable bool m_VertexBufferIsDirty;
		mutable IDirect3DVertexBuffer9* m_VertexBuffer;
		std::vector<Vertex> m_Vertices;

	public:
		YX2_API BasicMesh(Runtime* const runtime, Vertex const* const vertices, DWORD const verticesCount);
		YX2_API ~BasicMesh();

		YX2_API auto GetVertexBuffer() const;

		YX2_API auto GetVertices() const;
		YX2_API auto GetVerticesCount() const;
			
		YX2_API auto const& GetVertex(size_t const at) const;
		YX2_API void SetVertex(size_t const at, Vertex const& vertex) const;

	}; // class BasicMesh
		
	using LineMesh = BasicMesh<LineVertex>;
	using TriangleMesh = BasicMesh<TriangleVertex>;

	// ***********************************************************************************************
	// Meshes management.
	// ***********************************************************************************************

	/**
	* \brief Implements mesh rendering.
	* \tparam TMesh Type of a renderable mesh.
	*/
	template <typename TMesh>
	class BasicMeshRenderer : public NonCopyable
	{
	public:
		using Mesh = TMesh;
		using Vertex = typename Mesh::Vertex;
		static auto const FVF = Mesh::FVF;
		static auto const PrimitiveType = Mesh::TPrimitiveType;

	private:
		IDirect3DDevice9* m_Device;
		Mesh const& m_Mesh;

	public:
		vec3 Position = vec3(0.0f, 0.0f, 0.0f);
		vec3 Rotation = vec3(0.0f, 0.0f, 0.0f);
		vec3 Scale = vec3(1.0f, 1.0f, 1.0f);
		bool IsLit = true;
		bool IsOpacue = true;

	public:
		/**
		* \brief Initializes a mesh renderer.
		* \param[in] runtime Engine runtime instance.
		* \param[in] mesh The mesh instance.
		*/
		YX2_API BasicMeshRenderer(Runtime const* runtime, Mesh const& mesh)
			: m_Device(runtime->m_Device), m_Mesh(mesh)
		{
		}

		/**
		* \brief Performs rendering.
		*/
		YX2_API void Render() const
		{
			// Setting up the transformation matrix.
			auto const transformationMatrix = glm::translate(Position) *
												glm::yawPitchRoll(Rotation.x, Rotation.y, Rotation.z) *
												glm::scale(Scale);
			m_Device->SetTransform(D3DTS_WORLD, ToD3D(transformationMatrix));

			// And finally rendering.
			m_Device->SetFVF(FVF);
			m_Device->SetStreamSource(0, m_Mesh.m_VertexBuffer, 0, sizeof(Vertex));
			m_Device->DrawPrimitive(PrimitiveType, 0, m_Mesh.m_PrimitivesCount);

			m_Device->SetTransform(D3DTS_WORLD, ToD3D(mat4()));
		}

	}; // class BasicMeshRenderer

	using LineMeshRenderer = BasicMeshRenderer<LineMesh>;
	using TriangleMeshRenderer = BasicMeshRenderer<TriangleMesh>;

} // namespace engine
} // namespace yx2

#include "yx2Mesh.inl"
