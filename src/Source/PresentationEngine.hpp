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

#define GLM_FORCE_DEPTH_ZERO_TO_ONE 1 
#pragma warning(push, 0)
#include <glm/glm.hpp>
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
	// Engine Core.
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //

	class IEngineUpdatable : public IUpdatable {};
	class IEngineRenderable : public IRenderable {};

	using IEngineUpdatablePtr = std::shared_ptr<IEngineUpdatable>;
	using IEngineRenderablePtr = std::shared_ptr<IEngineRenderable>;

	/*************** 
	 * Helper tiny class that extracts context size from D3D device. */
	class DependsOnContextSize
	{
	protected:
		RECT m_Rect;

	protected:
		// -----------------------
		ADINL explicit DependsOnContextSize(IDirect3DDevice9* const device)
		{
			assert(device != nullptr);
			D3DDEVICE_CREATION_PARAMETERS deviceCreationParameters = {};
			Utils::RuntimeCheckH(device->GetCreationParameters(&deviceCreationParameters));
			Utils::RuntimeCheck(GetWindowRect(deviceCreationParameters.hFocusWindow, &m_Rect));
		}

		// -----------------------
		ADINL auto GetContextWidth() const
		{
			return static_cast<UINT>(m_Rect.right - m_Rect.left);
		}
		ADINL auto GetContextHeight() const
		{
			return static_cast<UINT>(m_Rect.bottom - m_Rect.top);
		}

		// -----------------------
		ADINL auto GetContextWidthF() const
		{
			return static_cast<FLOAT>(GetContextWidth());
		}
		ADINL auto GetContextHeightF() const
		{
			return static_cast<FLOAT>(GetContextHeight());
		}
	};	// class DependsOnContextSize

	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //
	// Meshes and materials.
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //

	enum UniversalCtor_t { UniversalCtor };

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
		ADINL LineVertex(UniversalCtor_t, dxm::vec3 const& position, dxm::vec3 const&, dxm::argb const diffuseColor, dxm::vec2 const&)
			: LineVertex(position, diffuseColor)
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
		ADINL TriangleVertex(UniversalCtor_t, dxm::vec3 const& position, dxm::vec3 const& normal, dxm::argb const diffuseColor, dxm::vec2 const& texCoord)
			: TriangleVertex(position, normal, diffuseColor, texCoord)
		{}
	};	// struct TriangleVertex

	/*************** 
	 * Describes a template immutable mesh. */
	template<typename TVertex, D3DPRIMITIVETYPE TPrimitiveType>
	class Mesh : public INonCopyable
	{
	public:
		using Vertex = TVertex;
		auto static const PrimitiveType = TPrimitiveType;
		auto static const FVF = Vertex::FVF;

	protected:
		IDirect3DDevice9* const m_Device;
	private:
		IDirect3DVertexBuffer9 mutable* m_VertexBuffer = nullptr;
		UINT mutable m_PrimitivesCount = 0;

	protected:
#if SUPPORT_LOADING_FROM_FILE
		ADINT void UpdateVertexBuffer(LPCWSTR const path, dxm::argb const color = ~0u) const;
#endif	// if SUPPORT_LOADING_FROM_FILE
		ADINT void UpdateVertexBuffer(LoadFromMemory_t, Vertex const* const vertices, UINT const verticesCount) const;

	public:

		// -----------------------
		ADINL explicit Mesh(IDirect3DDevice9* const device) 
			: m_Device(device)
		{
			assert(device != nullptr);
		}
#if SUPPORT_LOADING_FROM_FILE
		ADINL Mesh(IDirect3DDevice9* const device, LPCWSTR const path, dxm::argb const color = ~0u)
			: Mesh(device)
		{
			assert(device != nullptr);
			this->UpdateVertexBuffer(path, color);
		}
#endif	// if SUPPORT_LOADING_FROM_FILE
		ADINL Mesh(IDirect3DDevice9* const device, LoadFromMemory_t, Vertex const* const vertices, UINT const verticesCount)
			: Mesh(device)
		{
			assert(device != nullptr);
			this->UpdateVertexBuffer(LoadFromMemory, vertices, verticesCount);
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
	};	// class Mesh

	using TriangleMesh = Mesh<TriangleVertex, D3DPT_TRIANGLELIST>;
	using LineMesh = Mesh<LineVertex, D3DPT_LINELIST>;

	using TriangleMeshPtr = std::shared_ptr<TriangleMesh>;
	using LineMeshPtr = std::shared_ptr<LineMesh>;
	
	/*************** 
	 * Describes a template mutable mesh. */
	template<typename TVertex = TriangleVertex, D3DPRIMITIVETYPE TPrimitiveType = D3DPT_TRIANGLELIST>
	class MutableMesh final : public Mesh<TVertex, TPrimitiveType>
	{
	public:
		using Vertex = TVertex;
		using Base = Mesh<TVertex, TPrimitiveType>;
		auto static const PrimitiveType = TPrimitiveType;

	private:
		std::vector<Vertex> m_VertexAccumulator;
		std::mutex mutable m_Lock;

	public:
		// -----------------------
		ADINL explicit MutableMesh(IDirect3DDevice9* const device) 
			: Mesh(device) 
		{}

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
			this->Base::UpdateVertexBuffer(LoadFromMemory, m_VertexAccumulator.data(), m_VertexAccumulator.size());
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

	};	// class MutableMesh

	using TriangleMutableMesh = MutableMesh<TriangleVertex, D3DPT_TRIANGLELIST>;
	using LineMutableMesh = MutableMesh<LineVertex, D3DPT_LINELIST>;

	using TriangleMutableMeshPtr = std::shared_ptr<TriangleMutableMesh>;
	using LineMutableMeshPtr = std::shared_ptr<LineMutableMesh>;

	/*************** 
	 * Describes a layer of a mesh renderer. */
	using Layer_t = BYTE;
	namespace Layer
	{
		enum : Layer_t
		{
			Transparent = 1 << 0,
			Default = 1 << 1,
			Custom0 = 1 << 2,
		};
	}	// namespace Layer

	/*************** 
	 * Describes a base mesh renderer. */
	class BaseMeshRenderer : public IEngineRenderable
	{
	public:
		bool IsEnabled = true;
		Layer_t Layers = Layer::Default;
		dxm::vec3 PositionOffset;
		dxm::vec3 Position;
		dxm::vec3 Rotation;
		dxm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

	public:
		// -----------------------
		ADAPI virtual void OnRender() const override = 0;
		ADAPI virtual void OnRender(IDirect3DPixelShader9* const pixelShader) const = 0;
		ADAPI virtual void OnRender(IDirect3DPixelShader9* const pixelShader, IDirect3DTexture9* const texture1, IDirect3DTexture9* const texture2 = nullptr) const = 0;
	};	// class IMeshRenderer

	using BaseMeshRendererPtr = std::shared_ptr<BaseMeshRenderer>;

	/*************** 
	 * Describes a template mesh renderer. */
	template<typename TMesh = TriangleMutableMesh, BOOL TIsTransparent = FALSE, BOOL TIsLit = FALSE>
	class MeshRenderer final : public BaseMeshRenderer
	{
		friend class BaseCamera;

	public:
		using Mesh = TMesh;
		using MeshPtr = std::shared_ptr<Mesh>;
		using Vertex = typename Mesh::Vertex;
		auto static const FVF = Mesh::FVF;
		auto static const PrimitiveType = Mesh::PrimitiveType;

	public:
		D3DBLEND SourceBlend = D3DBLEND_SRCALPHA;
		D3DBLEND DestBlend = D3DBLEND_ONE;
	protected:
		IDirect3DDevice9* const m_Device;
		IDirect3DTexture9* m_Texture = nullptr;
		IDirect3DPixelShader9* m_PixelShader = nullptr;
	private:
		MeshPtr m_Mesh;

	public:
		// -----------------------
		ADAPI MeshRenderer(IDirect3DDevice9* const device, MeshPtr const& mesh);
#if SUPPORT_LOADING_FROM_FILE
		ADAPI MeshRenderer(IDirect3DDevice9* const device, MeshPtr const& mesh, LPCWSTR const texturePath, LPCWSTR const pixelShaderPath = nullptr);
#endif	// if SUPPORT_LOADING_FROM_FILE
		ADAPI MeshRenderer(IDirect3DDevice9* const device, MeshPtr const& mesh, LoadFromMemory_t, void const* const data, UINT const width, UINT const height);
		ADINL ~MeshRenderer()
		{
			if (m_Texture != nullptr)
			{
				m_Texture->Release();
			}
		}

		// -----------------------
		ADAPI void OnRender() const override final { OnRender(m_PixelShader, m_Texture); }
		ADAPI void OnRender(IDirect3DPixelShader9* const pixelShader, IDirect3DTexture9* const texture1, IDirect3DTexture9* const texture2 = nullptr) const override;
		ADAPI void OnRender(IDirect3DPixelShader9* const pixelShader) const override { OnRender(pixelShader, m_Texture); }
		ADAPI void OnUpdate() override final 
		{
			IEngineRenderable::OnUpdate();
		}

	};	// class MeshRenderer

	template<BOOL TIsTransparent = FALSE, BOOL TIsLit = FALSE>
	using TriangleMeshRenderer = MeshRenderer<TriangleMesh, TIsTransparent, TIsLit>;
	template<BOOL TIsTransparent = FALSE, BOOL TIsLit = FALSE>
	using TriangleMutableMeshRenderer = MeshRenderer<TriangleMutableMesh, TIsTransparent, TIsLit>;
	// -----------------------
	template<BOOL TIsTransparent = FALSE, BOOL TIsLit = FALSE>
	using LineMeshRenderer = MeshRenderer<LineMesh, TIsTransparent, TIsLit>;
	template<BOOL TIsTransparent = FALSE, BOOL TIsLit = FALSE>
	using LineMutableMeshRenderer = MeshRenderer<LineMutableMesh, TIsTransparent, TIsLit>;

	template<BOOL TIsTransparent = FALSE, BOOL TIsLit = FALSE>
	using TriangleMeshRendererPtr = std::shared_ptr<TriangleMeshRenderer<TIsTransparent, TIsLit>>;
	template<BOOL TIsTransparent = FALSE, BOOL TIsLit = FALSE>
	using TriangleMutableMeshRendererPtr = std::shared_ptr<TriangleMutableMeshRenderer<TIsTransparent, TIsLit>>;
	// -----------------------
	template<BOOL TIsTransparent = FALSE, BOOL TIsLit = FALSE>
	using LineMeshRendererPtr = std::shared_ptr<LineMeshRenderer<TIsTransparent, TIsLit>>;
	template<BOOL TIsTransparent = FALSE, BOOL TIsLit = FALSE>
	using LineMutableMeshRendererPtr = std::shared_ptr<LineMutableMeshRenderer<TIsTransparent, TIsLit>>;

	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //
	// Cameras and render targets.
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //

	using ScenePtr = std::shared_ptr<class Scene>;
	using BaseCameraPtr = std::shared_ptr<class BaseCamera>;
	using CameraPtr = std::shared_ptr<class Camera>;
	using OrbitalCameraPtr = std::shared_ptr<class OrbitalCamera>;

	enum class BaseCameraProjection
	{
		Perspective,
		Orthographic,
	};	// enum class CameraType

	/*************** 
	 * Describes a base camera. */
	class BaseCamera : public IEngineRenderable, public DependsOnContextSize
	{
		struct RenderTarget final
		{
			IDirect3DSurface9* Surface;
			IDirect3DTexture9* Texture;
		};	// struct RenderTarget

	protected:
		IDirect3DDevice9* const m_Device;
		RenderTarget m_RenderTargets[2];
	public:
		dxm::argb ClearColor = 0u;
		FLOAT FieldOfView = dxm::radians(60.0f);
		FLOAT Size = 0.25f;
		FLOAT NearClippingPlane = 0.01f;
		FLOAT FarClippingPlane = 20.0f;
		BaseCameraProjection Projection = BaseCameraProjection::Perspective;
		Rect Viewport;
		Layer_t Layer = Layer::Transparent | Layer::Default;
		IDirect3DSurface9* RenderTarget = nullptr;

	public:

		// -----------------------
		ADAPI explicit BaseCamera(IDirect3DDevice9* const device);

		// -----------------------
		ADAPI void OnRender() const;
		ADAPI void OnRenderToTargets(Scene const* scene) const;
		ADAPI void OnRenderTargets() const;
		ADAPI void OnUpdate() override 
		{
			IEngineRenderable::OnUpdate();
		}

	private:
		ADINT void Clear(dxm::argb const color = ~0u) const;
	};	// class Camera

	/*************** 
	 * Describes an pane mouse-controlled camera. */
	class Camera final : public BaseCamera
	{
	private:
		POINT mutable m_PrevMousePosition = {};
	public:
		dxm::vec3 Position;
		dxm::vec3 Rotation;

		// -----------------------
		ADINL explicit Camera(IDirect3DDevice9* const device)
			: BaseCamera(device)
		{}

		// -----------------------
		ADAPI void OnRender() const override final;
		ADAPI void OnUpdate() override final;
	};	// class Camera

	/*************** 
	 * Describes an orbital mouse-controlled camera. */
	class OrbitalCamera final : public BaseCamera
	{
	private:
		POINT mutable m_PrevMousePosition = {};
	public:
		dxm::vec2 Rotation, RotationMin, RotationMax;
		dxm::vec3 RotationCenter;
		dxm::vec3 CenterOffset;

		// -----------------------
		ADINL explicit OrbitalCamera(IDirect3DDevice9* const device)
			: BaseCamera(device)
		{}

		// -----------------------
		ADAPI void OnUpdate() override final;
		ADAPI void OnRender() const override final;
	};	// class OrbitalCamera

	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //
	// Scenes and bridging.
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //

	using EngineWidgetPtr = std::shared_ptr<class EngineWidget>;

	/*************** 
	 * Describes a scene. */
	class Scene : public IEngineRenderable, public DependsOnContextSize
	{
		friend class BaseCamera;
		
	protected:
		IDirect3DDevice9* const m_Device;
		RECT m_Rect;
	private:

		std::vector<BaseCameraPtr> m_Cameras;
		std::vector<BaseMeshRendererPtr> m_MeshRenderers;
		std::vector<IEngineRenderablePtr> m_CustomRenderables;
		std::vector<IEngineUpdatablePtr> m_Updatables;

	public:

		// -----------------------
		ADAPI explicit Scene(IDirect3DDevice9* const device);

		// -----------------------
		ADAPI void OnUpdate() override final;
		ADAPI void OnRender() const override final;

		// ***********************************************************************************************
		// Custom objects.
		// ***********************************************************************************************

		// -----------------------
		template<typename TEngineUpdatable, typename... TArgs>
		ADINL auto CustomUpdatable(TArgs&&... args)
		{
			static_assert(std::is_base_of_v<IEngineUpdatable, TEngineUpdatable>, "Invalid base type.");

			auto const updatable = std::make_shared<TEngineUpdatable>(std::forward<TArgs>(args)...);
			m_Updatables.push_back(updatable);
			return updatable;
		}

		// -----------------------
		template<typename TEngineRenderable, typename... TArgs>
		ADINL auto CustomRenderable(TArgs&&... args)
		{
			static_assert(std::is_base_of_v<IEngineRenderable, TEngineRenderable>, "Invalid base type.");

			auto const renderable = std::make_shared<TEngineRenderable>(std::forward<TArgs>(args)...);
			m_CustomRenderables.push_back(renderable);
			return renderable;
		}

		// ***********************************************************************************************
		// Built-in objects.
		// ***********************************************************************************************

		// -----------------------
		ADINL auto Camera()
		{
			auto const camera = std::make_shared<CameraPtr::element_type>(m_Device);
			m_Cameras.push_back(camera);
			return camera;
		}

		// -----------------------
		ADINL auto OrbitalCamera()
		{
			auto const orbitalCamera = std::make_shared<OrbitalCameraPtr::element_type>(m_Device);
			m_Cameras.push_back(orbitalCamera);
			return orbitalCamera;
		}

		// -----------------------
		template<typename... TArgs>
		ADINL auto TriangleMesh(TArgs&&... args) const
		{
			return std::make_shared<TriangleMeshPtr::element_type>(m_Device, std::forward<TArgs>(args)...);
		}
		template<typename... TArgs>
		ADINL auto TriangleMutableMesh(TArgs&&... args) const
		{
			return std::make_shared<TriangleMutableMeshPtr::element_type>(m_Device, std::forward<TArgs>(args)...);
		}

		// -----------------------
		template<typename... TArgs>
		ADINL auto LineMesh(TArgs&&... args) const
		{
			return std::make_shared<LineMeshPtr::element_type>(m_Device, std::forward<TArgs>(args)...);
		}
		template<typename... TArgs>
		ADINL auto LineMutableMesh(TArgs&&... args) const
		{
			return std::make_shared<LineMutableMeshPtr::element_type>(m_Device, std::forward<TArgs>(args)...);
		}

		// -----------------------
		template<BOOL TIsTransparent = FALSE, BOOL TIsLit = FALSE, typename... TArgs>
		ADINL auto TriangleMeshRenderer(TArgs&&... args)
		{
			auto const triangleMeshRenderer = std::make_shared<typename TriangleMeshRendererPtr<TIsTransparent, TIsLit>::element_type>(m_Device, std::forward<TArgs>(args)...);
			m_MeshRenderers.push_back(triangleMeshRenderer);
			return triangleMeshRenderer;
		}
		template<BOOL TIsTransparent = FALSE, BOOL TIsLit = FALSE, typename... TArgs>
		ADINL auto TriangleMutableMeshRenderer(TArgs&&... args)
		{
			auto const triangleMeshRenderer = std::make_shared<typename TriangleMutableMeshRendererPtr<TIsTransparent, TIsLit>::element_type>(m_Device, std::forward<TArgs>(args)...);
			m_MeshRenderers.push_back(triangleMeshRenderer);
			return triangleMeshRenderer;
		}

		// -----------------------
		template<BOOL TIsTransparent = FALSE, BOOL TIsLit = FALSE, typename... TArgs>
		ADINL auto LineMeshRenderer(TArgs&&... args)
		{
			auto const lineMeshRenderer = std::make_shared<typename LineMeshRendererPtr<TIsTransparent, TIsLit>::element_type>(m_Device, std::forward<TArgs>(args)...);
			m_MeshRenderers.push_back(lineMeshRenderer);
			return lineMeshRenderer;
		}
		template<BOOL TIsTransparent = FALSE, BOOL TIsLit = FALSE, typename... TArgs>
		ADINL auto LineMutableMeshRenderer(TArgs&&... args)
		{
			auto const lineMeshRenderer = std::make_shared<typename LineMutableMeshRendererPtr<TIsTransparent, TIsLit>::element_type>(m_Device, std::forward<TArgs>(args)...);
			m_MeshRenderers.push_back(lineMeshRenderer);
			return lineMeshRenderer;
		}

	};	// struct Scene

	/***************
	 * Bridges engine with the framework. */
	class EngineWidget : public GraphicsWidget
	{
	private:
		ScenePtr m_Scene;

	public:

		// -----------------------
		ADINL EngineWidget(HWND const hwnd, IDirect3DDevice9* const device)
			: GraphicsWidget(hwnd, device)
		{
		}

		// -----------------------
		template<typename TScene, typename... TArgs>
		ADINL std::shared_ptr<TScene> InitializeScene(TArgs&&... args)
		{
			static_assert(std::is_base_of_v<Scene, TScene>, "Invalid base type.");
			assert(m_Scene == nullptr);
			auto const scene = std::make_shared<TScene>(m_Device, std::forward<TArgs>(args)...);
			m_Scene = scene;
			return scene;
		}

		// -----------------------
		ADINL void OnUpdate() override
		{
			assert(m_Scene != nullptr);
			IRenderable::OnUpdate();
			m_Scene->OnUpdate();
		}
		ADINL void OnRender() const override
		{
			assert(m_Scene != nullptr);
			IRenderable::OnRender();
			m_Scene->OnRender();
		}

	};	// struct EngineWidget

}	// namespace Presentation2

#include "PresentationEngine.inl"
