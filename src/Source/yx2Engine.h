#pragma once

#include "Window.h"
#include "yx2Framework.h"

#include <type_traits>
#include <vector>

#define _USE_MATH_DEFINES 1
#define GLM_FORCE_LEFT_HANDED 1
#include <math.h>

#pragma warning(push, 0)
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>
#pragma warning(pop)

#define YX2_PI float(M_PI)

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

		YX2_API static float clampf(float value, float min, float max)
		{
			return value < min ? min : value > max ? max : value;
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

			/**
			 * \brief Initializes a mesh renderer.
			 * \param[in] runtime Engine runtime instance.
			 * \param[in] mesh The mesh instance.
			 */
			YX2_API BasicMeshRenderer(Runtime const& runtime, Mesh const& mesh)
				: m_Device(runtime.m_device), m_Mesh(mesh)
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

		// ***********************************************************************************************
		// Camera management.
		// ***********************************************************************************************

		/** 
		 * \brief A basic static camera.
		 */
		class Camera : public NonCopyable
		{
		protected:
			auto static const g_Width = 1280;
			auto static const g_Height = 720;

		private:
			IDirect3DDevice9* m_Device;
			mat4 m_ViewMatrix;
			mat4 m_ProjectionMatrix;

		public:
			vec3 Eye = vec3(0.0f, 0.0f, -3.0f);
			vec3 Center = vec3(0.0f, 0.0f, 0.0f);
			vec3 Up = vec3(0.0f, 1.0f, 0.0f);

		public:

			/**
			 * \brief Initializes a camera.
			 * \param[in] runtime Engine runtime instance.
			 */
			YX2_API explicit Camera(Runtime const& runtime) : m_Device(runtime.m_device) { Update(); }

			/** 
			 * \brief Updates camera matrices.
			 */
			YX2_API void Update() const
			{
				m_Device->SetTransform(D3DTS_VIEW, ToD3D(glm::lookAtLH(Eye, Center, Up)));
				m_Device->SetTransform(D3DTS_PROJECTION,
									   ToD3D(glm::perspectiveFovLH<float>(YX2_PI / 4.0f, g_Width, g_Height, 0.01f, 100.0f)));
			}

		}; // class Camera

		/** 
		 * \brief A mouse-control orbital camera with zoom support.
		 */
		class OrbitalCamera final : public Camera
		{
		private:
			float m_CameraRotationYaw = -YX2_PI / 2.0f;
			float m_CameraRotationPitch = 0.0f;
			POINT m_PrevMousePosition = {};

		public:

			/**
			 * \brief Initializes an orbital camera.
			 * \param[in] runtime Engine runtime instance.
			 */
			YX2_API explicit OrbitalCamera(Runtime const& runtime) : Camera(runtime) {}

			/** 
			 * \brief Updates camera matrices.
			 */
			YX2_API void Update()
			{
				/// @todo Move initial values outside somewhere.
				vec4 cameraRotationCenter(0.0f, 1.24f, 1.5f, 1.0f);
				vec4 cameraCenterOffset(0.0f, 0.0f, -3.0f, 1.0f);
				vec4 cameraUp(0.0f, 1.0f, 0.0f, 1.0f);

				if (GetAsyncKeyState(VK_LBUTTON) != 0)
				{
					// L button is pressed.
					POINT mouseCurrentPosition = {};
					GetCursorPos(&mouseCurrentPosition);

					auto const deltaYaw = static_cast<float>(mouseCurrentPosition.y - m_PrevMousePosition.y) / g_Height;
					auto const deltaPitch = static_cast<float>(mouseCurrentPosition.x - m_PrevMousePosition.x) / g_Width;

					m_CameraRotationYaw += deltaPitch;
					m_CameraRotationPitch = clampf(m_CameraRotationPitch + deltaYaw, -YX2_PI / 12.0f, YX2_PI / 3.0f);
				}
				GetCursorPos(&m_PrevMousePosition);

				auto const cameraTranslation = glm::translate(glm::vec3(cameraRotationCenter));
				auto const cameraRotation = glm::yawPitchRoll(m_CameraRotationYaw, m_CameraRotationPitch, 0.0f);
				cameraCenterOffset = cameraTranslation * cameraRotation * cameraCenterOffset;
				cameraUp = cameraRotation * cameraUp;

				Eye = vec3(cameraCenterOffset);
				Center = vec3(cameraRotationCenter);
				Up = vec3(cameraUp);
				Camera::Update();
			}

		};	// class OrbitalCamera

	} // namespace engine
} // namespace yx2
