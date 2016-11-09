// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// Copyright (C) 2016 Butakov Oleg, Plaxin Gleb.
// Computational Methods @ Computational Mathematics & Cybernetics, MSU.
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
#pragma once
#include "PresentationEngine.hpp"

#include <ctime>
#define USE_RAYS_RENDERING 1
#ifndef USE_RAYS_RENDERING
#define USE_RAYS_RENDERING 0
#endif

#pragma warning(push, 0)
#include <glm/gtx/rotate_vector.hpp>
#pragma warning(pop)

#include <array>
typedef std::array<double, 3> Rgb;

inline Rgb cie1931WavelengthToXYZFit(double wavelength) {
	double wave = wavelength;

	double x;
	{
		double t1 = (wave - 442.0) * ((wave < 442.0) ? 0.0624 : 0.0374);
		double t2 = (wave - 599.8) * ((wave < 599.8) ? 0.0264 : 0.0323);
		double t3 = (wave - 501.1) * ((wave < 501.1) ? 0.0490 : 0.0382);

		x = 0.362 * exp(-0.5 * t1 * t1)
			+ 1.056 * exp(-0.5 * t2 * t2)
			- 0.065 * exp(-0.5 * t3 * t3);
	}

	double y;
	{
		double t1 = (wave - 568.8) * ((wave < 568.8) ? 0.0213 : 0.0247);
		double t2 = (wave - 530.9) * ((wave < 530.9) ? 0.0613 : 0.0322);

		y = 0.821 * exp(-0.5 * t1 * t1)
			+ 0.286 * exp(-0.5 * t2 * t2);
	}

	double z;
	{
		double t1 = (wave - 437.0) * ((wave < 437.0) ? 0.0845 : 0.0278);
		double t2 = (wave - 459.0) * ((wave < 459.0) ? 0.0385 : 0.0725);

		z = 1.217 * exp(-0.5 * t1 * t1)
			+ 0.681 * exp(-0.5 * t2 * t2);
	}

	return Rgb{ x, y, z };
}
inline double srgbXYZ2RGBPostprocess(double c) {
	// clip if c is out of range
	c = c > 1 ? 1 : (c < 0 ? 0 : c);

	// apply the color component transfer function
	c = c <= 0.0031308 ? c * 12.92 : 1.055 * pow(c, 1. / 2.4) - 0.055;

	return c;
}
inline Rgb srgbXYZ2RGB(Rgb xyz) {
	double x = xyz[0];
	double y = xyz[1];
	double z = xyz[2];

	double rl = 3.2406255 * x + -1.537208  * y + -0.4986286 * z;
	double gl = -0.9689307 * x + 1.8757561 * y + 0.0415175 * z;
	double bl = 0.0557101 * x + -0.2040211 * y + 1.0569959 * z;

	return {
			srgbXYZ2RGBPostprocess(rl),
			srgbXYZ2RGBPostprocess(gl),
			srgbXYZ2RGBPostprocess(bl)
	};
}
inline D3DCOLOR wavelengthToRGB(double wavelength) {
	Rgb xyz = cie1931WavelengthToXYZFit(wavelength);
	Rgb rgb = srgbXYZ2RGB(xyz);

	D3DCOLOR co = D3DCOLOR_ARGB(0xFF / 8,
		(D3DCOLOR)(rgb[0] * 0xFF), (D3DCOLOR)(rgb[1] * 0xFF), (D3DCOLOR)(rgb[2] * 0xFF)
	);
	return co;
}

namespace Presentation2
{
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

	typedef DOUBLE(*IndexFunc)(DOUBLE const waveLength);

	static DOUBLE DummyIndex(DOUBLE const waveLength)
	{
		(void)waveLength;
		return 1.0f;
	}

	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
	static DOUBLE GovnoAbsorptionIndex(DOUBLE const waveLength)
	{
		auto const x = waveLength / 1000.0;
		auto const y = 0.55 * exp(-40 * M_PI * (x - 0.58) * (x - 0.58));
		return ((x > 0.61414) && ( x < 0.691326 )) ? dxm::clamp(1.0 - 1.5743*y, 0.0, 1.0) * 0.9 :dxm::clamp(1.0 - y, 0.0, 1.0) * 0.9;
	}

	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
	static DOUBLE AirGlassRefractiveIndex(DOUBLE const waveLength)
	{
		/*auto static const violetWaveLength = 380.0;
		auto static const redWaveLength = 780.0;

		auto static const violetRefractiveIndex = 1.550;
		auto static const redRefractiveIndex = 1.510;

		auto const v = (waveLength - violetWaveLength) / (redWaveLength - violetWaveLength);
		return (violetRefractiveIndex + v * (redRefractiveIndex - violetRefractiveIndex)); */
		auto const x = 0.001 * waveLength;
		auto const y = 0.286458 * x * x - 0.469792*x + 1.70216;
		return 0.95*y;
	}
	static DOUBLE GlassAirRefractiveIndex(DOUBLE const waveLength)
	{
		return 1.15 / AirGlassRefractiveIndex(waveLength);
	}

	static DOUBLE AirGovnoRefractiveIndex(DOUBLE const waveLength)
	{
		/// @todo Implement me.
		double static const grid[] = {
			0.380000 * 0 + 1.60000,	0.384040 * 0 + 1.59588, 0.388081 * 0 + 1.59145, 0.392121 * 0 + 1.58665, 0.396162 * 0 + 1.58156, 0.400202 * 0 + 1.57621, 0.404242 * 0 + 1.57060,
			0.408283 * 0 + 1.56468, 0.412323 * 0 + 1.55836, 0.416364 * 0 + 1.55151, 0.420404 * 0 + 1.54400, 0.424444 * 0 + 1.53573, 0.428485 * 0 + 1.52658, 0.432525 * 0 + 1.51646,
			0.436566 * 0 + 1.50530, 0.440606 * 0 + 1.49304, 0.444646 * 0 + 1.47965, 0.448687 * 0 + 1.46511, 0.452727 * 0 + 1.44944, 0.456768 * 0 + 1.43268, 0.460808 * 0 + 1.41492,
			0.464848 * 0 + 1.39627, 0.468889 * 0 + 1.37692, 0.472929 * 0 + 1.35706, 0.476970 * 0 + 1.33696, 0.481010 * 0 + 1.31688, 0.485051 * 0 + 1.29717, 0.489091 * 0 + 1.27815,
			0.493131 * 0 + 1.26019, 0.497172 * 0 + 1.24364, 0.501212 * 0 + 1.22886, 0.505253 * 0 + 1.21618, 0.509293 * 0 + 1.20592, 0.513333 * 0 + 1.19835, 0.517374 * 0 + 1.19372,
			0.521414 * 0 + 1.19225, 0.525455 * 0 + 1.19408, 0.529495 * 0 + 1.19935, 0.533535 * 0 + 1.20814, 0.537576 * 0 + 1.22049, 0.541616 * 0 + 1.23641, 0.545657 * 0 + 1.25589,
			0.549697 * 0 + 1.27887, 0.553737 * 0 + 1.30526, 0.557778 * 0 + 1.33497, 0.561818 * 0 + 1.36783, 0.565859 * 0 + 1.40368, 0.569899 * 0 + 1.44231, 0.573939 * 0 + 1.48346,
			0.577980 * 0 + 1.52684, 0.582020 * 0 + 1.57209, 0.586061 * 0 + 1.61884, 0.590101 * 0 + 1.66663, 0.594141 * 0 + 1.71500, 0.598182 * 0 + 1.76341, 0.602222 * 0 + 1.81131,
			0.606263 * 0 + 1.85815, 0.610303 * 0 + 1.90333, 0.614343 * 0 + 1.94632, 0.618384 * 0 + 1.98657, 0.622424 * 0 + 2.02361, 0.626465 * 0 + 2.05702, 0.630505 * 0 + 2.08646,
			0.634545 * 0 + 2.11169, 0.638586 * 0 + 2.13256, 0.642626 * 0 + 2.14903, 0.646667 * 0 + 2.16118, 0.650707 * 0 + 2.16915, 0.654747 * 0 + 2.17322, 0.658788 * 0 + 2.17371,
			0.662828 * 0 + 2.17101, 0.666869 * 0 + 2.16555, 0.670909 * 0 + 2.15777, 0.674949 * 0 + 2.14813, 0.678990 * 0 + 2.13704, 0.683030 * 0 + 2.12490, 0.687071 * 0 + 2.11206,
			0.691111 * 0 + 2.09880, 0.695152 * 0 + 2.08535, 0.699192 * 0 + 2.07189, 0.703232 * 0 + 2.05856, 0.707273 * 0 + 2.04543, 0.711313 * 0 + 2.03255, 0.715354 * 0 + 2.01997,
			0.719394 * 0 + 2.00769, 0.723434 * 0 + 1.99571, 0.727475 * 0 + 1.98404, 0.731515 * 0 + 1.97269, 0.735556 * 0 + 1.96165, 0.739596 * 0 + 1.95092, 0.743636 * 0 + 1.94052,
			0.747677 * 0 + 1.93043, 0.751717 * 0 + 1.92065, 0.755758 * 0 + 1.91118, 0.759798 * 0 + 1.90202, 0.763838 * 0 + 1.89316, 0.767879 * 0 + 1.88458, 0.771919 * 0 + 1.87625,
			0.775960 * 0 + 1.86809, 0.780000 * 0 + 1.86000,
		};

		auto static const violetWaveLength = 380.0;
		auto static const redWaveLength = 780.0;

		auto const v = (waveLength - violetWaveLength) / (redWaveLength - violetWaveLength);
		auto const i = size_t(v * (dxm::countof(grid) - 1));
		if (i == dxm::countof(grid) - 1)
			return grid[dxm::countof(grid) - 1];

		auto const yi = grid[i], yi1 = grid[i + 1];
		auto const xi = violetWaveLength + i * (redWaveLength - violetWaveLength) / dxm::countof(grid);
		auto const xi1 = violetWaveLength + (i + 1) * (redWaveLength - violetWaveLength) / dxm::countof(grid);
		auto y = yi + (waveLength - xi) / (xi1 - xi) * (yi1 - yi);
		return 0.5 * (y - 1.6) + 1.6;
	}
	static DOUBLE GovnoAirRefractiveIndex(DOUBLE const waveLength)
	{
		return 1.4 / AirGovnoRefractiveIndex(waveLength);
	}

	struct Plane final
	{
		dxm::vec3 PointMin, PointMax;
		dxm::vec3 Normal;
		IndexFunc RefractiveIndex;
		IndexFunc AbsorptionIndex;
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
				return dxm::aabb_check(PointMin, PointMax, intersectionPoint);
			}
			return false;
		}

		// -----------------------
		bool Refract(dxm::vec3& direction, DOUBLE const waveLength) const
		{
			auto const angleBefore = acosf(dxm::dot(direction, Normal) / dxm::length(Normal) / dxm::length(direction));
			auto const sinAngleAfter = 1.0 / RefractiveIndex(waveLength) * sin(angleBefore);
			if (sinAngleAfter >= -1.0 && sinAngleAfter <= 1.0)
			{
				auto const angleAfter = static_cast<float>(asin(sinAngleAfter));
				direction = dxm::rotate(direction, angleBefore - angleAfter, dxm::cross(direction, Normal));
				return true;
			}
			return false;
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
		bool IsEnabled = true;
		PrismType Type = PrismType::Air;
		FLOAT Angle = F_PI / 3.0f, AngleMin = F_PI / 12.0f, AngleMax = dxm::radians(66.0f);
		dxm::vec3 Position, PositionMin, PositionMax;
		FLOAT RotationX = 0.0f, RotationXMin = -F_PI / 2, RotationXMax = F_PI / 2;
		FLOAT RotationZ = 0.0f;

	private:
		TriangleMutableMeshRendererPtr<TRUE, TRUE> m_PrismMesh;
		TriangleMutableMeshRendererPtr<FALSE, TRUE> m_PrismHolderBase, m_PrismHolderLeg, m_PrismHolderGimbal;

	public:
		Prism(IDirect3DDevice9* const device, TriangleMutableMeshPtr const& prismMesh
			, TriangleMutableMeshPtr const& prismHolderBase, TriangleMutableMeshPtr const& prismHolderLeg, TriangleMutableMeshPtr const& prismHolderGimbal)
			// -----------------------
			: m_PrismMesh(std::make_shared<TriangleMutableMeshRenderer<TRUE, TRUE>>(device, prismMesh))
			// -----------------------
			, m_PrismHolderBase(std::make_shared<TriangleMutableMeshRenderer<FALSE, TRUE>>(device, prismHolderBase))
			, m_PrismHolderLeg(std::make_shared<TriangleMutableMeshRenderer<FALSE, TRUE>>(device, prismHolderLeg))
			, m_PrismHolderGimbal(std::make_shared<TriangleMutableMeshRenderer<FALSE, TRUE>>(device, prismHolderGimbal))
		{
		}

		// -----------------------
		void UpdatePlanes(std::vector<Plane>& planes) const
		{
			if (!IsEnabled)
				return;

			struct { IndexFunc In, Out; } static const refractiveIndexFuncsTable[] = {
				{ &AirGlassRefractiveIndex, &GlassAirRefractiveIndex },
				{ &AirGovnoRefractiveIndex, &GovnoAirRefractiveIndex },
			};

			IndexFunc static const absorptionIndexFuncTable[]{
				&DummyIndex,
				&GovnoAbsorptionIndex
			};

			auto const& refractiveIndexFuncs = refractiveIndexFuncsTable[static_cast<size_t>(Type)];
			auto const absorptionIndexFunc = absorptionIndexFuncTable[static_cast<size_t>(Type)];

			auto const transformationMatrix = dxm::translate(Position)
				* dxm::toMat4(dxm::quat(dxm::vec3(RotationX, 0.0f, RotationZ)))
				* dxm::translate(glm::vec3{ 0, -0.1f + 0.2f / 3, 0 })
				* dxm::scale(glm::vec3(1.0f, 1.0f, tanf(Angle / 2.0f)));
			{
				auto const p1 = transformationMatrix * dxm::vec4(-0.1f, -0.1f, +0.0f, 1.0);
				auto const p2 = transformationMatrix * dxm::vec4(+0.1f, -0.1f, +0.0f, 1.0);
				auto const p3 = transformationMatrix * dxm::vec4(+0.1f, +0.1f, -0.2f, 1.0);
				auto const normal = dxm::cross(dxm::vec3(p2 - p1), dxm::vec3(p3 - p1));
				planes.push_back({ p1, p3, normal, refractiveIndexFuncs.In, absorptionIndexFunc });
			}

			{
				auto const p1 = transformationMatrix * dxm::vec4(-0.1f, -0.1f, +0.0f, 1.0);
				auto const p2 = transformationMatrix * dxm::vec4(+0.1f, -0.1f, +0.0f, 1.0);
				auto const p3 = transformationMatrix * dxm::vec4(-0.1f, +0.1f, +0.2f, 1.0);
				auto const normal = dxm::cross(dxm::vec3(p2 - p1), dxm::vec3(p3 - p1));
				planes.push_back({ p2, p3, normal, refractiveIndexFuncs.Out, &DummyIndex });
			}
		}

		// -----------------------
		void Update()
		{
			if (!IsEnabled)
				return;

			auto static const LegHeight = 1.5f;
			auto static const GimbalHeight = 0.2f;

			m_PrismHolderBase->Position = { Position.x, 0.0f, Position.z };
			m_PrismHolderBase->Render();

			m_PrismHolderLeg->Position = { Position.x, Position.y - LegHeight - GimbalHeight, Position.z };
			m_PrismHolderLeg->Render();

			m_PrismHolderGimbal->Position = { Position.x, Position.y, Position.z };
			m_PrismHolderGimbal->Rotation.z = RotationZ;
			m_PrismHolderGimbal->Render();

			m_PrismMesh->PositionOffset = {0, -0.1f+0.2f/3, 0};
			m_PrismMesh->Position = Position;
			m_PrismMesh->Scale = glm::vec3(1.0f, 1.0f, tanf(Angle / 2.0f));
			m_PrismMesh->Rotation.x = RotationX;
			m_PrismMesh->Rotation.z = RotationZ;
			m_PrismMesh->Render();
		}
	};	// struct Prism

	typedef std::shared_ptr<class PresentationWidget> PresentationWidgetPtr;
	class PresentationWidget final : public GraphicsWidget
	{
	public:
		OrbitalCamera m_Camera;

		TriangleMutableMeshPtr m_RoomMesh;
		TriangleMutableMeshRendererPtr<> m_RoomRenderer;

		TriangleMutableMeshPtr m_ScreenMesh;
		TriangleMutableMeshRendererPtr<> m_ScreenRenderer;

		bool mutable m_AreRaysSynced = false;
#if USE_RAYS_RENDERING
		LineMutableMeshPtr m_RaysMesh;
		LineMutableMeshRendererPtr<TRUE> m_RaysRenderer;
#else
		TriangleMutableMeshPtr m_RaysMesh;
		TriangleMutableMeshRendererPtr<TRUE> m_RaysRenderer;
#endif
		TriangleMutableMeshPtr m_RaysProjectionMesh;
		TriangleMutableMeshRendererPtr<TRUE> m_RaysProjectionRenderer;

		TriangleMutableMeshPtr m_PrismMesh;
		TriangleMutableMeshPtr m_PrismHolderBase, m_PrismHolderLeg, m_PrismHolderGimbal;
		Prism mutable m_PrismRenderers[2];
		std::vector<Plane> mutable m_PrismPlanes;

	public:
		explicit PresentationWidget(HWND const hwnd, IDirect3DDevice9* const device, ...)
			: GraphicsWidget(hwnd, device)
			// -----------------------
			, m_Camera(device)
			// -----------------------
			, m_RoomMesh(std::make_shared<TriangleMutableMesh>(device)), m_RoomRenderer(std::make_shared<TriangleMutableMeshRenderer<>>(device, m_RoomMesh, L"../gfx/roomLightMap.png"))
			// -----------------------
			, m_ScreenMesh(std::make_shared<TriangleMutableMesh>(device)), m_ScreenRenderer(std::make_shared<TriangleMutableMeshRenderer<>>(device, m_ScreenMesh))
			// -----------------------
			, m_RaysMesh(std::make_shared<LineMutableMesh>(device)), m_RaysRenderer(std::make_shared<LineMutableMeshRenderer<TRUE>>(device, m_RaysMesh))
			, m_RaysProjectionMesh(std::make_shared<TriangleMutableMesh>(device)), m_RaysProjectionRenderer(std::make_shared<TriangleMutableMeshRenderer<TRUE>>(device, m_RaysProjectionMesh))
			// -----------------------
			, m_PrismMesh(std::make_shared<TriangleMutableMesh>(device))
			, m_PrismHolderBase(std::make_shared<TriangleMutableMesh>(device)), m_PrismHolderLeg(std::make_shared<TriangleMutableMesh>(device)), m_PrismHolderGimbal(std::make_shared<TriangleMutableMesh>(device))
			, m_PrismRenderers{ { m_Device, m_PrismMesh, m_PrismHolderBase, m_PrismHolderLeg, m_PrismHolderGimbal }
			,{ m_Device, m_PrismMesh, m_PrismHolderBase, m_PrismHolderLeg, m_PrismHolderGimbal } }
		{
			/* Setting up default alpha-blending. */
			Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD));
			Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
			Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE));
		//	Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME));

			/* Setting up default lights. */
			D3DLIGHT9 light = {};
			light.Type = D3DLIGHT_DIRECTIONAL;
			light.Diffuse = { 0.7f, 0.7f, 0.7f, 1.0f };
			light.Direction = { 0.0f, -0.3f, 1.0f };
			light.Attenuation1 = 0.1f;
			Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_LIGHTING, TRUE));
			Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(0x50, 0x50, 0x50)));
			Utils::RuntimeCheckH(m_Device->SetLight(0, &light));

			/* Setting up default texture parameters. */
			Utils::RuntimeCheckH(m_Device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE));
			Utils::RuntimeCheckH(m_Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE));
			Utils::RuntimeCheckH(m_Device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE));

			/* Setting up static scene parameters. */
		//	LoadOBJ(L"../gfx/room.obj", m_RoomMesh);
		//	LoadTexture(m_Device, L"../gfx/roomLightMap.png", &m_RoomRenderer->Texture);
			m_RoomRenderer->Position.z = 2.0f;
			m_RoomRenderer->Rotation.y = F_PI;
		//	LoadOBJ(L"../gfx/screen.obj", m_ScreenMesh);
		//	LoadTexture(m_Device, L"../gfx/screenLightMap.png", &m_ScreenRenderer->Texture);
			m_ScreenRenderer->Position.z = 2.0f;
			m_ScreenRenderer->Rotation.y = F_PI;

			/* Setting up dynamic scene parameters. */
		//	LoadOBJ(L"../gfx/prism.obj", m_PrismMesh, D3DCOLOR_ARGB(0xFF / 3, 0xFF / 3, 0xFF, 0xFF));
		//	LoadOBJ(L"../gfx/holder_base.obj", m_PrismHolderBase);
		//	LoadOBJ(L"../gfx/holder_leg.obj", m_PrismHolderLeg);
		//	LoadOBJ(L"../gfx/holder_gimbal.obj", m_PrismHolderGimbal);

			SetDualPrismsLayout();
			SetSinglePrismLayout();

			/* Setting up some other shit. */
		//	LoadTexture(m_Device, L"../gfx/color_mask.png", &m_RaysProjectionRenderer->Texture);
		//	LoadPixelShader(m_Device, L"../gfx/ColoredTextureShader.hlsl", &m_RaysProjectionRenderer->PixelShader);
		}

		// -----------------------
		void SetSinglePrismLayout()
		{
			m_PrismRenderers[0].Type = PrismType::Air;
			m_PrismRenderers[0].IsEnabled = true;
			m_PrismRenderers[0].Angle = F_PI / 3.0f;
			m_PrismRenderers[0].AngleMin = dxm::radians(15.0f);
			m_PrismRenderers[0].Position = { 0.0f, 0.75f, 1.3f };
			m_PrismRenderers[0].PositionMin = { -1.05f, 0.5f, 2.0f };
			m_PrismRenderers[0].PositionMax = { +1.05f, 1.0f, 2.7f };
			m_PrismRenderers[0].RotationX = 0;
			m_PrismRenderers[0].RotationXMin = dxm::radians(-5.0f);
			m_PrismRenderers[0].RotationXMax = dxm::radians(13.0f);
			m_PrismRenderers[1].IsEnabled = false;

			m_AreRaysSynced = false;
		}

		void SetDualPrismsLayout()
		{
			m_PrismRenderers[0].Type = PrismType::Air;
			m_PrismRenderers[0].IsEnabled = true;
			m_PrismRenderers[0].Angle = F_PI / 3.0f;
			m_PrismRenderers[0].AngleMin = dxm::radians(55.0f);
			m_PrismRenderers[0].Position = { 0.0f, 0.75f, 1.4f };
			m_PrismRenderers[0].PositionMin = { -1.05f, 0.5f, 0.4f };
			m_PrismRenderers[0].PositionMax = { +1.05f, 1.0f, 1.6f };
			m_PrismRenderers[0].RotationXMin = dxm::radians(-5.0f);
			m_PrismRenderers[0].RotationXMax = dxm::radians(13.0f);
			m_PrismRenderers[0].RotationX = 0;

			m_PrismRenderers[1].Type = PrismType::Air;
			m_PrismRenderers[1].IsEnabled = true;
			m_PrismRenderers[1].Angle = F_PI / 3.0f;
			m_PrismRenderers[1].AngleMin = dxm::radians(55.0f);
			m_PrismRenderers[1].Position = { 0.0f, 1.0f, 2.0f };
			m_PrismRenderers[1].PositionMin = { -1.05f, 0.5f, 2.0f };
			m_PrismRenderers[1].PositionMax = { +1.05f, 1.0f, 2.7f };
			m_PrismRenderers[1].RotationZ = F_PI / 2;
			m_PrismRenderers[1].RotationX = 0;
			m_PrismRenderers[1].RotationXMin = dxm::radians(-5.0f);
			m_PrismRenderers[1].RotationXMax = dxm::radians(13.0f);

			m_AreRaysSynced = false;
		}

		// -----------------------
		void Render() const override
		{
			m_Device->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
			m_Device->Clear(0, nullptr, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
			m_Device->BeginScene();
			{
				/* Rendering scene. */
				m_RoomRenderer->Render();
				m_ScreenRenderer->Render();

				/* Rendering rays. */
				m_RaysProjectionRenderer->Render();
				m_RaysRenderer->Render();

				/* Updating and rendering prisms and holders. */
		//		Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
		//		Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
				for (auto& prism : m_PrismRenderers)
				{
					prism.Update();
				}
		//		Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
		//		Utils::RuntimeCheckH(m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE));
			}
			m_Device->EndScene();
			m_Device->Present(nullptr, nullptr, nullptr, nullptr);
		}
		// -----------------------
		void Update() override
		{
			/* Updating camera first. */
			((OrbitalCamera&)m_Camera).Render();

			/* Updating rays. */
			if (!m_AreRaysSynced)
			{
				m_PrismPlanes.clear();
				for (auto& prism : m_PrismRenderers)
				{
					prism.UpdatePlanes(m_PrismPlanes);
				}
				m_PrismPlanes.push_back({ { -1.77f, 0.485f, 3.49f },{ 1.77f, 2.07f, 3.49f },{ 0.0f, 0.0f, 1.0f }, &DummyIndex, &DummyIndex });
				((PresentationWidget*)this)->GenerateRaysMesh(USE_RAYS_RENDERING * 1000 + !USE_RAYS_RENDERING * 100);
				m_AreRaysSynced = true;
			}
		}

		// -----------------------
		void GenerateRaysMesh(UINT const partitioning)
		{
			m_RaysMesh->BeginUpdateVertices();
			m_RaysProjectionMesh->BeginUpdateVertices();

#if USE_RAYS_RENDERING
			for (auto i = 0u; i < partitioning; ++i)
			{
				auto coord = dxm::vec3(0.0f, 0.72f, -2.0f);
				auto direction = dxm::vec3(0.0f, 0.0f, 1.0f);

				auto static const violetWaveLength = 380.0;
				auto static const redWaveLength = 780.0;
				auto const waveLength = violetWaveLength + i * (redWaveLength - violetWaveLength) / partitioning;
				auto color = ConvertWaveLengthToRGB(waveLength);

				for (auto j = 0u; j < m_PrismPlanes.size(); ++j)
				{
					auto& plane = m_PrismPlanes[j];

					auto const prevCoord = coord;
					if (!plane.Intersect(coord, coord, direction))
					{
						coord = prevCoord;
						continue;
					}
					m_RaysMesh->AddVertex({ prevCoord, color });
					m_RaysMesh->AddVertex({ coord, j == m_PrismPlanes.size() - 1 ? color & 0x00FFFFFF : color });
					if (!plane.Refract(direction, waveLength))
						break;

					AbsorbAlpha(color, plane.AbsorptionIndex(waveLength));

					if (j == m_PrismPlanes.size() - 1)
					{
						AbsorbAlpha(color, m_PrismRenderers[1].Type == PrismType::Air ? 0.42 : 0.8f);

						auto const scale = 0.03f;
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
							m_RaysProjectionMesh->AddVertex({ triangleVert[k] * scale + coord,{ 0.0, 0.0, -1.0f }, color, triangleVert[k] + uvOffset });
						}
					}
				}
			}
#else
			struct SavedRay
			{
				dxm::vec3 Coord;
				dxm::vec3 Direction;
				dxm::argb Color;
				dxm::vec3 PrevCoord;
				dxm::argb PrevColor;
			};	// struct SavedRay

			auto static const rayUV = dxm::vec2(0.0f, 0.0f);
			auto static const rayNormal = dxm::vec3(1.0, 0.0, 0.0f);
			auto static const projectionNormal = dxm::vec3(0.0, 0.0, -1.0f);

			auto const startCoord = dxm::vec3(0.0f, 0.75f, -2.0f);
			auto const startDirection = dxm::vec3(0, 0, 1);
			std::vector<SavedRay> savedRays(partitioning, { startCoord, startDirection });
			for (auto i = 0u; i < partitioning; ++i)
			{
				auto static const violetWaveLength = 380.0;
				auto static const redWaveLength = 740.0;
				auto const waveLength = violetWaveLength + i * (redWaveLength - violetWaveLength) / partitioning;
				auto color = ConvertWaveLengthToRGB(waveLength);
				savedRays[i].Color = color;
			}

			for (auto j = 0u; j < m_PrismPlanes.size(); ++j)
			{
				auto& plane = m_PrismPlanes[j];

				for (auto i = 0u; i < partitioning; ++i)
				{
					auto static const violetWaveLength = 380.0;
					auto static const redWaveLength = 740.0;
					auto const waveLength = violetWaveLength + i * (redWaveLength - violetWaveLength) / partitioning;

					auto& coord = savedRays[i].Coord;
					auto& direction = savedRays[i].Direction;
					auto& color = savedRays[i].Color;
					auto& prevCoord = savedRays[i].PrevCoord;
					auto& prevColor = savedRays[i].PrevColor;

					prevCoord = coord;
					prevColor = color;
					if (!plane.Intersect(coord, coord, direction) && m_PrismPlanes.size() - 1 != j)
					{
						coord = prevCoord;
						break;
					}

					auto const finalColor = j == m_PrismPlanes.size() - 1 ? color & 0x00FFFFFF : color;
					if (i != 0)
					{
						m_RaysMesh.AddVertex({ coord, rayNormal, finalColor, rayUV });
						if (j != 0)
						{
							m_RaysMesh.AddVertex({ savedRays[i - 1].PrevCoord, rayNormal, savedRays[i - 1].PrevColor, rayUV });
							m_RaysMesh.AddVertex({ prevCoord, rayNormal, color, rayUV });
							m_RaysMesh.AddVertex({ coord, rayNormal, color, rayUV });
						}
					}
					if (i != partitioning - 1)
					{
						m_RaysMesh.AddVertex({ prevCoord, rayNormal, color, rayUV });
						m_RaysMesh.AddVertex({ j != 0 ? coord : coord + dxm::vec3(0.002f), rayNormal, finalColor, rayUV });
					}

					if (!plane.Refract(direction, waveLength))
						continue;
					AbsorbAlpha(color, plane.AbsorptionIndex(waveLength));

					if (j == m_PrismPlanes.size() - 1)
					{
						AbsorbAlpha(color, 0.3);

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
							m_RaysProjectionMesh.AddVertex({ triangleVert[k] * scale + coord, projectionNormal, color, triangleVert[k] + uvOffset });
						}
					}
				}
			}
#endif

			m_RaysMesh->EndUpdateVertices();
			m_RaysProjectionMesh->EndUpdateVertices();
			m_AreRaysSynced = true;
		}

		// -----------------------
		static void AbsorbAlpha(dxm::argb& color, DOUBLE const index)
		{
			// Modifying alpha-based absorbtion.
			auto const alphaPrev = (color >> 24 & 0xFF) / 255.0;
			auto const alpha = alphaPrev * index;
			color &= 0x00FFFFFF;
			color |= DWORD(alpha * 255.0) << 24;
		}

		// -----------------------
		static dxm::argb ConvertWaveLengthToRGB(double waveLength)
		{
			/*auto static const violetWaveLength = 380.0;
			auto static const redWaveLength = 740.0;
			waveLength = (waveLength - violetWaveLength) / (redWaveLength - violetWaveLength);
			auto static const funcVioletWaveLength = 380.0;
			auto static const funcRedWaveLength = 781.0;
			waveLength = funcVioletWaveLength + waveLength * (funcRedWaveLength - funcVioletWaveLength);*/

			auto static const gamma = 0.8;
			auto static const intensityMax = 255.0;

			double red, green, blue;
			if (waveLength >= 380.0 && waveLength < 440.0)
			{
				red = -(waveLength - 440.0) / (440.0 - 380.0);
				green = 0.0;
				blue = 1.0;
			}
			else if (waveLength >= 440.0 && waveLength < 490.0)
			{
				red = 0.0;
				green = (waveLength - 440.0) / (490.0 - 440.0);
				blue = 1.0;
			}
			else if (waveLength >= 490.0 && waveLength < 510.0)
			{
				red = 0.0;
				green = 1.0;
				blue = -(waveLength - 510.0) / (510.0 - 490.0);
			}
			else if (waveLength >= 510.0 && waveLength < 580.0)
			{
				red = (waveLength - 510.0) / (580.0 - 510.0);
				green = 1.0;
				blue = 0.0;
			}
			else if (waveLength >= 580.0 && waveLength < 645.0)
			{
				red = 1.0;
				green = -(waveLength - 645.0) / (645.0 - 580.0);
				blue = 0.0;
			}
			else if (waveLength >= 645.0 && waveLength < 781.0)
			{
				red = 1.0;
				green = 0.0;
				blue = 0.0;
			}
			else
			{
				red = 0.0;
				green = 0.0;
				blue = 0.0;
			}

			// Let the intensity fall off near the vision limits.
			double factor;
			if (waveLength >= 380.0 && waveLength < 395.00)
			{
				factor = 0.3 + 0.7 * (waveLength - 380.0) / (395.0 - 380.0);
			}
			else if (waveLength >= 395.0 && waveLength < 745.52)
			{
				factor = 1.0;
			}
			else if (waveLength >= 745.52 && waveLength < 781.0)
			{
				factor = 0.3 + 0.7 * (780.0 - waveLength) / (780.0 - 745.52);
			}
			else
			{
				factor = 0.0;
			}

			//factor = 1.0;

			unsigned rgb[3];
			rgb[0] = red == 0.0 ? 0 : static_cast<int>(round(intensityMax * pow(red * factor, gamma)));
			rgb[1] = green == 0.0 ? 0 : static_cast<int>(round(intensityMax * pow(green * factor, gamma)));
			rgb[2] = blue == 0.0 ? 0 : static_cast<int>(round(intensityMax * pow(blue * factor, gamma)));
			return D3DCOLOR_RGBA(rgb[0], rgb[1], rgb[2], USE_RAYS_RENDERING * 0xFF / 8 + !USE_RAYS_RENDERING * 2 * 0xFF / 3);
		}

	}; // class PresentationWidget

} // namespace Presentation