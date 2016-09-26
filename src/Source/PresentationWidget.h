// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// Copyright (C) 2016 Butakov Oleg, Plaxin Gleb.
// Computational Methods @ Computational Mathematics & Cybernetics, MSU.
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
#pragma once
#include "yx2Engine.h"
#include "Presentation.h"
#include "PresentationEngine.h"
#include <ctime>
#include <algorithm>

namespace Presentation1
{
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

	typedef DOUBLE(*RefractiveIndexFunc)(DOUBLE const waveLength);

	static DOUBLE DummyRefractiveIndex(DOUBLE const waveLength)
	{
		(void)waveLength;
		return 1.0f;
	}

	static DOUBLE AirGlassRefractiveIndex(DOUBLE const waveLength)
	{
		auto static const violetWaveLength = 380.0f;
		auto static const redWaveLength = 780.0f;

		auto static const violetRefractiveIndex = 1.550f;
		auto static const redRefractiveIndex = 1.510f;

		auto const v = (waveLength - violetWaveLength) / (redWaveLength - violetWaveLength);
		return violetRefractiveIndex + v * (redRefractiveIndex - violetRefractiveIndex);
	}
	static DOUBLE GlassAirRefractiveIndex(DOUBLE const waveLength)
	{
		return 1.0f / AirGlassRefractiveIndex(waveLength);
	}

	static DOUBLE GovnoAirRefractiveIndex(DOUBLE const waveLength)
	{
		/// @todo Implement me.
		float static const grid[] = {
			0.38 * 0 + 1.6,			0.38404 * 0 + 1.59588,  0.388081 * 0 + 1.59145, 0.392121 * 0 + 1.58665, 0.396162 * 0 + 1.58156, 0.400202 * 0 + 1.57621, 0.404242 * 0 + 1.5706,
			0.408283 * 0 + 1.56468, 0.412323 * 0 + 1.55836, 0.416364 * 0 + 1.55151, 0.420404 * 0 + 1.544,   0.424444 * 0 + 1.53573, 0.428485 * 0 + 1.52658, 0.432525 * 0 + 1.51646,
			0.436566 * 0 + 1.5053,  0.440606 * 0 + 1.49304, 0.444646 * 0 + 1.47965, 0.448687 * 0 + 1.46511, 0.452727 * 0 + 1.44944, 0.456768 * 0 + 1.43268, 0.460808 * 0 + 1.41492,
			0.464848 * 0 + 1.39627, 0.468889 * 0 + 1.37692, 0.472929 * 0 + 1.35706, 0.47697 * 0 + 1.33696,  0.48101 * 0 + 1.31688,  0.485051 * 0 + 1.29717, 0.489091 * 0 + 1.27815,
			0.493131 * 0 + 1.26019, 0.497172 * 0 + 1.24364, 0.501212 * 0 + 1.22886, 0.505253 * 0 + 1.21618, 0.509293 * 0 + 1.20592, 0.513333 * 0 + 1.19835, 0.517374 * 0 + 1.19372,
			0.521414 * 0 + 1.19225, 0.525455 * 0 + 1.19408, 0.529495 * 0 + 1.19935, 0.533535 * 0 + 1.20814, 0.537576 * 0 + 1.22049, 0.541616 * 0 + 1.23641, 0.545657 * 0 + 1.25589,
			0.549697 * 0 + 1.27887, 0.553737 * 0 + 1.30526, 0.557778 * 0 + 1.33497, 0.561818 * 0 + 1.36783, 0.565859 * 0 + 1.40368, 0.569899 * 0 + 1.44231, 0.573939 * 0 + 1.48346,
			0.57798 * 0 + 1.52684,  0.58202 * 0 + 1.57209,  0.586061 * 0 + 1.61884, 0.590101 * 0 + 1.66663, 0.594141 * 0 + 1.715,   0.598182 * 0 + 1.76341, 0.602222 * 0 + 1.81131,
			0.606263 * 0 + 1.85815, 0.610303 * 0 + 1.90333, 0.614343 * 0 + 1.94632, 0.618384 * 0 + 1.98657, 0.622424 * 0 + 2.02361, 0.626465 * 0 + 2.05702, 0.630505 * 0 + 2.08646,
			0.634545 * 0 + 2.11169, 0.638586 * 0 + 2.13256, 0.642626 * 0 + 2.14903, 0.646667 * 0 + 2.16118, 0.650707 * 0 + 2.16915, 0.654747 * 0 + 2.17322, 0.658788 * 0 + 2.17371,
			0.662828 * 0 + 2.17101, 0.666869 * 0 + 2.16555, 0.670909 * 0 + 2.15777, 0.674949 * 0 + 2.14813, 0.67899 * 0 + 2.13704,  0.68303 * 0 + 2.1249,   0.687071 * 0 + 2.11206,
			0.691111 * 0 + 2.0988,  0.695152 * 0 + 2.08535, 0.699192 * 0 + 2.07189, 0.703232 * 0 + 2.05856, 0.707273 * 0 + 2.04543, 0.711313 * 0 + 2.03255, 0.715354 * 0 + 2.01997,
			0.719394 * 0 + 2.00769, 0.723434 * 0 + 1.99571, 0.727475 * 0 + 1.98404, 0.731515 * 0 + 1.97269, 0.735556 * 0 + 1.96165, 0.739596 * 0 + 1.95092, 0.743636 * 0 + 1.94052,
			0.747677 * 0 + 1.93043, 0.751717 * 0 + 1.92065, 0.755758 * 0 + 1.91118, 0.759798 * 0 + 1.90202, 0.763838 * 0 + 1.89316, 0.767879 * 0 + 1.88458, 0.771919 * 0 + 1.87625,
			0.77596 * 0 + 1.86809,  0.78 * 0 + 1.86
		};

		auto static const violetWaveLength = 380.0f;
		auto static const redWaveLength = 780.0f;

		auto const v = (waveLength - violetWaveLength) / (redWaveLength - violetWaveLength);
		auto const i = size_t(v * (dxm::countof(grid) - 1));
		if (i == dxm::countof(grid) - 1)
			return grid[dxm::countof(grid) - 1];

		auto const yi = grid[i], yi1 = grid[i + 1];
		auto const xi = violetWaveLength + i * (redWaveLength - violetWaveLength) / dxm::countof(grid);
		auto const xi1 = violetWaveLength + (i + 1) * (redWaveLength - violetWaveLength) / dxm::countof(grid);
		auto y = yi + (waveLength - xi) / (xi1 - xi) * (yi1 - yi);
		return y;
	}
	static DOUBLE AirGovnoRefractiveIndex(DOUBLE const waveLength)
	{
		return 1.0f / GovnoAirRefractiveIndex(waveLength);
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
		dxm::vec3 Refract(dxm::vec3 const& direction, DOUBLE const waveLength) const
		{
			auto const angleBefore = acosf(dxm::dot(direction, Normal) / dxm::length(Normal) / dxm::length(direction));
			auto const angleAfter = asinf(dxm::clamp(1.0 / RefractiveIndex(waveLength) * sin(angleBefore), -1.0, 1.0));
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
			LoadOBJ("../gfx/room.obj", m_RoomMesh);
			LoadTexture(m_Device, L"../gfx/roomLightMap.png", &m_RoomRenderer.Texture);
			m_RoomRenderer.Position.z = 2.0f;
			m_RoomRenderer.Rotation.x = DXM_PI;
			LoadOBJ("../gfx/screen.obj", m_ScreenMesh);
			LoadTexture(m_Device, L"../gfx/screenLightMap.png", &m_ScreenRenderer.Texture);
			m_ScreenRenderer.Position.z = 2.0f;
			m_ScreenRenderer.Rotation.x = DXM_PI;
		
			/* Setting up dynamic scene parameters. */
			LoadOBJ("../gfx/prism.obj", m_PrismMesh, 0xFF/3);
			LoadOBJ("../gfx/holder_base.obj", m_PrismHolderBase);
			LoadOBJ("../gfx/holder_leg.obj", m_PrismHolderLeg);
			LoadOBJ("../gfx/holder_gimbal.obj", m_PrismHolderGimbal);
			
			m_PrismRenderers.push_back({ m_Device, m_PrismMesh, m_PrismHolderBase, m_PrismHolderLeg, m_PrismHolderGimbal });
			m_PrismRenderers.push_back({ m_Device, m_PrismMesh, m_PrismHolderBase, m_PrismHolderLeg, m_PrismHolderGimbal });
			m_PrismRenderers[0].Position = { 0.0f, 0.5f, 1.0f };
			m_PrismRenderers[1].Type = PrismType::Govno;
			m_PrismRenderers[1].Angle = DXM_PI / 6.0f;
			m_PrismRenderers[1].Position = { 0.0f, 0.9f, 2.0f };
			m_PrismRenderers[1].RotationZ = DXM_PI / 2.0f;
			for (auto& prism : m_PrismRenderers)
			{
				prism.UpdatePlanes(m_PrismPlanes);
			}
			m_PrismPlanes.push_back({ { 0.0f, 0.0f, 3.49f },{ 0.0f, 0.0f, 3.0f }, { 0.0f, 0.0f, 1.0f }, &DummyRefractiveIndex });

			/* Setting up some other shit. */
			LoadTexture(m_Device, L"../gfx/color_mask.png", &m_RaysProjectionRenderer.Texture);
			LoadShader(m_Device, L"../gfx/ColoredTextureShader.hlsl", &m_RaysProjectionRenderer.PixelShader);
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
					GenerateRaysMesh(1000);
				}
				m_RaysProjectionRenderer.Render();
				m_RaysRenderer.Render();

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

				auto static const violetWaveLength = 380.0;
				auto static const redWaveLength = 740.0;
				auto const waveLength = violetWaveLength + i * (redWaveLength - violetWaveLength) / partitioning;
				auto const rgb = Presentation::ConvertWaveLengthToRGB(waveLength);

				if (waveLength >= 550.0f && waveLength <= 620.0f)
					continue;

				for (auto j = 0u; j < m_PrismPlanes.size(); ++j)
				{
					auto& plane = m_PrismPlanes[j];
					auto static const eps = 0.00001;
					auto const d = (plane.RefractiveIndex(waveLength + eps) - plane.RefractiveIndex(waveLength)) / eps;
				//	if (d > 0)
				//		break;

					m_RaysMesh.AddVertex({ coord, rgb });
					plane.Intersect(coord, coord, direction);
					m_RaysMesh.AddVertex({ coord, j == m_PrismPlanes.size() - 1 ? rgb & 0x00FFFFFF : rgb });
					direction = plane.Refract(direction, waveLength);

					if (j == m_PrismPlanes.size() - 1)
					{
						auto const irgb = rgb & 0x00FFFFFF | 0x03000000;
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
							m_RaysProjectionMesh.AddVertex({ triangleVert[k] * scale + coord,{ 0.0, 0.0, -1.0f }, irgb, triangleVert[k] + uvOffset });
						}
					}
				}
			}
			m_AreRaysSynced = true;
		}

	}; // class PresentationWidget

} // namespace Presentation
