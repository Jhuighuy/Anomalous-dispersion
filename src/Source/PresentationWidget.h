// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// Copyright (C) 2016 Butakov Oleg, Plaxin Gleb.
// Computational Methods @ Computational Mathematics & Cybernetics, MSU.
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
#pragma once
#include "yx2Engine.h"
#include "Presentation.h"
#include "PresentationEngine.h"
#include <ctime>

namespace Presentation1
{
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

	typedef FLOAT(*RefractiveIndexFunc)(FLOAT const waveLength);

	static FLOAT DummyRefractiveIndex(FLOAT const waveLength)
	{
		(void)waveLength;
		return 1.0f;
	}

	static FLOAT AirGlassRefractiveIndex(FLOAT const waveLength)
	{
		auto static const violetWaveLength = 380.0f;
		auto static const redWaveLength = 740.0f;

		auto static const redRefractiveIndex = 1.550f;
		auto static const violetRefractiveIndex = 1.510f;

		auto const v = (redWaveLength - waveLength) / (redWaveLength - violetWaveLength);
		return violetRefractiveIndex - v * (violetRefractiveIndex - redRefractiveIndex);
	}
	static FLOAT GlassAirRefractiveIndex(FLOAT const waveLength)
	{
		return 1.0f / AirGlassRefractiveIndex(waveLength);
	}

	static FLOAT AirGovnoRefractiveIndex(FLOAT const waveLength)
	{
		/// @todo Implement me.
		return 0.3 * (
			0.0000000000098913823223182 * pow(waveLength, 5.0)
			+ -0.0000000289763923941455186 * pow(waveLength, 4.0)
			+ +0.0000333125484955842371898  * pow(waveLength, 3.0)
			+ -0.0187615519724694658533727  * pow(waveLength, 2.0)
			+ +5.1727621842059236445914745  * waveLength
			+ -557.0298741325654066692800118);
	}
	static FLOAT GovnoAirRefractiveIndex(FLOAT const waveLength)
	{
		return 1.0f / AirGovnoRefractiveIndex(waveLength);
	}

	struct Plane final
	{
		dxm::vec3 PointMin, PointMax;
		dxm::vec3 Normal;
		RefractiveIndexFunc RefractiveIndex;
		bool IsScreen;

		// -----------------------
		bool Intersect(dxm::vec3& intersectionPoint, dxm::vec3 const& coord, dxm::vec3 const& direction) const
		{
			auto const dp1 = dxm::dot(PointMin - coord, Normal);
			auto const dp2 = dxm::dot(direction, Normal);
			if (dp2 != 0.0)
			{
				auto const v = dp1 / dp2;
				intersectionPoint = coord + v * direction;
				return PointMin <= intersectionPoint && intersectionPoint >= PointMax;
			}
			return false;
		}

		// -----------------------
		dxm::vec3 Refract(dxm::vec3 const& direction, FLOAT const waveLength) const
		{
			auto const angleBefore = acosf(dxm::dot(direction, Normal) / dxm::length(Normal) / dxm::length(direction));
			auto const angleAfter = asinf(dxm::clamp(1.0f /RefractiveIndex(waveLength) * sinf(angleBefore), -1.0f, 1.0f));
			return dxm::rotate(direction, angleBefore - angleAfter, dxm::cross(direction, Normal));
		}
	};	// struct Plane

	enum class PrismType
	{
		Air,
		Govno,
		Count,
	};	// enum class PrismType

	struct Prism final
	{
		PrismType Type = PrismType::Air;
		dxm::vec3 Position;
		FLOAT Angle = DXM_PI / 3.0f;
		FLOAT RotationX = 0.0f;// -F_PI / 12.0f;
		FLOAT RotationZ = 0.0f;//-F_PI / 3.0f;

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
		void UpdatePlanes(std::vector<Plane>& planes) const
		{
			struct { RefractiveIndexFunc In, Out; } static const refractiveIndexFuncsTable[] = {
				{ &AirGlassRefractiveIndex, &GlassAirRefractiveIndex },
				{ &AirGovnoRefractiveIndex, &GovnoAirRefractiveIndex },
			};
			
			auto const& refractiveIndexFuncs = refractiveIndexFuncsTable[static_cast<size_t>(Type)];
			auto const transformationMatrix = dxm::translate(Position)
				* dxm::yawPitchRoll(RotationX, 0.0f, RotationZ) * dxm::scale(/*2.0f * */glm::vec3(1.0f, 1.0f, tanf(Angle / 2.0f)));
			{
				auto const p1 = transformationMatrix * dxm::vec4(-0.1f, -0.1f, +0.0f, 1.0);
				auto const p2 = transformationMatrix * dxm::vec4(+0.1f, -0.1f, +0.0f, 1.0);
				auto const p3 = transformationMatrix * dxm::vec4(+0.1f, +0.1f, -0.2f, 1.0);
				auto const normal = dxm::cross(dxm::vec3(p2 - p1), dxm::vec3(p3 - p1));
				planes.push_back({ p1, p3, normal, refractiveIndexFuncs.In });
			}

			{
				auto const p1 = transformationMatrix * dxm::vec4(-0.1f, -0.1f, +0.0f, 1.0);
				auto const p2 = transformationMatrix * dxm::vec4(+0.1f, -0.1f, +0.0f, 1.0);
				auto const p3 = transformationMatrix * dxm::vec4(-0.1f, +0.1f, +0.2f, 1.0);
				auto const normal = dxm::cross(dxm::vec3(p2 - p1), dxm::vec3(p3 - p1));
				planes.push_back({ p1, p3, normal, refractiveIndexFuncs.Out });
			}
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
			m_PrismMesh.Rotation.x = RotationX;
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

		TriangleMutableMesh m_ScreenMesh;
		TriangleMutableMeshRenderer<> m_ScreenRenderer;

		bool m_AreRaysSynced = false;
		LineMutableMesh m_RaysMesh;
		LineMutableMeshRenderer<TRUE> m_RaysRenderer;
		TriangleMutableMesh m_RaysProjectionMesh;
		TriangleMutableMeshRenderer<TRUE> m_RaysProjectionRenderer;

		TriangleMutableMesh m_PrismMesh;
		TriangleMutableMesh m_PrismHolderBase, m_PrismHolderLeg, m_PrismHolderGimbal;
		std::vector<Prism> m_PrismRenderers;
		std::vector<Plane> m_PrismPlanes;

	public:
		explicit PresentationWidget(HWND const hwnd, IDirect3DDevice9* const device)
			: Runtime(hwnd, device)
			, m_Camera(device)
			// -----------------------
			, m_RoomMesh(device), m_RoomRenderer(device, m_RoomMesh)
			// -----------------------
			, m_ScreenMesh(device), m_ScreenRenderer(device, m_ScreenMesh)
			// -----------------------
			, m_RaysMesh(device), m_RaysRenderer(device, m_RaysMesh)
			, m_RaysProjectionMesh(device), m_RaysProjectionRenderer(device, m_RaysProjectionMesh)
			// -----------------------
			, m_PrismMesh(device)
			, m_PrismHolderBase(device), m_PrismHolderLeg(device), m_PrismHolderGimbal(device)
		{
			/* Setting up default alpha-blending. */
			m_Device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

			/* Setting up default lights. */
			m_Device->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(0x50, 0x50, 0x50));
			D3DLIGHT9 spotLight = {};
			spotLight.Type = D3DLIGHT_SPOT;
			spotLight.Diffuse = { 250.0f / 255.0f, 130.0f / 255.0f, 82.0f / 255.0f, 1.0f};
			spotLight.Position = { 2.8f, 120.0f, 0.0f };
			spotLight.Direction = { -1.0f, 0.0f, 0.0f };
			spotLight.Range = 4.32f;
			spotLight.Falloff = 1.0f;
			spotLight.Theta = 4.0f / DXM_PI * 180.0f;
			spotLight.Phi = 52.5f / DXM_PI * 180.0f;
			spotLight.Attenuation0 = 0.1f;
			m_Device->SetLight(0, &spotLight);

			/* Setting up default texture parameters. */
			m_Device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			m_Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

			/* Setting up static scene parameters. */
			LoadOBJ("../../gfx/room.obj", m_RoomMesh);
			LoadTexture(m_Device, L"../../gfx/roomLightMap.png", &m_RoomRenderer.Texture);
			m_RoomRenderer.Position.z = 2.0f;
			m_RoomRenderer.Rotation.x = DXM_PI;
			LoadOBJ("../../gfx/screen.obj", m_ScreenMesh);
			LoadTexture(m_Device, L"../../gfx/screenLightMap.png", &m_ScreenRenderer.Texture);
			m_ScreenRenderer.Position.z = 2.0f;
			m_ScreenRenderer.Rotation.x = DXM_PI;
		
			/* Setting up dynamic scene parameters. */
			LoadOBJ("../../gfx/prism.obj", m_PrismMesh, 0xFF/3);
			LoadOBJ("../../gfx/holder_base.obj", m_PrismHolderBase);
			LoadOBJ("../../gfx/holder_leg.obj", m_PrismHolderLeg);
			LoadOBJ("../../gfx/holder_gimbal.obj", m_PrismHolderGimbal);
			
			m_PrismRenderers.push_back({ m_Device, m_PrismMesh, m_PrismHolderBase, m_PrismHolderLeg, m_PrismHolderGimbal });
			m_PrismRenderers.push_back({ m_Device, m_PrismMesh, m_PrismHolderBase, m_PrismHolderLeg, m_PrismHolderGimbal });
			m_PrismRenderers[0].Position = { 0.0f, 0.5f, 1.0f };
			m_PrismRenderers[1].Type = PrismType::Govno;
			m_PrismRenderers[1].Angle = DXM_PI / 4.0f;
			m_PrismRenderers[1].Position = { 0.0f, 1.0f, 2.0f };
			m_PrismRenderers[1].RotationZ = DXM_PI / 2.0f;
			for (auto& prism : m_PrismRenderers)
			{
				prism.UpdatePlanes(m_PrismPlanes);
			}
			m_PrismPlanes.push_back({ { 0.0f, 0.0f, 3.49f },{ 0.0f, 0.0f, 3.0f }, { 0.0f, 0.0f, 1.0f }, &DummyRefractiveIndex });

			/* Setting up some other shit. */
			LoadTexture(m_Device, L"../../gfx/color_mask.png", &m_RaysProjectionRenderer.Texture);
			LoadShader(m_Device, L"../../gfx/ColoredTextureShader.hlsl", &m_RaysProjectionRenderer.PixelShader);
		}

		// -----------------------
		void Update()
		{
			m_Device->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
			m_Device->Clear(0, nullptr, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
			m_Device->BeginScene();
			{
				/* Updating camera first. */
				m_Camera.Update();

				/* Rendering scene. */
				m_RoomRenderer.Render();
				m_ScreenRenderer.Render();

				/* Updating and rendering rays. */
				if (!m_AreRaysSynced)
				{
					m_AreRaysSynced = true;
					GenerateRaysMesh(10000);
				}
			//	m_RaysRenderer.Render();
				m_RaysProjectionRenderer.Render();

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
		void GenerateRaysMesh(UINT const partitioning)
		{
			for (auto i = 0u; i < partitioning; ++i)
			{
				auto coord = dxm::vec3(0.0f, 1.5f, -2.0f);
				auto direction = m_PrismRenderers[0].Position - coord;

				auto static const violetWaveLength = 380.0f;
				auto static const redWaveLength = 740.0f;
				auto const waveLength = violetWaveLength + i * (redWaveLength - violetWaveLength) / partitioning;
				auto const rgb = Presentation::ConvertWaveLengthToRGB(waveLength);

				if (waveLength >= 560.0f && waveLength <= 620.0f)
					continue;

				for (auto j = 0u; j < m_PrismPlanes.size(); ++j)
				{
					auto& plane = m_PrismPlanes[j];

					m_RaysMesh.AddVertex({ coord, rgb });
					plane.Intersect(coord, coord, direction);
					m_RaysMesh.AddVertex({ coord, rgb });
					direction = plane.Refract(direction, waveLength);

					if (j == m_PrismPlanes.size() - 1)
					{
						auto const scale = 0.09f;
						dxm::vec3 static const uvOffset = { 0.5f, 0.5f, 0.0f };
						dxm::vec3 static const triangleVert[] = {
							{ -0.5f, +0.5f, 0.0f },
							{ +0.5f, -0.5f, 0.0f },
							{ -0.5f, -0.5f, 0.0f },

							{ -0.5f, +0.5f, 0.0f },
							{ +0.5f, +0.5f, 0.0f },
							{ +0.5f, -0.5f, 0.0f },
						};
						for (auto k = 0u; k < dxm::countof(triangleVert); ++k)
						{
							m_RaysProjectionMesh.AddVertex({ triangleVert[k] * scale + coord,{ 0.0, 0.0, -1.0f }, rgb, triangleVert[k] + uvOffset });
						}
					}
				}
			}
			m_AreRaysSynced = true;
		}

	}; // class PresentationWidget

} // namespace Presentation
