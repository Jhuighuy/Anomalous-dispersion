#pragma once

#include "Window.h"
#include "yx2Framework.h"

#include <type_traits>
#include <vector>

#pragma warning(push, 0)
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>
#pragma warning(pop)

namespace yx2
{
	using glm::vec3;
	using glm::vec4;
	using glm::mat4;

	using color = D3DCOLOR;
	auto static const color_black = D3DCOLOR_RGBA(0x00, 0x00, 0x00, 0xFF);
	auto static const color_white = D3DCOLOR_RGBA(0xFF, 0xFF, 0xFF, 0xFF);

	namespace engine
	{
		/**
		 * \brief Extracts a Direct3D matrix pointer from GLM matrix.
		 */
		YX2_API static D3DMATRIX const* ToD3D(mat4 const& matrix)
		{
			return reinterpret_cast<D3DMATRIX const*>(&matrix[0][0]);
		}

		// ***********************************************************************************************
		// Runtime management.
		// ***********************************************************************************************

		class Runtime : public framework::D3DWidget
		{
		}; // class Runtime

		// ***********************************************************************************************
		// Meshes management.
		// ***********************************************************************************************

		namespace fvfs
		{
			template <DWORD TFVF, D3DPRIMITIVETYPE TPrimitiveType>
			struct _BasicVertex
			{
				static auto const FVF = TFVF;
				static auto const PrimitiveType = TPrimitiveType;
			}; // struct _BaseVertex

			struct LineVertex final : public fvfs::_BasicVertex<D3DFVF_XYZ | D3DFVF_DIFFUSE, D3DPT_LINELIST>
			{
			}; // struct LineVertex

			struct TriangleVertex final
				: public fvfs::_BasicVertex<D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE, D3DPT_TRIANGLELIST>
			{
			}; // struct TriangleVertex
		}	  // namespace fvfs

		using fvfs::LineVertex;
		using fvfs::TriangleVertex;

		/**
		 * \brief Container for vertices.
		 * \tparam TVertex Type of a vertex.
		 */
		template <typename TVertex>
		class YX2_API BasicMesh final : public NonCopyable
		{
		public:
			using Vertex = TVertex;
			static auto const FVF = TVertex::FVF;
			static auto const PrimitiveType = TVertex::PrimitiveType;
			static auto const VerticesPerPrimitive = PrimitiveType == D3DPT_TRIANGLELIST ? 3 : 2;

		private:
			IDirect3DDevice9* m_Device = nullptr;
			IDirect3DVertexBuffer9* m_VertexBuffer = nullptr;
			DWORD m_PrimitivesCount = 0;

		public:
			/**
			 * \brief Initializes a mesh.
			 * \param[in] runtime Engine runtime instance.
			 * \param[in] vertices Mesh vertices.
			 */
			YX2_API BasicMesh(Runtime const& runtime, Vertex const* const vertices, DWORD const verticesCount)
				: m_Device(runtime.m_device), m_VertexBuffer(nullptr),
				  m_PrimitivesCount(verticesCount / VerticesPerPrimitive)
			{
				assert(vertices != nullptr);
				assert(verticesCount != 0);
				assert(verticesCount % VerticesPerPrimitive == 0);

				void* bufferData = nullptr;
				auto const bufferSize = verticesCount * sizeof(Vertex);
				m_Device->CreateVertexBuffer(bufferSize, 0, FVF, D3DPOOL_MANAGED, &m_VertexBuffer, nullptr);
				m_VertexBuffer->Lock(0, 0, &bufferData, 0);
				::memcpy_s(bufferData, bufferSize, vertices, bufferSize);
				m_VertexBuffer->Unlock();
			}

			/**
			 * \brief Destroyes the mesh object.
			 */
			YX2_API ~BasicMesh()
			{
				m_VertexBuffer->Release();
				m_VertexBuffer = nullptr;
			}

			/**
			 * \returns Amount of the primitives in this mesh.
			 */
			YX2_API auto GetPrimitivesCount() const { return m_PrimitivesCount; }

			/**
			 * \returns Vertex buffer that contains this mesh.
			 */
			YX2_API auto GetVertexBuffer() const { return m_VertexBuffer; }

		}; // class BasicMesh

		using LineMesh = BasicMesh<LineVertex>;
		using TriangleMesh = BasicMesh<TriangleVertex>;

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
			YX2_API BasicMeshRenderer(Runtime const& runtime, Mesh const& mesh)
				: m_Device(runtime.m_device), m_Mesh(mesh)
			{
			}

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

		// ***********************************************************************************************
		// Camera management.
		// ***********************************************************************************************

		class Camera : public NonCopyable
		{
		private:
			IDirect3DDevice9* m_Device;
			mat4 m_ViewMatrix;
			mat4 m_ProjectionMatrix;

		public:
			YX2_API explicit Camera(Runtime const& runtime) : m_Device(runtime.m_device), {}

		}; // class Camera

	} // namespace engine
} // namespace yx2
