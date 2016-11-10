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
	ADAPI void Camera::Render() const
	{
		IEngineRenderable::Render();
		
		auto const projection = dxm::perspectiveFovLH<FLOAT>(FieldOfView, GetContextWidth(), GetContextHeight(), NearClippingPlane, FarClippingPlane);
		Utils::RuntimeCheckH(m_Device->SetTransform(D3DTS_PROJECTION, dxm::ptr(projection)));
	}
	
	// -----------------------
	ADAPI void OrbitalCamera::Update()
	{
		Camera::Update();

		if (GetAsyncKeyState(VK_LBUTTON) != 0)
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
		GetCursorPos(&m_PrevMousePosition);
	}

	// -----------------------
	ADAPI void OrbitalCamera::Render() const
	{
		Camera::Render();

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
		: m_Device(device)
	{
		assert(m_Device != nullptr);

		{
			/* Setting up default blending parameters. */
			Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD));
			Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
			Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
		}

		{
			/* Setting up default texture parameters. */
			Utils::RuntimeCheckH(m_Device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE));
			Utils::RuntimeCheckH(m_Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE));
			Utils::RuntimeCheckH(m_Device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE));
		}

		/* Setting up default lights. */
		D3DLIGHT9 light = {};
		light.Type = D3DLIGHT_DIRECTIONAL;
		light.Diffuse = { 0.7f, 0.7f, 0.7f, 1.0f };
		light.Direction = { 0.0f, -0.3f, 1.0f };
		light.Attenuation1 = 0.1f;
		Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_LIGHTING, TRUE));
		Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_AMBIENT, ~0u));
		Utils::RuntimeCheckH(m_Device->SetLight(0, &light));
	}

	// -----------------------
	ADAPI void Scene::Update()
	{
		IEngineRenderable::Update();
		for (auto const& updatable : m_Updatables)
		{
			updatable->Update();
		}
		for (auto const& renderable : m_Renderables)
		{
			renderable->Update();
		}
	}

	// -----------------------
	ADAPI void Scene::Render() const
	{
		IEngineRenderable::Render();

		/* Clearing the buffers and starting rendering. */
		Utils::RuntimeCheckH(m_Device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, ~0u, 1.0f, 0));
		Utils::RuntimeCheckH(m_Device->BeginScene());

		/* Rendering the scene. */
		for (auto const& renderable : m_Renderables)
		{
			renderable->Render();
		}

		/* Ending rendering and swapping buffers. */
		Utils::RuntimeCheckH(m_Device->EndScene());
		Utils::RuntimeCheckH(m_Device->Present(nullptr, nullptr, nullptr, nullptr));
	}

}	// namespace Presentation2
