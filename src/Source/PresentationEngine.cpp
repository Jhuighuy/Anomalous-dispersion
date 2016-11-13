// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// Copyright (C) 2016 Butakov Oleg, Plaxin Gleb.
// Computational Methods @ Computational Mathematics & Cybernetics, MSU.
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

#include "PresentationEngine.hpp"

#if SUPPORT_LOADING_FROM_FILE
#include <d3dx9.h>
#endif	// if SUPPORT_LOADING_FROM_FILE

namespace Presentation2
{
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //
	// Cameras.
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //
	
	ADINT auto static const Up = dxm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

	// -----------------------
	ADAPI Camera::Camera(IDirect3DDevice9* const device) 
		: m_Device(device), m_Rect{}
	{
		assert(m_Device != nullptr);

		D3DDEVICE_CREATION_PARAMETERS deviceCreationParameters = {};
		Utils::RuntimeCheckH(m_Device->GetCreationParameters(&deviceCreationParameters));
		Utils::RuntimeCheck(GetWindowRect(deviceCreationParameters.hFocusWindow, &m_Rect));
	}
	// -----------------------
	ADAPI void Camera::OnRender() const
	{
		IEngineRenderable::OnRender();
		
		auto const projection = dxm::perspectiveFovLH<FLOAT>(FieldOfView, GetContextWidth(), GetContextHeight(), NearClippingPlane, FarClippingPlane);
		Utils::RuntimeCheckH(m_Device->SetTransform(D3DTS_PROJECTION, dxm::ptr(projection)));
	}
	
	// -----------------------
	ADAPI void OrbitalCamera::OnUpdate()
	{
		Camera::OnUpdate();

		if (GetAsyncKeyState(VK_LBUTTON) != 0 && m_PrevMousePosition.x != 0 && m_PrevMousePosition.y != 0)
		{
			POINT mouseCurrentPosition = {};
			Utils::RuntimeCheck(GetCursorPos(&mouseCurrentPosition));
			if (PtInRect(&m_Rect, mouseCurrentPosition))
			{
				/* Mouse has been moved inside the context rect while left button has been pressed. */
				auto const deltaX = static_cast<FLOAT>(mouseCurrentPosition.y - m_PrevMousePosition.y) / GetContextWidth();
				auto const deltaY = static_cast<FLOAT>(mouseCurrentPosition.x - m_PrevMousePosition.x) / GetContextHeight();

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
		Camera::OnRender();

		auto const translation = dxm::translate(RotationCenter);
		auto const cameraRotation = dxm::toMat4(dxm::quat(dxm::vec3(Rotation, 0.0f)));
		auto const view = dxm::lookAtLH(dxm::vec3(translation * cameraRotation * dxm::vec4(CenterOffset, 1.0f))
			, RotationCenter, dxm::vec3(cameraRotation * Up));
		Utils::RuntimeCheckH(m_Device->SetTransform(D3DTS_VIEW, dxm::ptr(view)));
	}

	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //
	// Scenes and bridging.
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //

	// -----------------------
	ADAPI Scene::Scene(IDirect3DDevice9* const device)
		: m_Device(device), m_ForceBlackShader(nullptr), m_BlendTexturesShader(nullptr)
		, m_RenderTargets{ nullptr }
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
		for (auto const& renderable : m_Renderables)
		{
			renderable->OnUpdate();
		}
	}

	// -----------------------
	ADAPI void Scene::OnRender() const
	{
		IEngineRenderable::OnRender();

		TriangleMeshRendererPtr<> static screenQuad;
		if (m_ScreenQuad == nullptr)
		{
			for (auto i = 0; i < dxm::countof(m_RenderTargets); ++i)
			{
				Utils::RuntimeCheckH(D3DXCreateTexture(m_Device, 1440, 1080, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT, &m_RenderTargets[i].Texture));
				Utils::RuntimeCheckH(m_RenderTargets[i].Texture->GetSurfaceLevel(0, &m_RenderTargets[i].Surface));
			}

			/// @todo
			auto static screenQuadMesh = TriangleMesh(L"../gfx/quad.obj");
			m_ScreenQuad = std::make_shared<::Presentation2::TriangleMeshRenderer<TRUE>>(m_Device, screenQuadMesh, nullptr, L"../gfx/Shaders/ForceTransparent.hlsl");
			m_ForceBlackShader = m_ScreenQuad->m_PixelShader;
			m_ScreenQuad = std::make_shared<::Presentation2::TriangleMeshRenderer<TRUE>>(m_Device, screenQuadMesh, nullptr, L"../gfx/Shaders/BlendTexturesFXAA.hlsl");
			m_BlendTexturesShader = m_ScreenQuad->m_PixelShader;
			m_ScreenQuad->m_PixelShader = nullptr;
		}
		
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
				Utils::RuntimeCheckH(m_Device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0));
			}
		}

		{
			/* Setting up the transparency. */
			Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
			Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
			Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE));

			/* Rendering all the solid meshed in black shader. */
			for (auto const& renderable : m_Renderables)
			{
				auto const meshRenderer = std::dynamic_pointer_cast<IMeshRenderer>(renderable);
				if (meshRenderer != nullptr)
				{
					if (!meshRenderer->IsTransparent())
					{
						meshRenderer->OnRender(m_ForceBlackShader);
					}
				}
				else
				{
					renderable->OnRender();
				}
			}
			/* And transparent ones as they are.. */
			for (auto const& renderable : m_Renderables)
			{
				auto const meshRenderer = std::dynamic_pointer_cast<IMeshRenderer>(renderable);
				if (meshRenderer != nullptr && meshRenderer->IsTransparent())
				{
					renderable->OnRender();
				}
			}
		}

		{
			/* Ending our scene. */
			Utils::RuntimeCheckH(m_Device->EndScene());
		}

		// -------------------------------------------------------------------------------------------------------------------
		// Pass 2: rendering only solid objects.
		// -------------------------------------------------------------------------------------------------------------------

		{
			/* Setting our second render target. */
			Utils::RuntimeCheckH(m_Device->SetRenderTarget(0, m_RenderTargets[1].Surface));

			{	/* Preparing for the second pass.. */
				Utils::RuntimeCheckH(m_Device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00FFFFFF, 1.0f, 0));
				Utils::RuntimeCheckH(m_Device->BeginScene());
			}
		}

		{
			/* Setting up the transparency. */
			Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));

			/* Rendering all the solid meshed in black shader and transparent as they are. */
			for (auto const& renderable : m_Renderables)
			{
				auto const meshRenderer = std::dynamic_pointer_cast<IMeshRenderer>(renderable);
				if (meshRenderer == nullptr || !meshRenderer->IsTransparent())
				{
					renderable->OnRender();
				}
			}
		}

		{
			/* Ending our scene. */
			Utils::RuntimeCheckH(m_Device->EndScene());
		}

		// -------------------------------------------------------------------------------------------------------------------
		// Pass 3: merging passes 1 and 2.
		// -------------------------------------------------------------------------------------------------------------------

		{
			/* Restoring original render targets. */
			Utils::RuntimeCheckH(m_Device->SetRenderTarget(0, backBuffer));

			{	/* Preparing for the third pass.. */
				Utils::RuntimeCheckH(m_Device->BeginScene());
				Utils::RuntimeCheckH(m_Device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00FFFFFF, 1.0f, 0));
			}
		}

		/* Blending results of two passes and performing anti-aliasing. */
		{
			/* Setting up new view and projection. */
			auto const viewMatrix = dxm::mat4();
			auto const projectionMatrix = dxm::orthoLH(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);

			Utils::RuntimeCheckH(m_Device->SetTransform(D3DTS_PROJECTION, dxm::ptr(projectionMatrix)));
			Utils::RuntimeCheckH(m_Device->SetTransform(D3DTS_VIEW, dxm::ptr(viewMatrix)));

			/* We want our picture been blended into the screen. */
			Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO));
			Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR));

			/* We do not need any culling for now. */
			Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
			Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_ZENABLE, FALSE));
			m_ScreenQuad->OnRender(m_BlendTexturesShader, m_RenderTargets[0].Texture, m_RenderTargets[1].Texture);
			Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW));
			Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_ZENABLE, TRUE));
		}

		{
			/* Ending rendering and swapping buffers. */
			Utils::RuntimeCheckH(m_Device->EndScene());
			Utils::RuntimeCheckH(m_Device->Present(nullptr, nullptr, nullptr, nullptr));
		}
	}

}	// namespace Presentation2
