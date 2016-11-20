// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// Copyright (C) 2016 Butakov Oleg, Plaxin Gleb.
// Computational Methods @ Computational Mathematics & Cybernetics, MSU.
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

#pragma once
#include "PresentationEngine.hpp"
#include "PresentationPhysics.hpp"

#include <array>
#define STANDART_DESKTOP_WIDTH 1920
#define STANDART_DESKTOP_HEIGHT 1080

namespace Presentation2
{
	enum class PrismType
	{
		Air,
		Cyanine,
		COUNT,
	};	// enum class PrismType

	template<typename T>
	struct BoundedValue
	{
		T Value, Min, Max;

		ADINL operator T const& () const
		{
			return Value;
		}
		ADINL operator T& () 
		{
			return Value;
		}
	};	// struct BoundedValue

	class PrismController final : public IEngineUpdatable
	{
	public:
		bool IsEnabled;
		PrismType Type = PrismType::Air;
		BoundedValue<FLOAT> Angle = { dxm::degrees(60.0f) };
		BoundedValue<dxm::vec3> Position;
		BoundedValue<FLOAT> RotationX = {};
		FLOAT RotationZ = 0.0f;

		TriangleMeshRendererPtr<FALSE, TRUE> PrismHolderBase;
		TriangleMeshRendererPtr<FALSE, TRUE> PrismHolderLeg;
		TriangleMeshRendererPtr<FALSE, TRUE> PrismHolderGimbal;
		TriangleMeshRendererPtr<TRUE, TRUE> Prism;

	public:
		ADINT void OnUpdate() override final
		{
			IEngineUpdatable::OnUpdate();

			/* Setting up the prism holder transformations. */
			auto static const LegHeight = 1.5f;
			auto static const GimbalHeight = 0.2f;

			PrismHolderBase->IsEnabled = IsEnabled;
			PrismHolderBase->Position = Position;
			PrismHolderBase->Position.y = 0.0f;
			PrismHolderBase->Rotation = {};
			PrismHolderBase->Scale = { 1.0f, 1.0f, 1.0f };

			PrismHolderLeg->IsEnabled = IsEnabled;
			PrismHolderLeg->Position = Position;
			PrismHolderLeg->Position.y -= LegHeight + GimbalHeight;
			PrismHolderLeg->Rotation = {};
			PrismHolderLeg->Scale = { 1.0f, 1.0f, 1.0f };

			PrismHolderGimbal->IsEnabled = IsEnabled;
			PrismHolderGimbal->Position = Position;
			PrismHolderGimbal->Rotation = {};
			PrismHolderGimbal->Rotation.z = RotationZ;
			PrismHolderGimbal->Scale = { 1.0f, 1.0f, 1.0f };

			/* Setting up the prism transformations. */
			Prism->IsEnabled = IsEnabled;
			Prism->PositionOffset = { 0.0f, 0.2f / 3 - 0.1f, 0.0f };
			Prism->Position = Position;
			Prism->Rotation.x = RotationX;
			Prism->Rotation.z = RotationZ;
			Prism->Scale = { 1.0f, 1.0f, tanf(Angle / 2.0f) };
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

			auto const transformationMatrix = dxm::translate(Position.Value)
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
	};	// class PrismController
	using PrismControllerPtr = std::shared_ptr<PrismController>;
	using PrismControllerPtrs = std::array<PrismControllerPtr, 2>;

	class RaysController final : public IEngineUpdatable
	{
	public:
		bool& IsSceneSynced;
		PrismControllerPtrs PrismControllers;
		LineMutableMeshPtr RaysMesh;
		TriangleMutableMeshPtr RaysProjectionMesh;
		CameraPtr RaysProjectionCamera;

	public:
		ADINL explicit RaysController(bool& isSceneSynced)
			: IsSceneSynced(isSceneSynced)
		{
		}

		ADINT void OnUpdate() override final
		{
			IEngineUpdatable::OnUpdate();

			if (!IsSceneSynced)
			{
				std::vector<Plane> prismPlanes;
				for (auto const& prismController : PrismControllers)
				{
					prismController->UpdatePlanes(prismPlanes);
				}
				prismPlanes.push_back({ { -1.77f, 0.485f, 3.49f },{ 1.77f, 2.07f, 3.49f },{ 0.0f, 0.0f, 1.0f }, &DummyIndex, &DummyIndex });

				RaysMesh->BeginUpdateVertices();
				RaysProjectionMesh->BeginUpdateVertices();

				dxm::vec3 minBound(FLT_MAX);
				dxm::vec3 maxBound(-FLT_MAX);

				auto static const raysCount = 2000u;
				auto static const skipped = 0u;
				for (auto i = skipped; i < raysCount - skipped; ++i)
				{
					auto const waveLength = g_VioletWaveLength + i * (g_RedWaveLength - g_VioletWaveLength) / raysCount;
					auto waveColor = ConvertWaveLengthToRGB(waveLength);

					auto coord = dxm::vec3(0.0f, 0.72f, -2.0f);
					auto direction = dxm::vec3(0.0f, 0.0f, 1.0f);
					for (auto j = 0u; j < prismPlanes.size(); ++j)
					{
						auto& plane = prismPlanes[j];

						auto const prevCoord = coord;
						if (!plane.Intersect(coord, coord, direction))
						{
							coord = prevCoord;
							continue;
						}
						RaysMesh->AddVertex({ prevCoord, waveColor });
						RaysMesh->AddVertex({ coord, j == prismPlanes.size() - 1 ? waveColor & 0x01FFFFFF : waveColor });
						if (!plane.Refract(direction, waveLength))
							break;

						AbsorbAlpha(waveColor, plane.AbsorptionIndex(waveLength));

						if (j == prismPlanes.size() - 1)
						{
							AbsorbAlpha(waveColor, 0.7);
							minBound.x = std::min(minBound.x, coord.x);
							minBound.y = std::min(minBound.y, coord.y);
							minBound.z = std::min(minBound.z, coord.z);

							maxBound.x = std::max(maxBound.x, coord.x);
							maxBound.y = std::max(maxBound.y, coord.y);
							maxBound.z = std::max(maxBound.z, coord.z);

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
								RaysProjectionMesh->AddVertex({ triangleVert[k] * scale + coord,{ 0.0, 0.0, -1.0f }, waveColor, triangleVert[k] + uvOffset });
							}
							/*auto const scale = 0.03f;
							dxm::vec3 static prevHitCoord;
							dxm::argb static prevHitWaveColor;

							if (i == 0 || i == raysCount - 1)
							{
								RaysProjectionMesh->AddVertex(TriangleVertex(coord - dxm::vec3(scale, 0.0f, 0.0f), { 0.0, 0.0, -1.0f }, waveColor, { 0.5f, 0.5f }));
								RaysProjectionMesh->AddVertex(TriangleVertex(coord + dxm::vec3(scale, 0.0f, 0.0f), { 0.0, 0.0, -1.0f }, waveColor, { 0.5f, 0.5f }));
								prevHitCoord = coord;
								prevHitWaveColor = waveColor;
							}
							else
							{
								RaysProjectionMesh->AddVertex(TriangleVertex(coord + dxm::vec3(scale, 0.0f, 0.0f), { 0.0, 0.0, -1.0f }, waveColor, {0.5f, 0.5f}));

								RaysProjectionMesh->AddVertex(TriangleVertex(coord - dxm::vec3(scale, 0.0f, 0.0f), { 0.0, 0.0, -1.0f }, waveColor, { 0.5f, 0.5f }));
								RaysProjectionMesh->AddVertex(TriangleVertex(coord + dxm::vec3(scale, 0.0f, 0.0f), { 0.0, 0.0, -1.0f }, waveColor, { 0.5f, 0.5f }));
								RaysProjectionMesh->AddVertex(TriangleVertex(prevHitCoord - dxm::vec3(scale, 0.0f, 0.0f), { 0.0, 0.0, -1.0f }, prevHitWaveColor, { 0.5f, 0.5f }));

								RaysProjectionMesh->AddVertex(TriangleVertex(coord - dxm::vec3(scale, 0.0f, 0.0f), { 0.0, 0.0, -1.0f }, waveColor, { 0.5f, 0.5f }));
								RaysProjectionMesh->AddVertex(TriangleVertex(coord + dxm::vec3(scale, 0.0f, 0.0f), { 0.0, 0.0, -1.0f }, waveColor, { 0.5f, 0.5f }));
								prevHitCoord = coord;
								prevHitWaveColor = waveColor;
							}*/
						}
					}
				}
				RaysProjectionCamera->Position = (minBound + maxBound) / 2.0f;
				RaysProjectionCamera->Position.z = 0.0f;
				RaysProjectionCamera->Size = std::max(maxBound.x - minBound.x, maxBound.y - minBound.y);

				RaysMesh->EndUpdateVertices();
				RaysProjectionMesh->EndUpdateVertices();

				IsSceneSynced = true;
			}
		}
	};	// class RaysController
	using RaysControllerPtr = std::shared_ptr<RaysController>;

	class PresentationScene final : public Scene
	{
	public:
		bool m_IsSceneSynced = false;
		PrismControllerPtrs m_PrismContollers;
		RaysControllerPtr m_RaysController;

	public:
		// -----------------------
		ADINT explicit PresentationScene(IDirect3DDevice9* const device)
			: Scene(device)
		{
			{	/* Setting up the room. */
				auto const roomMesh = TriangleMesh(L"../gfx/room.obj");
				auto const screenMesh = TriangleMesh(L"../gfx/screen.obj");

				auto const screen = TriangleMeshRenderer(screenMesh, L"../gfx/screenLightMap.png", L"../gfx/Shaders/ColoredTextureShader.hlsl");
				auto const room = TriangleMeshRenderer(roomMesh, L"../gfx/roomLightMap.png", L"../gfx/Shaders/ColoredTextureShader.hlsl");

				room->Position.z = screen->Position.z = 2.0f;
				room->Rotation.y = screen->Rotation.y = dxm::radians(180.0f);
			}

			{	/* Setting up the rays. */
				m_RaysController = CustomUpdatable<RaysController>(m_IsSceneSynced);

				m_RaysController->RaysProjectionMesh = TriangleMutableMesh();
				auto const raysProjection = TriangleMutableMeshRenderer<TRUE>(m_RaysController->RaysProjectionMesh, L"../gfx/color_mask.png", L"../gfx/Shaders/ColoredTextureShader.hlsl");
				raysProjection->Layers |= Layer::Custom0;

				m_RaysController->RaysMesh = LineMutableMesh();
				auto const rays = LineMutableMeshRenderer<TRUE>(m_RaysController->RaysMesh);
			}

			{	/* Setting up the camera. */
				auto const camera = OrbitalCamera();
				camera->Rotation = { 0.0, -dxm::radians(45.0f) };
				camera->RotationCenter = { 0.0f, 1.2f, 2.0f };
				camera->CenterOffset = { 0.0f, 0.0f, -1.8f };
				camera->Layer = Layer::Default | Layer::Custom0 | Layer::Transparent;

				m_RaysController->RaysProjectionCamera = Camera();
				m_RaysController->RaysProjectionCamera->Projection = BaseCameraProjection::Orthographic;
				m_RaysController->RaysProjectionCamera->Rotation = dxm::vec3(0, dxm::radians(0.0f), 0);
				m_RaysController->RaysProjectionCamera->Layer = Layer::Custom0 | Layer::Transparent;
				m_RaysController->RaysProjectionCamera->Viewport = Rect(UpperLeftPivot, 1440 - 500 - 50, 50, 500, 500);
			}

			{	/* Setting up the prisms. */
				auto const prismHolderBaseMesh = TriangleMesh(L"../gfx/holder_base.obj");
				auto const prismHolderLegMesh = TriangleMesh(L"../gfx/holder_leg.obj");
				auto const prismHolderGimbalMesh = TriangleMesh(L"../gfx/holder_gimbal.obj");
				auto const prismMesh = TriangleMesh(L"../gfx/prism.obj", D3DCOLOR_ARGB(0xEE, 0xFF / 3, 0xFF, 0xFF));

				for (auto& prismController : m_PrismContollers)
				{
					prismController = CustomUpdatable<PrismController>();
					prismController->PrismHolderBase = TriangleMeshRenderer<FALSE, TRUE>(prismHolderBaseMesh);
					prismController->PrismHolderLeg = TriangleMeshRenderer<FALSE, TRUE>(prismHolderLegMesh);
					prismController->PrismHolderGimbal = TriangleMeshRenderer<FALSE, TRUE>(prismHolderGimbalMesh);
					prismController->Prism = TriangleMeshRenderer<TRUE, TRUE>(prismMesh);
					prismController->Prism->DestBlend = D3DBLEND_INVSRCALPHA;
				}
				m_RaysController->PrismControllers = m_PrismContollers;

				ResetTwoPrismsLayout();
			}
		}

		// -----------------------
		ADINT void ResetOnePrismLayout()
		{
			{
				auto& firstPrismController = m_PrismContollers[0];
				firstPrismController->IsEnabled = true;
				firstPrismController->Type = PrismType::Air;
				firstPrismController->Angle = { dxm::radians(60.0f), dxm::radians(15.0f), dxm::radians(66.0f) };
				firstPrismController->Position = { { 0.0f, 0.75f, 1.3f }, { -1.05f, 0.5f, 0.4f }, { +1.05f, 1.0f, 1.6f } };
				firstPrismController->RotationX = { 0.0f, dxm::radians(0.0f), dxm::radians(10.0f) };
				firstPrismController->RotationZ = 0.0f;
			}
			{
				auto& secondPrismController = m_PrismContollers[1];
				secondPrismController->IsEnabled = false;
			}
			m_IsSceneSynced = false;
		}

		// -----------------------
		ADINT void ResetTwoPrismsLayout()
		{
			{
				auto& firstPrismController = m_PrismContollers[0];
				firstPrismController->IsEnabled = true;
				firstPrismController->Type = PrismType::Air;
				firstPrismController->Angle = { dxm::radians(60.0f), dxm::radians(55.0f), dxm::radians(66.0f) };
				firstPrismController->Position = { { 0.0f, 0.75f, 1.4f }, { -1.05f, 0.5f, 0.4f }, { +1.05f, 1.0f, 1.6f } };
				firstPrismController->RotationX = { 0.0f, dxm::radians(0.0f), dxm::radians(10.0f) };
				firstPrismController->RotationZ = 0.0f;
			}
			{
				auto& secondPrismController = m_PrismContollers[1];
				secondPrismController->IsEnabled = true;
				secondPrismController->Type = PrismType::Air;
				secondPrismController->Angle = { dxm::radians(55.0f), dxm::radians(55.0f), dxm::radians(66.0f) };
				secondPrismController->Position = { { 0.0f, 1.0f, 2.0f }, { -1.05f, 0.5f, 2.0f }, { +1.05f, 1.0f, 2.7f } };
				secondPrismController->RotationX = { 0.0f, 0.0f, dxm::radians(13.0f) };
				secondPrismController->RotationZ = dxm::radians(90.0f);
			}
			m_IsSceneSynced = false;
		}

	};	// class PresentationScene

	using PresentationScenePtr = std::shared_ptr<PresentationScene>;

}	// namespace Presentation2
