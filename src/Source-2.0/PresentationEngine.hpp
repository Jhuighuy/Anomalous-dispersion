// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// Copyright (C) 2016 Butakov Oleg, Plaxin Gleb.
// Computational Methods @ Computational Mathematics & Cybernetics, MSU.
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

#pragma once
#include "PresentationFramework.hpp"

#ifndef _USE_MATH_DEFINES 
#define _USE_MATH_DEFINES 1
#endif	// ifndef _USE_MATH_DEFINES
#include <math.h>
#define F_PI float(M_PI)

#include <mutex>

#pragma warning(push, 0)
#include <glm/glm.hpp>
#pragma warning(pop)

#pragma warning(push, 0)
#include <glm/gtx/transform.hpp>
#pragma warning(pop)

namespace Presentation2
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
		static bool aabb_check(vec3 const& a, vec3 const& b, vec3 const& v)
		{
			auto const mx = min(a.x, b.x), px = max(a.x, b.x);
			auto const my = min(a.y, b.y), py = max(a.y, b.y);
			auto const mz = min(a.z, b.z), pz = max(a.z, b.z);
			auto const r = mx <= v.x && v.x <= px
				&& my <= v.y && v.y <= py
				&& mz <= v.z && v.z <= pz;
			return r;
		}
	}	// namespace dxm

	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //
	// Camera setup.
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //

	/*************** 
	 * Describes a base camera. */
	struct Camera : public IUpdatable
	{
	public:
		FLOAT FieldOfView = F_PI / 1.5f;
		FLOAT NearClippingPlane = 0.01f, FarClippingPlane = 100.0f;
	protected:
		IDirect3DDevice9* const m_Device;
		RECT m_Rect;

		// -----------------------
		ADINL auto GetContextWidth() const
		{
			return static_cast<FLOAT>(m_Rect.right - m_Rect.left);
		}
		ADINL auto GetContextHeight() const
		{
			return static_cast<FLOAT>(m_Rect.bottom - m_Rect.top);
		}

	public:

		// -----------------------
		ADAPI explicit Camera(IDirect3DDevice9* const device);

		// -----------------------
		ADAPI void Update() const override;

	};	// struct Camera

	/*************** 
	 * Describes an orbital mouse-controlled camera. */
	struct OrbitalCamera final : public Camera
	{
	private:
		FLOAT mutable m_CameraRotationYaw = -F_PI / 2.0f;	/// @todo Calibrate value here.
		FLOAT mutable m_CameraRotationPitch = 0.0f;
		POINT mutable m_PrevMousePosition = {};
		dxm::mat4 mutable m_ProjectionMatrix;
		dxm::mat4 mutable m_ViewMatrix;
	public:
		dxm::vec3 const RotationCenter = { 0.0f, 1.2f, 2.0f };	/// @todo And here.
		dxm::vec3 const CenterOffset = { 0.0f, 0.0f, -1.8f };
		dxm::vec3 const Up = { 0.0f, 1.0f, 0.0f };

		// -----------------------
		ADAPI explicit OrbitalCamera(IDirect3DDevice9* const device)
			: Camera(device)
		{
			m_ProjectionMatrix = dxm::perspectiveFovLH(F_PI / 3.0f, GetContextWidth(), GetContextHeight(), 0.01f, 100.0f);
			Update(true);
		}

		// -----------------------
		ADINL void Update() const override { Update(false); }
		ADAPI void Update(bool const forceUpdate) const;
	};	// struct OrbitalCamera

	using CameraPtr = std::shared_ptr<Camera>;
	using OrbitalCameraPtr = std::shared_ptr<OrbitalCamera>;

	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //
	// Mesh setup.
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //

	/*************** 
	 * Describes a single colored line vertex. */
	struct LineVertex final
	{
		DWORD static const FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;
		dxm::vec3 const Position;
		dxm::argb const DiffuseColor;

		// -----------------------
		ADINL LineVertex(dxm::vec3 const& position, dxm::argb const diffuseColor)
			: Position(position), DiffuseColor(diffuseColor)
		{}
	};	// struct LineVertex

	/*************** 
	 * Describes a single colored textured triangle vertex with a normal. */
	struct TriangleVertex final
	{
		DWORD static const FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1;
		dxm::vec3 const Position;
		dxm::vec3 const Normal;
		dxm::argb const DiffuseColor;
		dxm::vec2 const TexCoord;

		// -----------------------
		ADINL TriangleVertex(dxm::vec3 const& position, dxm::vec3 const& normal, dxm::argb const diffuseColor, dxm::vec2 const& texCoord)
			: Position(position), Normal(normal), DiffuseColor(diffuseColor), TexCoord(texCoord)
		{}
	};	// struct TriangleVertex

	/*************** 
	 * Describes a template immutable mesh. */
	template<typename TVertex, D3DPRIMITIVETYPE TPrimitiveType>
	struct Mesh : public INonCopyable
	{
		using Vertex = TVertex;
		auto static const PrimitiveType = TPrimitiveType;
		auto static const FVF = Vertex::FVF;

	protected:
		IDirect3DDevice9* const m_Device;
	private:
		IDirect3DVertexBuffer9 mutable* m_VertexBuffer = nullptr;
		UINT mutable m_PrimitivesCount = 0;

	protected:
		ADINT void UpdateVertexBuffer(Vertex const* const vertices, UINT const verticesCount) const;

	public:

		// -----------------------
		ADINL explicit Mesh(IDirect3DDevice9* const device) 
			: m_Device(device)
		{
			assert(device != nullptr);
		}
		ADINL Mesh(IDirect3DDevice9* const device, Vertex const* const vertices, UINT const verticesCount) 
			: Mesh(device)
		{
			this->UpdateVertexBuffer(device, vertices, verticesCount);
		}
		ADINL virtual ~Mesh()
		{
			if (m_VertexBuffer != nullptr)
			{
				m_VertexBuffer->Release();
			}
		}

		// -----------------------
		ADINL virtual IDirect3DVertexBuffer9* GetVertexBuffer() const
		{
			return m_VertexBuffer;
		}
		ADINL virtual UINT GetPrimitivesCount() const
		{
			return m_PrimitivesCount;
		}
	};	// struct Mesh

	using TriangleMesh = Mesh<TriangleVertex, D3DPT_TRIANGLELIST>;
	using LineMesh = Mesh<LineVertex, D3DPT_LINELIST>;

	using TriangleMeshPtr = std::shared_ptr<TriangleMesh>;
	using LineMeshPtr = std::shared_ptr<LineMesh>;
	
	/*************** 
	 * Describes a template mutable mesh. */
	template<typename TVertex = TriangleVertex, D3DPRIMITIVETYPE TPrimitiveType = D3DPT_TRIANGLELIST>
	struct MutableMesh final : public Mesh<TVertex, TPrimitiveType>
	{
		auto static const PrimitiveType = TPrimitiveType;
		using Vertex = TVertex;
		using Base = Mesh<TVertex, TPrimitiveType>;

	private:
		std::vector<Vertex> m_VertexAccumulator;
		std::mutex mutable m_Lock;

	public:
		// -----------------------
		ADINL explicit MutableMesh(IDirect3DDevice9* const device) 
			: Mesh(device) {}

		// -----------------------
		ADINL void BeginUpdateVertices()
		{
			m_Lock.lock();
			m_VertexAccumulator.clear();
		}
		ADINL void AddVertex(Vertex const& vertex)
		{
			m_VertexAccumulator.push_back(vertex);
		}
		ADINL void EndUpdateVertices()
		{
			this->Base::UpdateVertexBuffer(m_VertexAccumulator.data(), m_VertexAccumulator.size());
			m_Lock.unlock();
		}

		// -----------------------
		ADINL IDirect3DVertexBuffer9* GetVertexBuffer() const override
		{
			/* If we can lock the mesh - it is not being updated - can return the real vertex buffer. */
			std::unique_lock<std::mutex> lock(m_Lock, std::try_to_lock);
			return lock.owns_lock() ? this->Base::GetVertexBuffer() : nullptr;
		}
		ADINL UINT GetPrimitivesCount() const override
		{
			/* If we can lock the mesh - it is not being updated - can return the real primitives count. */
			std::unique_lock<std::mutex> lock(m_Lock, std::try_to_lock);
			return lock.owns_lock() ? this->Base::GetPrimitivesCount() : 0;
		}

	};	// struct MutableMesh

	using TriangleMutableMesh = MutableMesh<TriangleVertex, D3DPT_TRIANGLELIST>;
	using LineMutableMesh = MutableMesh<LineVertex, D3DPT_LINELIST>;

	using TriangleMutableMeshPtr = std::shared_ptr<TriangleMutableMesh>;
	using LineMutableMeshPtr = std::shared_ptr<LineMutableMesh>;

	/*************** 
	 * Describes a template mesh renderer. */
	template<typename TMesh = TriangleMutableMesh, BOOL TIsTransparent = FALSE, BOOL TIsLit = FALSE>
	struct MeshRenderer final : public IRenderable
	{
		using Mesh = TMesh;
		using MeshPtr = std::shared_ptr<Mesh>;
		using Vertex = typename Mesh::Vertex;
		auto static const FVF = Mesh::FVF;
		auto static const PrimitiveType = Mesh::PrimitiveType;

	public:
		dxm::vec3 PositionOffset;
		dxm::vec3 Position;
		dxm::vec3 Rotation;
		dxm::vec3 Scale = { 1.0f, 1.0f, 1.0f };
		IDirect3DTexture9* Texture = nullptr;
		IDirect3DPixelShader9* PixelShader = nullptr;
	protected:
		IDirect3DDevice9* const m_Device;
	private:
		MeshPtr m_Mesh;

	public:
		// -----------------------
		ADINL MeshRenderer(IDirect3DDevice9* const device, MeshPtr const& mesh)
			: m_Device(device), m_Mesh(mesh)
		{
			assert(m_Device != nullptr);
			assert(m_Mesh != nullptr);
		}
		ADINL ~MeshRenderer()
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
		ADAPI void Update() const override {}
		ADAPI void Render() const override;

	};	// struct MeshRenderer

	template<BOOL TIsTransparent = FALSE, BOOL TIsLit = FALSE>
	using TriangleMutableMeshRenderer = MeshRenderer<TriangleMutableMesh, TIsTransparent, TIsLit>;
	template<BOOL TIsTransparent = FALSE, BOOL TIsLit = FALSE>
	using LineMutableMeshRenderer = MeshRenderer<LineMutableMesh, TIsTransparent, TIsLit>;

	template<BOOL TIsTransparent = FALSE, BOOL TIsLit = FALSE>
	using TriangleMutableMeshRendererPtr = std::shared_ptr<TriangleMutableMeshRenderer<TIsTransparent, TIsLit>>;
	template<BOOL TIsTransparent = FALSE, BOOL TIsLit = FALSE>
	using LineMutableMeshRendererPtr = std::shared_ptr<LineMutableMeshRenderer<TIsTransparent, TIsLit>>;

	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //
	// Textures and Pixel shaders setup.
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //

	// -----------------------
#ifdef SUPPORT_LOADING_FROM_FILE
	ADAPI extern void LoadOBJ(wchar_t const* const path, TriangleMutableMeshPtr const& mesh, dxm::argb const color = ~0);
#endif	// ifdef SUPPORT_LOADING_FROM_FILE

	// -----------------------
#ifdef SUPPORT_LOADING_FROM_FILE
	ADAPI extern void LoadTexture(IDirect3DDevice9* const device, wchar_t const* const path, IDirect3DTexture9** const texturePtr);
#endif	// ifdef SUPPORT_LOADING_FROM_FILE
	ADAPI extern void LoadTexture(IDirect3DDevice9* const device, LoadFromMemory_t, void const* const compiledData, UINT const width, UINT const height, IDirect3DTexture9** const texturePtr);

	// -----------------------
#ifdef SUPPORT_LOADING_FROM_FILE
	ADAPI extern void LoadPixelShader(IDirect3DDevice9* const device, wchar_t const* path, IDirect3DPixelShader9** const pixelShaderPtr);
#endif	// ifdef SUPPORT_LOADING_FROM_FILE
	ADAPI extern void LoadPixelShader(IDirect3DDevice9* const device, LoadFromMemory_t, void const* const data, IDirect3DPixelShader9** const pixelShaderPtr);

}	// namespace Presentation2

#include "PresentationEngine.inl"
