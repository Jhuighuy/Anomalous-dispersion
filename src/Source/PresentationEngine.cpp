// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// Copyright (C) 2016 Butakov Oleg, Plaxin Gleb.
// Computational Methods @ Computational Mathematics & Cybernetics, MSU.
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

#include "PresentationEngine.hpp"
#include "PresentationScene.hpp"

#if SUPPORT_LOADING_FROM_FILE
#include <d3dx9.h>
#endif	// if SUPPORT_LOADING_FROM_FILE

namespace Presentation2
{
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //
	// Cameras and render targets.
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //
	
	ADINT auto static const Up = dxm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

	ADINT static std::vector<IEngineUpdatablePtr> m_Updatables;
	ADINT static std::vector<IEngineRenderablePtr> m_Renderables;

	// -----------------------
	ADAPI BaseCamera::BaseCamera(IDirect3DDevice9* const device) 
		: DependsOnContextSize(device)
		, m_Device(device), Viewport(UpperLeftPivot, 0, 0, /*GetContextWidth()*/1440, /*GetContextHeight()*/1080)
	{
		assert(m_Device != nullptr);

		for (auto& renderTarget : m_RenderTargets)
		{
			Utils::RuntimeCheckH(D3DXCreateTexture(m_Device, GetContextWidth(), GetContextHeight(), 0
				, D3DUSAGE_RENDERTARGET, D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT, &const_cast<IDirect3DTexture9*&>(renderTarget.Texture)));
			Utils::RuntimeCheckH(renderTarget.Texture->GetSurfaceLevel(0, &const_cast<IDirect3DSurface9*&>(renderTarget.Surface)));
		}
	}

	// -----------------------
	ADAPI void BaseCamera::OnRender() const
	{
		IEngineRenderable::OnRender();
		
		auto const viewportWidth = static_cast<FLOAT>(Viewport.Width);
		auto const viewportHeight = static_cast<FLOAT>(Viewport.Height);
		auto const viewportAspect = viewportWidth / viewportHeight;

		auto const projection = Projection == BaseCameraProjection::Perspective
			? dxm::perspectiveFovLH<FLOAT>(FieldOfView, viewportWidth, viewportHeight, NearClippingPlane, FarClippingPlane)
			: dxm::orthoLH(-0.5f * Size * viewportAspect, 0.5f * Size * viewportAspect, -0.5f * Size, 0.5f * Size, NearClippingPlane, FarClippingPlane);
		Utils::RuntimeCheckH(m_Device->SetTransform(D3DTS_PROJECTION, dxm::ptr(projection)));
	}
	
	// -----------------------
	ADINT void BaseCamera::Clear(dxm::argb const color) const
	{
	//	D3DVIEWPORT9 viewport = { Viewport.X, Viewport.Y, Viewport.Width, Viewport.Height, 0.0f, 1.0f };
	//	Utils::RuntimeCheckH(m_Device->SetViewport(&viewport));
		Utils::RuntimeCheckH(m_Device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, color, 1.0f, 0));
	}

	TriangleMeshRendererPtr<TRUE> static screenQuad;
	IDirect3DPixelShader9 static* forceBlackShader = nullptr;
	IDirect3DPixelShader9 static* blendTexturesShader = nullptr;

	// -----------------------
	ADINT void BaseCamera::OnRenderToTargets(Scene const* scene) const
	{
		/// @todo Move this thing somewhere.
		if (screenQuad == nullptr)
		{
			/// @todo
			auto static screenQuadMesh = scene->TriangleMesh(L"../gfx/quad.obj");
			screenQuad = std::make_shared<::Presentation2::TriangleMeshRenderer<TRUE>>(m_Device, screenQuadMesh, nullptr, L"../gfx/Shaders/ForceTransparent.hlsl");
			forceBlackShader = screenQuad->m_PixelShader;
			screenQuad = std::make_shared<::Presentation2::TriangleMeshRenderer<TRUE>>(m_Device, screenQuadMesh, nullptr, L"../gfx/Shaders/BlendTexturesFXAA.hlsl");
			blendTexturesShader = screenQuad->m_PixelShader;
			screenQuad->m_PixelShader = nullptr;
		}
		
		auto const layer = Layer & ~Layer::Transparent;
		auto const rendersOpaque = layer != 0;

		// -------------------------------------------------------------------------------------------------------------------
		// Pass 1: rendering all transparent objects with masked solid.
		// -------------------------------------------------------------------------------------------------------------------

		IDirect3DSurface9* backBuffer = nullptr;
		{
			/* Reading our old render targets out of the device. */
			Utils::RuntimeCheckH(m_Device->GetRenderTarget(0, &backBuffer));
			Utils::RuntimeCheckH(m_Device->SetRenderTarget(0, m_RenderTargets[0].Surface));

			{
				/* Beginning out scene. */
				Utils::RuntimeCheckH(m_Device->BeginScene());
				Clear(D3DCOLOR_ARGB(0x00, 0x00, 0x00, 0x00));
			}
		}

		{
			/* Setting up the transparency. */
			Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
			Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
			Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE));

			/* Rendering all the opaque meshed in black shader. */
			if (rendersOpaque)
			{
				for (auto const& meshRenderer : scene->m_MeshRenderers)
				{
					auto const isMeshRendererOpaque = (meshRenderer->Layers & Layer::Transparent) == 0;
					auto const meshRendererLayers = meshRenderer->Layers & ~Layer::Transparent;;
					if ((meshRendererLayers & layer) != 0 && isMeshRendererOpaque)
					{
						meshRenderer->OnRender(forceBlackShader);
					}
				}
			}
			/* And transparent ones as they are.. */
			for (auto const& meshRenderer : scene->m_MeshRenderers)
			{
				auto const isMeshRendererTransparent = (meshRenderer->Layers & Layer::Transparent) != 0;
				auto const meshRendererLayers = meshRenderer->Layers & ~Layer::Transparent;;
				if ((meshRendererLayers & layer) != 0 && isMeshRendererTransparent)
				{
					meshRenderer->OnRender();
				}
			}
			/* And some custom renderables at the end.. */
			for (auto const& customRenderable : scene->m_CustomRenderables)
			{
				customRenderable->OnRender();
			}
		}

		/* Ending our scene. */
		Utils::RuntimeCheckH(m_Device->EndScene());

		// -------------------------------------------------------------------------------------------------------------------
		// Pass 2: rendering only opaque objects.
		// -------------------------------------------------------------------------------------------------------------------

		if (rendersOpaque)
		{
			/* Setting our second render target. */
			Utils::RuntimeCheckH(m_Device->SetRenderTarget(0, m_RenderTargets[1].Surface));

			{	/* Preparing for the second pass.. */
				Utils::RuntimeCheckH(m_Device->BeginScene());
				Clear(ClearColor);
			}
		}
		if (rendersOpaque)
		{
			/* Setting up the transparency. */
			Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));

			/* Rendering all the opaque meshes. */
			for (auto const& meshRenderer : scene->m_MeshRenderers)
			{
				auto const isMeshRendererOpaque = (meshRenderer->Layers & Layer::Transparent) == 0;
				auto const meshRendererLayers = meshRenderer->Layers & ~Layer::Transparent;;
				if ((meshRendererLayers & layer) != 0 && isMeshRendererOpaque)
				{
					meshRenderer->OnRender();
				}
			}

			/* Ending our scene. */
			Utils::RuntimeCheckH(m_Device->EndScene());
		}

		/* Restoring original render targets. */
		Utils::RuntimeCheckH(m_Device->SetRenderTarget(0, backBuffer));
	}

	// -----------------------
	ADINT void BaseCamera::OnRenderTargets() const
	{
		// -------------------------------------------------------------------------------------------------------------------
		// Pass 3: merging passes 1 and 2.
		// -------------------------------------------------------------------------------------------------------------------

		/* Blending results of two passes and performing anti-aliasing. */
		/* Setting up new view and projection. */
		auto const viewMatrix = dxm::mat4();
		auto const projectionMatrix = dxm::orthoLH(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);

		screenQuad->Position = dxm::vec3(Viewport.X / GetContextWidthF(), Viewport.Y / GetContextHeightF(), 0.0f);
		screenQuad->Scale = dxm::vec3(Viewport.Width / GetContextWidthF(), Viewport.Height / GetContextHeightF(), 1.0f);

		Utils::RuntimeCheckH(m_Device->SetTransform(D3DTS_PROJECTION, dxm::ptr(projectionMatrix)));
		Utils::RuntimeCheckH(m_Device->SetTransform(D3DTS_VIEW, dxm::ptr(viewMatrix)));

		/* We want our picture been blended into the screen. */
		Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO));
		Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR));

		/* We do not need any culling for now. */
		Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
		Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_ZENABLE, FALSE));
		screenQuad->OnRender(blendTexturesShader, m_RenderTargets[0].Texture, m_RenderTargets[1].Texture);
		Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW));
		Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_ZENABLE, TRUE));
	}

	// -----------------------
	ADAPI void Camera::OnRender() const
	{
		auto const world = dxm::translate(Position) * dxm::toMat4(dxm::quat(Rotation));
		auto const view = dxm::inverse(world);
		Utils::RuntimeCheckH(m_Device->SetTransform(D3DTS_VIEW, dxm::ptr(view)));
		BaseCamera::OnRender();
	}
	// -----------------------
	ADAPI void Camera::OnUpdate()
	{
		BaseCamera::OnUpdate();

		//if (GetAsyncKeyState(VK_LBUTTON) != 0 && m_PrevMousePosition.x != 0 && m_PrevMousePosition.y != 0)
		//{
		//	POINT mouseCurrentPosition = {};
		//	Utils::RuntimeCheck(GetCursorPos(&mouseCurrentPosition));
		//	if (PtInRect(&m_Rect, mouseCurrentPosition))
		//	{
		//		/* Mouse has been moved inside the context rect while left button has been pressed. */
		//		auto const deltaY = static_cast<FLOAT>(mouseCurrentPosition.y - m_PrevMousePosition.y) / GetContextWidthF();
		//		auto const deltaX = static_cast<FLOAT>(mouseCurrentPosition.x - m_PrevMousePosition.x) / GetContextHeightF();

		//		Position.x += deltaX;
		//		Position.y += deltaY;
		//	}
		//}
		Utils::RuntimeCheck(GetCursorPos(&m_PrevMousePosition));
	}

	// -----------------------
	ADAPI void OrbitalCamera::OnUpdate()
	{
		BaseCamera::OnUpdate();

		if (GetAsyncKeyState(VK_LBUTTON) != 0 && m_PrevMousePosition.x != 0 && m_PrevMousePosition.y != 0)
		{
			POINT mouseCurrentPosition = {};
			Utils::RuntimeCheck(GetCursorPos(&mouseCurrentPosition));
			if (PtInRect(&m_Rect, mouseCurrentPosition))
			{
				/* Mouse has been moved inside the context rect while left button has been pressed. */
				auto const deltaX = static_cast<FLOAT>(mouseCurrentPosition.y - m_PrevMousePosition.y) / GetContextWidthF();
				auto const deltaY = static_cast<FLOAT>(mouseCurrentPosition.x - m_PrevMousePosition.x) / GetContextHeightF();

				Rotation.y += deltaY;
				Rotation.x = dxm::clamp(Rotation.x + deltaX, -F_PI / 12.0f, F_PI / 7.5f);
				/// @todo Move this constants somewhere.
			}
		}
		Utils::RuntimeCheck(GetCursorPos(&m_PrevMousePosition));
	}
	// -----------------------
	ADAPI void OrbitalCamera::OnRender() const
	{
		auto const translation = dxm::translate(RotationCenter);
		auto const cameraRotation = dxm::toMat4(dxm::quat(dxm::vec3(Rotation, 0.0f)));
		auto const view = dxm::lookAtLH(dxm::vec3(translation * cameraRotation * dxm::vec4(CenterOffset, 1.0f))
			, RotationCenter, dxm::vec3(cameraRotation * Up));
		Utils::RuntimeCheckH(m_Device->SetTransform(D3DTS_VIEW, dxm::ptr(view)));
		BaseCamera::OnRender();
	}

	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //
	// Scenes and bridging.
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //

	// -----------------------
	ADAPI Scene::Scene(IDirect3DDevice9* const device)
		: DependsOnContextSize(device)
		, m_Device(device)
	{
		assert(m_Device != nullptr);

		{
			/* Setting up default blending parameters. */
			Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD));
			Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
			Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE));
		}

		/* Setting up default lights. */
		D3DLIGHT9 light = { };
		light.Type = D3DLIGHT_DIRECTIONAL;
		light.Diffuse = { 0.7f, 0.7f, 0.7f, 1.0f };
		light.Direction = { 0.0f, -0.3f, 1.0f };
		light.Attenuation1 = 0.1f;
		Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_LIGHTING, TRUE));
		Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_AMBIENT, ~0u));
		Utils::RuntimeCheckH(m_Device->SetLight(0, &light));
	}

	// -----------------------
	ADAPI void Scene::OnUpdate()
	{
		IEngineRenderable::OnUpdate();
		for (auto const& updatable : m_Updatables)
		{
			updatable->OnUpdate();
		}
		for (auto const& camera : m_Cameras)
		{
			camera->OnUpdate();
		}
		for (auto const& meshRenderer : m_MeshRenderers)
		{
			meshRenderer->OnUpdate();
		}
		for (auto const& renderable : m_CustomRenderables)
		{
			renderable->OnUpdate();
		}
	}

	// -----------------------
	ADAPI void Scene::OnRender() const
	{
		IEngineRenderable::OnRender();

		for (auto& camera : m_Cameras)
		{
			camera->OnRender();
			camera->OnRenderToTargets(this);
		}

		Utils::RuntimeCheckH(m_Device->BeginScene());
		Utils::RuntimeCheckH(m_Device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0u, 1.0f, 0));
		for (auto& camera : m_Cameras)
		{
			camera->OnRenderTargets();
		}
		Utils::RuntimeCheckH(m_Device->EndScene());
		Utils::RuntimeCheckH(m_Device->Present(nullptr, nullptr, nullptr, nullptr));
	}

}	// namespace Presentation2
