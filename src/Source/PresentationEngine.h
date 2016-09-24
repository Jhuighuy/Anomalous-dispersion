// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// Copyright (C) 2016 Butakov Oleg, Plaxin Gleb.
// Computational Methods @ Computational Mathematics & Cybernetics, MSU.
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
#pragma once
#include "yx2Engine.h"
#include "Presentation.h"

// ReSharper disable CppRedundantQualifier
#include <d3d9.h>
#include <d3dx9.h>
#pragma warning(push, 0)
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>
#pragma warning(pop)

namespace Presentation1
{
	/** A small naming fix for the OpenGL Mathematics library. */
	namespace dxm
	{
		using namespace glm;
		using argb = ::D3DCOLOR;

		static D3DMATRIX const* ptr(mat4x4 const& matrix)
		{
			return reinterpret_cast<D3DMATRIX const*>(&matrix[0][0]);
		}

		template<typename T>
		static T clamp(T const value, T const min, T const max)
		{
			return value < min ? min : value > max ? max : value;
		}
	}	// namespace dxm

	static void ThrowIfFailed(HRESULT const result)
	{
		if (FAILED(result))
		{
			throw 0;
		}
	}

	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //
	// Camera setup.
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //

	/** \brief Describes an orbital mouse-controlled camera. */
	struct OrbitalCamera final
	{
	private:
		IDirect3DDevice9* const m_Device;
		FLOAT const m_Width;
		FLOAT const m_Height;
		FLOAT m_CameraRotationYaw = 0.0f;	/// @todo Calibrate value here.
		FLOAT m_CameraRotationPitch = 0.0f;
		POINT m_PrevMousePosition = {};
		dxm::mat4 m_ProjectionMatrix;
		dxm::mat4 m_ViewMatrix;
	public:
		dxm::vec3 const RotationCenter = { 0.0f, 0.7f, 2.0f };	/// @todo And here.
		dxm::vec3 const CenterOffset = { 0.0f, 0.0f, -1.8f };
		dxm::vec3 const Up = { 0.0f, 1.0f, 0.0f };

		// -----------------------
		explicit OrbitalCamera(IDirect3DDevice9* const device, UINT const width = 1280, UINT const height = 720)
			: m_Device(device), m_Width(static_cast<FLOAT>(width)), m_Height(static_cast<FLOAT>(height))
		{
			m_ProjectionMatrix = dxm::perspectiveFovLH(DXM_PI / 3.0f, m_Width, m_Height, 0.01f, 100.0f);
			m_ViewMatrix = dxm::lookAtLH(RotationCenter + CenterOffset, RotationCenter, Up);
		}

		// -----------------------
		void Update(UINT const mouseWheelDelta = 0)
		{
			// Scaling is not implemented.
			assert(mouseWheelDelta == 0);
			(void)mouseWheelDelta;

			if (GetAsyncKeyState(VK_LBUTTON) != 0)
			{
				POINT mouseCurrentPosition = {};
				GetCursorPos(&mouseCurrentPosition);

				auto const deltaYaw = static_cast<float>(mouseCurrentPosition.y - m_PrevMousePosition.y) / m_Height;
				auto const deltaPitch = static_cast<float>(mouseCurrentPosition.x - m_PrevMousePosition.x) / m_Width;

				m_CameraRotationYaw += deltaPitch;
				m_CameraRotationPitch = dxm::clamp(m_CameraRotationPitch + deltaYaw, -DXM_PI / 12.0f, DXM_PI / 7.5f);

				auto const translation = dxm::translate(glm::vec3(RotationCenter));
				auto const cameraRotation = dxm::yawPitchRoll(m_CameraRotationYaw, m_CameraRotationPitch, 0.0f);
				m_ViewMatrix = dxm::lookAtLH(dxm::vec3(translation * cameraRotation * dxm::vec4(CenterOffset, 1.0f))
					, RotationCenter, dxm::vec3(cameraRotation * dxm::vec4(Up, 1.0f)));
			}
			GetCursorPos(&m_PrevMousePosition);

			ThrowIfFailed(m_Device->SetTransform(D3DTS_VIEW, dxm::ptr(m_ViewMatrix)));
			ThrowIfFailed(m_Device->SetTransform(D3DTS_PROJECTION, dxm::ptr(m_ProjectionMatrix)));
		}
	};	// struct OrbitalCamera

	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //
	// Mesh setup.
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //

	/** \brief Describes a single colored line vertex. */
	struct LineVertex final
	{
		DWORD static const FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;
		dxm::vec3 const Position;
		dxm::argb const DiffuseColor;

		// -----------------------
		LineVertex(dxm::vec3 const& position, dxm::argb const diffuseColor)
			: Position(position), DiffuseColor(diffuseColor)
		{}
	};	// struct LineVertex

	/** \brief Describes a single colored textured triangle vertex with a normal. */
	struct TriangleVertex final
	{
		DWORD static const FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1;
		dxm::vec3 const Position;
		dxm::vec3 const Normal;
		dxm::argb const DiffuseColor;
		dxm::vec2 const TexCoord;

		// -----------------------
		TriangleVertex(dxm::vec3 const& position, dxm::vec3 const& normal, dxm::argb const diffuseColor, dxm::vec2 const& texCoord)
			: Position(position), Normal(normal), DiffuseColor(diffuseColor), TexCoord(texCoord)
		{}
	};	// struct TriangleVertex

	/** \brief Describes a template immutable mesh. */
	template<typename TVertex, D3DPRIMITIVETYPE TPrimitiveType>
	struct Mesh
	{
		typedef TVertex Vertex;
		D3DPRIMITIVETYPE static const PrimitiveType = TPrimitiveType;
		DWORD static const FVF = Vertex::FVF;

	protected:
		IDirect3DDevice9* const m_Device;
	private:
		IDirect3DVertexBuffer9 mutable* m_VertexBuffer = nullptr;
		UINT mutable m_PrimitivesCount = 0;

	protected:
		void Update(Vertex const* const vertices, UINT const verticesCount) const
		{
			assert(vertices != nullptr);
			assert(verticesCount != 0);

			auto const verticesSize = verticesCount * sizeof(Vertex);
			if (m_VertexBuffer != nullptr)
			{
				/* Buffer already exists, but we can upload new data only if
				 * buffer size is equal or large than data size. */
				D3DVERTEXBUFFER_DESC vertexBufferDesc = {};
				ThrowIfFailed(m_VertexBuffer->GetDesc(&vertexBufferDesc));
				if (vertexBufferDesc.Size < verticesSize)
				{
					m_VertexBuffer->Release();
					m_VertexBuffer = nullptr;
				}
			}
			if (m_VertexBuffer == nullptr)
			{
				/* Buffer does not exist. We need to create the new of the
				 * corresponding size. */
				ThrowIfFailed(m_Device->CreateVertexBuffer(verticesSize, 0, Vertex::FVF, D3DPOOL_MANAGED, &m_VertexBuffer, nullptr));
			}
			assert(m_VertexBuffer != nullptr);

			void* verticesGpuData = nullptr;
			ThrowIfFailed(m_VertexBuffer->Lock(0, 0, &verticesGpuData, 0));
			memcpy_s(verticesGpuData, verticesSize, vertices, verticesSize);
			ThrowIfFailed(m_VertexBuffer->Unlock());
			m_PrimitivesCount = verticesCount / (PrimitiveType == D3DPT_TRIANGLELIST ? 3 : 2);
		}

	public:
		Mesh(Mesh&&) = delete;
		Mesh(Mesh const&) = delete;
		Mesh& operator= (Mesh const&) = delete;

		// -----------------------
		explicit Mesh(IDirect3DDevice9* const device) : m_Device(device) {}
		Mesh(IDirect3DDevice9* const device, Vertex const* const vertices, UINT const verticesCount) : m_Device(device)
		{
			this->Update(device, vertices, verticesCount);
		}
		~Mesh()
		{
			if (m_VertexBuffer != nullptr)
			{
				m_VertexBuffer->Release();
			}
		}

		// -----------------------
		auto GetVertexBuffer() const
		{
			return m_VertexBuffer;
		}
		auto GetPrimitivesCount() const
		{
			return m_PrimitivesCount;
		}
	};	// struct Mesh

	using TriangleMesh = Mesh<TriangleVertex, D3DPT_TRIANGLELIST>;
	using LineMesh = Mesh<LineVertex, D3DPT_LINELIST>;

	/** \brief Describes a template mutable mesh. */
	template<typename TVertex = TriangleVertex, D3DPRIMITIVETYPE TPrimitiveType = D3DPT_TRIANGLELIST>
	struct MutableMesh final : public Mesh<TVertex, TPrimitiveType>
	{
		D3DPRIMITIVETYPE static const PrimitiveType = TPrimitiveType;
		typedef TVertex Vertex;
		typedef Mesh<TVertex, TPrimitiveType> Base;

	private:
		std::vector<Vertex> m_VertexAccumulator;
		bool mutable m_IsSynced = false;

	protected:
		void Update() const
		{
			Base::Update(m_VertexAccumulator.data(), m_VertexAccumulator.size());
		}

	public:
		explicit MutableMesh(IDirect3DDevice9* const device) : Mesh(device) {}

		// -----------------------
		void AddVertex(Vertex const& vertex)
		{
			m_VertexAccumulator.push_back(vertex);
			m_IsSynced = false;
		}
		void ClearVertices()
		{
			m_VertexAccumulator.resize(0);
			m_IsSynced = false;
		}

		// -----------------------
		auto GetVertexBuffer() const
		{
			if (!m_IsSynced)
			{
				Update();
				m_IsSynced = true;
			}
			return Base::GetVertexBuffer();
		}
		auto GetPrimitivesCount() const
		{
			if (!m_IsSynced)
			{
				Update();
				m_IsSynced = true;
			}
			return Base::GetPrimitivesCount();
		}
	};	// struct MutableMesh

	using TriangleMutableMesh = MutableMesh<TriangleVertex, D3DPT_TRIANGLELIST>;
	using LineMutableMesh = MutableMesh<LineVertex, D3DPT_LINELIST>;

	/// @todo Load from raw data here.
	bool LoadOBJ(const char* path, TriangleMutableMesh& mesh);

	/** \brief Describes a template mesh renderer. */
	template<typename TMesh = TriangleMutableMesh, BOOL TIsTransparent = FALSE, BOOL TIsLit = FALSE>
	struct MeshRenderer final
	{
		typedef TMesh Mesh;
		typedef typename Mesh::Vertex Vertex;
		DWORD static const FVF = Mesh::FVF;
		D3DPRIMITIVETYPE static const PrimitiveType = Mesh::PrimitiveType;

		dxm::vec3 Position;
		dxm::vec3 Rotation;
		dxm::vec3 Scale = { 1.0f, 1.0f, 1.0f };
		IDirect3DTexture9* Texture = nullptr;
		IDirect3DPixelShader9* PixelShader = nullptr;
	protected:
		IDirect3DDevice9* const m_Device;
	private:
		Mesh const& m_Mesh;

	public:
		MeshRenderer(IDirect3DDevice9* const device, Mesh const& mesh)
			: m_Device(device), m_Mesh(mesh)
		{}
		~MeshRenderer()
		{
			if (Texture != nullptr)
			{
				Texture->Release();
			}
			if (PixelShader != nullptr)
			{
				PixelShader->Release();
			}
		}

		// -----------------------
		void Render() const
		{
			/* Setting up the transformations. */
			auto const matrix = dxm::translate(Position) * dxm::yawPitchRoll(Rotation.x, Rotation.y, Rotation.z) * dxm::scale(Scale);
			ThrowIfFailed(m_Device->SetTransform(D3DTS_WORLD, dxm::ptr(matrix)));

			/* Setting up the transparency. */
			ThrowIfFailed(m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TIsTransparent));

			/* Setting up the lights (we have a signle light source on the scene). */
			ThrowIfFailed(m_Device->SetRenderState(D3DRS_LIGHTING, TIsLit));
			ThrowIfFailed(m_Device->LightEnable(0, TIsLit));

			/* Setting up the textures and shaders (we have a signle texture and shared per object). */
			ThrowIfFailed(m_Device->SetTexture(0, Texture));
			ThrowIfFailed(m_Device->SetPixelShader(PixelShader));

			/* Setting up the mesh data. */
			ThrowIfFailed(m_Device->SetFVF(FVF));
			ThrowIfFailed(m_Device->SetStreamSource(0, m_Mesh.GetVertexBuffer(), 0, sizeof(Vertex)));
			ThrowIfFailed(m_Device->DrawPrimitive(PrimitiveType, 0, m_Mesh.GetPrimitivesCount()));
		}
	};	// struct MeshRenderer

	template<BOOL TIsTransparent = FALSE, BOOL TIsLit = FALSE>
	using TriangleMutableMeshRenderer = MeshRenderer<TriangleMutableMesh, TIsTransparent, TIsLit>;

	template<BOOL TIsTransparent = FALSE, BOOL TIsLit = FALSE>
	using LineMutableMeshRenderer = MeshRenderer<LineMutableMesh, TIsTransparent, TIsLit>;

	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //
	// Textures and Pixel shaders setup.
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //

	static void LoadTexture(IDirect3DDevice9* const device, wchar_t const* path, IDirect3DTexture9** const texturePtr)
	{
		/// @todo Load from raw data here.
		ThrowIfFailed(SUCCEEDED(D3DXCreateTextureFromFile(device, path, texturePtr)));
	}

	static void LoadShader(IDirect3DDevice9* const device, wchar_t const* path, IDirect3DPixelShader9** const pixelShaderPtr)
	{
		/// @todo Load from compiled data here.
		ID3DXBuffer* pixelShaderCode = nullptr;
		ID3DXBuffer* pixelShaderErrors = nullptr;
		ID3DXConstantTable* constantTable = nullptr;
		if (FAILED(D3DXCompileShaderFromFile(path, nullptr, nullptr, "main", "ps_2_0", 0, &pixelShaderCode, &pixelShaderErrors, &constantTable)))
		{
			MessageBoxA(nullptr, static_cast<char const*>(pixelShaderErrors->GetBufferPointer()), "Error", MB_OK);
			ThrowIfFailed(E_FAIL);
		}
		ThrowIfFailed(device->CreatePixelShader(static_cast<DWORD*>(pixelShaderCode->GetBufferPointer()), pixelShaderPtr));
	}
}	// namespace Presentation1
