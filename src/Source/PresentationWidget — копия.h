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

	class PresentationWidget final : public yx2::engine::Runtime
	{
		using Plane = Presentation::Plane;
		using Prism = Presentation::Prism;
		std::vector<Plane> planes;
		std::vector<Prism> m_Prisms;

	private:
		OrbitalCamera m_Camera;

		TriangleMutableMesh m_RoomMesh;
		TriangleMutableMeshRenderer<> m_RoomMeshRenderer;

		bool m_AreRaysSynced = false;
		LineMutableMesh m_RaysMesh;
		LineMutableMeshRenderer<TRUE> m_RaysMeshRenderer;
		TriangleMutableMesh m_RaysProjectionMesh;
		TriangleMutableMeshRenderer<TRUE> m_RaysProjectionMeshRenderer;

	public:
		explicit PresentationWidget(HWND const hwnd, IDirect3DDevice9* const device)
			: Runtime(hwnd, device)
			, m_Camera(device)
			, m_RoomMesh(device), m_RoomMeshRenderer(device, m_RoomMesh)
			, m_RaysMesh(device), m_RaysMeshRenderer(device, m_RaysMesh)
			, m_RaysProjectionMesh(device), m_RaysProjectionMeshRenderer(device, m_RaysProjectionMesh)
		{
			m_Device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

			m_Device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			m_Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

			LoadOBJ("../room.obj", m_RoomMesh);
			LoadTexture(m_Device, L"../baked.png", &m_RoomMeshRenderer.Texture);
			m_RoomMeshRenderer.Position = {0.0f, 0.0f, 2.0f};
			m_RoomMeshRenderer.Rotation.x = DXM_PI / 2.0f;
		
			LoadTexture(m_Device, L"../color_mask.png", &m_RaysProjectionMeshRenderer.Texture);
			LoadShader(m_Device, L"../ColoredTextureShader.hlsl", &m_RaysProjectionMeshRenderer.PixelShader);

			m_Prisms = {
				{ { 0.0f, 1.2f, 0.75f }, DXM_PI / 3, },
				{ { 0.0f, 0.9f, 2.0f }, DXM_PI / 3, -DXM_PI / 2, -DXM_PI / 12 },
			};

			for (auto& firstPrism : m_Prisms)
			{
				firstPrism.Transformation = glm::translate(firstPrism.Point) * glm::yawPitchRoll(firstPrism.yRotation, 0.0f, firstPrism.zRotation) *
					glm::scale(glm::vec3(1.0f, 1.0f, tanf(firstPrism.Angle / 2)));

				{
					glm::vec4 vector1 = firstPrism.Transformation * glm::vec4(-0.2f, 0.2, 0.0f, 1.0);
					glm::vec4 vector2 = firstPrism.Transformation * glm::vec4(0.2f, 0.2, 0.0f, 1.0);
					glm::vec4 vector3 = firstPrism.Transformation * glm::vec4(0.2f, -0.2, -0.4f, 1.0);
					glm::vec3 currentNormal =
						glm::normalize(glm::cross(glm::vec3(vector3 - vector1), glm::vec3(vector2 - vector1)));
					planes.push_back({ glm::vec3(vector1), currentNormal, airGlass });
				}

				{
					glm::vec4 vector1 = firstPrism.Transformation * glm::vec4(-0.2f, 0.2, 0.0f, 1.0);
					glm::vec4 vector2 = firstPrism.Transformation * glm::vec4(0.2f, 0.2, 0.0f, 1.0);
					glm::vec4 vector3 = firstPrism.Transformation * glm::vec4(-0.2f, -0.2, 0.4f, 1.0);
					glm::vec3 currentNormal =
						glm::normalize(glm::cross(glm::vec3(vector3 - vector1), glm::vec3(vector2 - vector1)));
					planes.push_back({ glm::vec3(vector1), currentNormal, glassAir });
				}
			}
			planes.push_back({ { 0.0f, 0.0f, 3.0f },{ 0.0f, 0.0f, 1.0f } });
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
				m_RoomMeshRenderer.Render();

				/* Updating rays and redering them. */
				if (!m_AreRaysSynced)
				{
					m_AreRaysSynced = true;
					GenerateRaysMesh(100);
				}
				m_RaysMeshRenderer.Render();
				m_RaysProjectionMeshRenderer.Render();
			}
			m_Device->EndScene();
			m_Device->Present(nullptr, nullptr, nullptr, nullptr);
		}

		// -----------------------
		void GenerateRaysMesh(unsigned partitioning)
		{
			// double refractiveIndex1;
			for (auto i = 0u; i < partitioning; ++i)
			{
				/// @todo Move initial values outside somewhere.
				Vector3 coord = { 0.0f, 0.8f, 0.0f };
				Vector3 direction = m_Prisms[0].Point - coord;

				auto static const violetWaveLength = 380.0;
				auto static const redWaveLength = 740.0;
				auto const waveLength = violetWaveLength + i * (redWaveLength - violetWaveLength) / partitioning;
				auto const rgb = ConvertWaveLengthToRGB(waveLength);

				for (auto j = 0u; j < planes.size(); ++j)
				{
					auto& plane = planes[j];

					m_RaysMesh.AddVertex({ coord, rgb });

					// Computing the intercestion coordinates of the ray and refractive
					// plane.
					coord = Presentation::Intersect(plane, coord, direction);
					m_RaysMesh.AddVertex({ coord, rgb });

					// And finally refracting a direction.
					direction = Presentation::Refract(direction, plane, i);
					direction = direction;

					if (j == planes.size() - 1)
					{
						auto const scale = 0.06f;
						glm::vec3 static const triangleVert[] = {
							{ 0.0f,  0.5f, 0.0f },
							{ 1.0f, -0.5f, 0.0f },
							{ -1.0f, -0.5f, 0.0f },
						};
						glm::vec3 static const uvOffset = { 0.5f, 0.5f, 0.0f };

						m_RaysProjectionMesh.AddVertex({ triangleVert[0] * scale + coord,{ 0.0, 0.0, -1.0f }, rgb, triangleVert[0] + uvOffset });
						m_RaysProjectionMesh.AddVertex({ triangleVert[1] * scale + coord,{ 0.0, 0.0, -1.0f }, rgb, triangleVert[1] + uvOffset });
						m_RaysProjectionMesh.AddVertex({ triangleVert[2] * scale + coord, { 0.0, 0.0, -1.0f }, rgb, triangleVert[2] + uvOffset });
					}
				}
			}
		}

		static dxm::argb ConvertWaveLengthToRGB(double const waveLength)
		{
			auto static const gamma = 0.80;
			auto static const intensityMax = 255.0;

			double red, green, blue;

			auto static const violetMinWaveLength = 380.0;
			auto static const violetMaxWaveLength = 440.0;
			if (waveLength >= violetMinWaveLength && waveLength < violetMaxWaveLength)
			{
				red = -(waveLength - violetMaxWaveLength) / (violetMaxWaveLength - violetMinWaveLength);
				green = 0.0;
				blue = 1.0;
			}
			else
			{
				auto static const blueMinWaveLength = violetMaxWaveLength;
				auto static const blueMaxWaveLength = 490.0;
				if (waveLength >= blueMinWaveLength && waveLength < blueMaxWaveLength)
				{
					red = 0.0;
					green = (waveLength - blueMinWaveLength) / (blueMaxWaveLength - blueMinWaveLength);
					blue = 1.0;
				}
				else
				{
					auto static const skyBlueMinWaveLength = blueMinWaveLength;
					auto static const skyBlueMaxWaveLength = 500.0;
					if (waveLength >= 490.0 && waveLength < 510.0)
					{
						red = 0.0;
						green = 1.0;
						blue = -(waveLength - 510.0) / (510.0 - 490.0);
					}
					else if (waveLength >= 510.0 && waveLength < 580.0)
					{
						red = (waveLength - 510.0) / (580 - 510.0);
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
				}
			}

			double factor;
			if (waveLength >= 380 && waveLength < 420)
			{
				factor = 0.3 + 0.7 * (waveLength - 380.0) / (420.0 - 380.0);
			}
			else if (waveLength >= 420.0 && waveLength < 701.0)
			{
				factor = 1.0;
			}
			else if (waveLength >= 701.0 && waveLength < 781.0)
			{
				factor = 0.3 + 0.7 * (780.0 - waveLength) / (780.0 - 700.0);
			}
			else
			{
				factor = 0.0;
			}

			unsigned rgb[3];
			rgb[0] = red == 0.0 ? 0 : static_cast<int>(round(intensityMax * pow(red * factor, gamma)));
			rgb[1] = green == 0.0 ? 0 : static_cast<int>(round(intensityMax * pow(green * factor, gamma)));
			rgb[2] = blue == 0.0 ? 0 : static_cast<int>(round(intensityMax * pow(blue * factor, gamma)));
			return D3DCOLOR_RGBA(rgb[0], rgb[1], rgb[2], 0xFF / 2);
		}

	}; // class PresentationWidget

} // namespace Presentation
