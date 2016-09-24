// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// Copyright (C) 2016 Butakov Oleg, Plaxin Gleb.
// Computational Methods @ Computational Mathematics & Cybernetics, MSU.
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
#pragma once
#include "yx2Engine.h"
#include "Presentation.h"
#include "PresentationEngine.h"

namespace Presentation1
{
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

	struct Prism
	{
		dxm::vec3 Position = { 0.0f, 0.6f, 2.0f };
		FLOAT Angle = DXM_PI / 3.0f;
		FLOAT RotationY = -F_PI / 12.0f;
		FLOAT RotationZ = -F_PI / 3.0f;

	private:
		TriangleMutableMeshRenderer<TRUE> m_PrismMesh;
		TriangleMutableMeshRenderer<FALSE, TRUE> m_PrismHolderBase, m_PrismHolderLeg, m_PrismHolderGimbal;

	public:
		Prism(IDirect3DDevice9* const device, TriangleMutableMesh const& prismMesh
			, TriangleMutableMesh const& prismHolderBase, TriangleMutableMesh const& prismHolderLeg, TriangleMutableMesh const& prismHolderGimbal)
			: m_PrismMesh(device, prismMesh)
			, m_PrismHolderBase(device, prismHolderBase), m_PrismHolderLeg(device, prismHolderLeg), m_PrismHolderGimbal(device, prismHolderGimbal)
		{
		}

		// -----------------------
		void Update() 
		{
			auto static const LegHeight = 1.5f;
			auto static const GimbalHeight = 0.2f;

			m_PrismHolderBase.Position = { Position.x, 0.0f, Position.z };
			m_PrismHolderBase.Render();

			m_PrismHolderLeg.Position = { Position.x, Position.y - LegHeight - GimbalHeight, Position.z };
			m_PrismHolderLeg.Render();
			
			m_PrismHolderGimbal.Position = { Position.x, Position.y, Position.z };
			m_PrismHolderGimbal.Rotation.z = RotationZ;
			m_PrismHolderGimbal.Render();

			m_PrismMesh.Position = Position;
			m_PrismMesh.Scale = glm::vec3(1.0f, 1.0f, tanf(Angle / 2.0f));
			m_PrismMesh.Rotation.y = RotationY;
			m_PrismMesh.Rotation.z = RotationZ;
			m_PrismMesh.Render();
		}
	};	// struct Prism

	class PresentationWidget final : public yx2::engine::Runtime
	{
	private:
		OrbitalCamera m_Camera;

		TriangleMutableMesh m_RoomMesh;
		TriangleMutableMeshRenderer<> m_RoomRenderer;

		bool m_AreRaysSynced = false;
		LineMutableMesh m_RaysMesh;
		LineMutableMeshRenderer<TRUE> m_RaysRenderer;
		TriangleMutableMesh m_RaysProjectionMesh;
		TriangleMutableMeshRenderer<TRUE> m_RaysProjectionRenderer;

		TriangleMutableMesh m_PrismMesh;
		TriangleMutableMesh m_PrismHolderBase, m_PrismHolderLeg, m_PrismHolderGimbal;
		std::vector<Prism> m_PrismRenderers;

	public:
		explicit PresentationWidget(HWND const hwnd, IDirect3DDevice9* const device)
			: Runtime(hwnd, device)
			, m_Camera(device)
			// -----------------------
			, m_RoomMesh(device), m_RoomRenderer(device, m_RoomMesh)
			// -----------------------
			, m_RaysMesh(device), m_RaysRenderer(device, m_RaysMesh)
			, m_RaysProjectionMesh(device), m_RaysProjectionRenderer(device, m_RaysProjectionMesh)
			// -----------------------
			, m_PrismMesh(device)
			, m_PrismHolderBase(device), m_PrismHolderLeg(device), m_PrismHolderGimbal(device)
		{
			/* Setting up default alpha-blending. */
			m_Device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

			/* Setting up default texture parameters. */
			m_Device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			m_Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

			/* Setting up static scene parameters. */
			LoadOBJ("../../gfx/room.obj", m_RoomMesh);
			LoadTexture(m_Device, L"../../gfx/baked.png", &m_RoomRenderer.Texture);
			m_RoomRenderer.Position = {0.0f, 0.0f, 2.0f};
		//	m_RoomRenderer.Rotation.x = DXM_PI / 2.0f;
		
			/* Setting up dynamic scene parameters. */
			LoadOBJ("../../gfx/prism.obj", m_PrismMesh);
			LoadOBJ("../../gfx/holder_base.obj", m_PrismHolderBase);
			LoadOBJ("../../gfx/holder_leg.obj", m_PrismHolderLeg);
			LoadOBJ("../../gfx/holder_gimbal.obj", m_PrismHolderGimbal);
			m_PrismRenderers.push_back({ m_Device, m_PrismMesh, m_PrismHolderBase, m_PrismHolderLeg, m_PrismHolderGimbal });

			/* Setting up some other shit. */
			LoadTexture(m_Device, L"../../gfx/color_mask.png", &m_RaysProjectionRenderer.Texture);
			LoadShader(m_Device, L"../../gfx/ColoredTextureShader.hlsl", &m_RaysProjectionRenderer.PixelShader);
		}

		// -----------------------
		void Update()
		{
			m_Device->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 40), 1.0f, 0);
			m_Device->Clear(0, nullptr, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
			m_Device->BeginScene();
			{
				/* Updating camera first. */
				m_Camera.Update();

				/* Rendering scene. */
				m_RoomRenderer.Render();

				/* Updating and rendering rays. */
				if (!m_AreRaysSynced)
				{
					m_AreRaysSynced = true;
					GenerateRaysMesh(100);
				}
			//	m_RaysMeshRenderer.Render();
			//	m_RaysProjectionMeshRenderer.Render();

				/* Updating and rendering prisms and holders. */
				for (auto& prism : m_PrismRenderers)
				{
					prism.Update();
				}
			}
			m_Device->EndScene();
			m_Device->Present(nullptr, nullptr, nullptr, nullptr);
		}

		// -----------------------
		void GenerateRaysMesh(unsigned partitioning)
		{
			(void)partitioning;
			m_AreRaysSynced = true;
		}

	}; // class PresentationWidget

} // namespace Presentation
