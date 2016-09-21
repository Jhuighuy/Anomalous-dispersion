#pragma once
#define _USE_MATH_DEFINES 1
#define GLM_FORCE_LEFT_HANDED 1
#include <math.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <vector>
#pragma warning(push, 0)
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>
#pragma warning(pop)
#include "ObjLoader.h"
#include <array>

#include "yx2Engine.h"

#define PRESENTATION_API
#define F_PI float(M_PI)

// ***********************************************************************************************
// ***********************************************************************************************

/**
 * \brief
 * \param value
 * \param min
 * \param max
 * \return
 */
static float clampf(float value, float min, float max) { return value < min ? min : value > max ? max : value; }

/**
 * \brief
 * \param matrix
 * \return
 */
static D3DMATRIX const* to_d3d(glm::mat4x4 const& matrix) { return reinterpret_cast<D3DMATRIX const*>(&matrix[0][0]); }

// ***********************************************************************************************
// ***********************************************************************************************

// True GOVNOKOD BELOW. WARNING!
std::array<double, 100> AirGlassComputation()
{
	std::array<double, 100> array;
	double currentCoefficient = 1.510f;
	auto rayNumber = 100;
	for (auto i = rayNumber - 1; i >= 0; --i)
	{
		array[i] = currentCoefficient;
		currentCoefficient += /*0.3 **/ (1.550f - 1.510f) / rayNumber;
	}
	return array;
}

std::array<double, 100> GlassAirComputation()
{
	std::array<double, 100> array;
	double currentCoefficient = 1.510f;
	auto rayNumber = 100;
	for (auto i = rayNumber - 1; i >= 0; --i)
	{
		array[i] = 1 / currentCoefficient;
		currentCoefficient += /*0.3 **/ (1.550f - 1.510f) / rayNumber;
	}
	return array;
}

std::array<double, 100> airGlass = AirGlassComputation();
std::array<double, 100> glassAir = GlassAirComputation();

/**
 * \brief
 */
class Presentation final : public yx2::engine::Runtime
{
	auto static const g_Width = 1280;
	auto static const g_Height = 720;
	IDirect3DTexture9 *g_texture = NULL;

	/**
	 * \brief A 3D plane, defined with normal and point on it.
	 */
	struct Plane
	{
		Vector3 Point;
		Vector3 Normal;
		std::array<double, 100> RefractiveIndex;
	}; // struct Plane

	struct Prism
	{
		Vector3 Point;
		float Angle;
		float zRotation;
		float yRotation;
		glm::mat4 Trans;
	};	// struct Prism

private:
	float m_CameraZoom = 1.0f;
	float m_CameraRotationYaw = 0;
	float m_CameraRotationPitch = 0.0f;
	POINT m_PrevMousePosition = {};

	LineMesh m_rays;
	Mesh m_Kommunalks;

	Mesh m_PrismMesh;
	std::vector<Prism> m_Prisms;

	std::vector<Plane> planes;
	POINT m_prevMousePosition = {};

private:

	/**
	 * \brief Converts a light wave length to the RGBA representation.
	 * \param[in] waveLength Light wave length in nanometers.
	 */
	PRESENTATION_API static Color ConvertWaveLengthToRGB(double const waveLength);

	// ***********************************************************************************************
	// Scene setup.
	// ***********************************************************************************************

	// ***********************************************************************************************
	// ***********************************************************************************************

public:
	PRESENTATION_API explicit Presentation(HWND const hwnd, IDirect3DDevice9* const device) 
		: Runtime(hwnd, device)
	{
	//	device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	//	device->SetRenderState(D3DRS_LIGHTING, FALSE);

		D3DXCreateTextureFromFile(m_Device, L"../backed.png", &g_texture);
		m_Device->SetTexture(0, g_texture);
		m_Device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		m_Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_Device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);   //Ignored

		D3DLIGHT9 light = {};
		light.Type = D3DLIGHT_POINT;    // make the light type 'directional light'
		light.Diffuse = { 0.5f, 0.5f, 0.5f, 1.0f };    // set the light's color
		light.Position = { 0.0f, 2.0f, 2.0f };
		light.Range = 10;
		light.Attenuation0 = 0.1;
		device->SetLight(0, &light);
		device->LightEnable(0, TRUE);

		D3DMATERIAL9 material = {};
		material.Diffuse = { 0.5f, 0.5f, 0.5f, 1.0f }; // set diffuse color to white
		material.Ambient = { 1.0f, 1.0f, 1.0f, 1.0f };
		device->SetMaterial(&material);

		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		ImportozameshenieBJD("../room.obj", m_Kommunalks);
		m_Kommunalks.SetupVerticesBuffer(device);

		ImportozameshenieBJD("../Prizbma.bjd", m_PrismMesh);
		m_PrismMesh.SetupVerticesBuffer(device);

		m_Prisms = {
			{ { 0.0f, 1.2f, 0.75f }, F_PI / 3,  },
			{ { 0.0f, 0.9f, 2.0f }, F_PI / 3, -F_PI / 2, -F_PI/12 },
		};

		for (auto& firstPrism : m_Prisms)
		{
			firstPrism.Trans = glm::translate(firstPrism.Point) * glm::yawPitchRoll(firstPrism.yRotation, 0.0f, firstPrism.zRotation) *
				glm::scale(glm::vec3(1.0f, 1.0f, tanf(firstPrism.Angle / 2)));

			{
				glm::vec4 vector1 = firstPrism.Trans * glm::vec4(-0.2f, 0.2, 0.0f, 1.0);
				glm::vec4 vector2 = firstPrism.Trans * glm::vec4(0.2f, 0.2, 0.0f, 1.0);
				glm::vec4 vector3 = firstPrism.Trans * glm::vec4(0.2f, -0.2, -0.4f, 1.0);
				glm::vec3 currentNormal =
					glm::normalize(glm::cross(glm::vec3(vector3 - vector1), glm::vec3(vector2 - vector1)));
				planes.push_back({ glm::vec3(vector1), currentNormal, airGlass });
			}

			{
				glm::vec4 vector1 = firstPrism.Trans * glm::vec4(-0.2f, 0.2, 0.0f, 1.0);
				glm::vec4 vector2 = firstPrism.Trans * glm::vec4(0.2f, 0.2, 0.0f, 1.0);
				glm::vec4 vector3 = firstPrism.Trans * glm::vec4(-0.2f, -0.2, 0.4f, 1.0);
				glm::vec3 currentNormal =
					glm::normalize(glm::cross(glm::vec3(vector3 - vector1), glm::vec3(vector2 - vector1)));
				planes.push_back({ glm::vec3(vector1), currentNormal, glassAir });
			}
		}

		planes.push_back({ { 0.0f, 0.0f, 3.0f },{ 0.0f, 0.0f, 1.0f } });

		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		GenerateRaysMesh(100, m_rays);
		m_rays.SetupVerticesBuffer(device);
	}

	PRESENTATION_API static Vector3 Intersect(Plane const& plane, Vector3 const& coord, Vector3 const& direction)
	{
		auto const v = glm::dot(plane.Point - coord, plane.Normal) / glm::dot(direction, plane.Normal);
		return coord + v * direction;
	}

	PRESENTATION_API static Vector3 Refract(Vector3 const& direction, Plane const& plane, int const waveNumber)
	{
		auto const angleBefore =
			acosf(glm::dot(direction, plane.Normal) / glm::length(plane.Normal) / glm::length(direction));
		auto const angleAfter = 
			angleBefore - asinf(clampf(1 / static_cast<float>(plane.RefractiveIndex[waveNumber]) * sinf(angleBefore), -1.0f, 1.0f));

		return glm::rotate(direction, angleAfter, glm::cross(direction, plane.Normal));
	}

	PRESENTATION_API void GenerateRaysMesh(unsigned partitioning, LineMesh& mesh)
	{
		// double refractiveIndex1;
		for (auto i = 0u; i < partitioning; ++i)
		{
			/// @todo Move initial values outside somewhere.
			Vector3 coord = {0.0f, 0.8f, 0.0f};
			Vector3 direction = m_Prisms[0].Point - coord;

			auto static const violetWaveLength = 380.0;
			auto static const redWaveLength = 740.0;
			auto const waveLength = violetWaveLength + i * (redWaveLength - violetWaveLength) / partitioning;
			auto const rgb = ConvertWaveLengthToRGB(waveLength);

			for (auto const& plane : planes)
			{
				mesh.m_Vertices.push_back({coord, rgb});

				// Computing the intercestion coordinates of the ray and refractive
				// plane.
				coord = Intersect(plane, coord, direction);
				mesh.m_Vertices.push_back({coord, rgb});

				// And finally refracting a direction.
				direction = Refract(direction, plane, i);
				direction = direction;
			}
		}
	}

	// ***********************************************************************************************
	// Scene rendering.
	// ***********************************************************************************************

	PRESENTATION_API void Update()
	{
		m_Device->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_RGBA(0, 0, 40, 0xFF), 1.0f, 0);
		m_Device->Clear(0, nullptr, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
		m_Device->BeginScene();

		{
			glm::vec4 static const cameraRotationCenter(0.0f, 1.24f, 2.0f, 1.0f);
			glm::vec4 static const cameraCenterOffset(0.0f, 0.0f, -2.5f, 1.0f);
			glm::vec4 static const cameraUp(0.0f, 1.0f, 0.0f, 1.0f);

			if (GetAsyncKeyState(VK_LBUTTON) != 0)
			{
				// L button is pressed.
				POINT mouseCurrentPosition = {};
				GetCursorPos(&mouseCurrentPosition);

				auto const deltaYaw = static_cast<float>(mouseCurrentPosition.y - m_PrevMousePosition.y) / g_Height;
				auto const deltaPitch = static_cast<float>(mouseCurrentPosition.x - m_PrevMousePosition.x) / g_Width;

				m_CameraRotationYaw += deltaPitch;
				m_CameraRotationPitch = clampf(m_CameraRotationPitch + deltaYaw, -YX2_PI / 12.0f, YX2_PI / 3.0f);
			}
			GetCursorPos(&m_PrevMousePosition);

			auto const cameraTranslation = glm::translate(glm::vec3(cameraRotationCenter));
			auto const cameraRotation = glm::yawPitchRoll(m_CameraRotationYaw, m_CameraRotationPitch, 0.0f);

			auto const eye = glm::vec3(cameraTranslation * cameraRotation * cameraCenterOffset);
			auto const center = glm::vec3(cameraRotationCenter);
			auto const up = glm::vec3(cameraRotation * cameraUp);
			m_Device->SetTransform(D3DTS_VIEW, to_d3d(glm::lookAtLH(eye, center, up)));
			m_Device->SetTransform(D3DTS_PROJECTION, to_d3d(glm::perspectiveFovLH<float>(YX2_PI / 3.0f, g_Width, g_Height, 0.01f, 100.0f)));
		}

		m_Device->SetTexture(0, g_texture);
		m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		m_Device->SetTransform(D3DTS_WORLD, to_d3d(glm::translate(glm::vec3(0,0,2))));
		m_Kommunalks.Render(m_Device);
		m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		m_Device->SetTexture(0, nullptr);

	//	m_Device->SetRenderState(D3DRS_LIGHTING, FALSE);
		m_Device->SetTransform(D3DTS_WORLD, to_d3d(glm::mat4()));
		m_rays.Render(m_Device);
	//	m_Device->SetRenderState(D3DRS_LIGHTING, TRUE);

		for (auto& prism : m_Prisms)
		{
			m_Device->SetTransform(D3DTS_WORLD, to_d3d(prism.Trans));
			m_PrismMesh.Render(m_Device);
		}
		m_Device->SetTransform(D3DTS_WORLD, to_d3d(glm::mat4()));

		m_Device->EndScene();
		m_Device->Present(nullptr, nullptr, nullptr, nullptr);
	}

}; // class Presentation

/**
* \brief Converts a light wave length to the RGBA representation.
* \param[in] waveLength Light wave length in nanometers.
*/
PRESENTATION_API inline Color Presentation::ConvertWaveLengthToRGB(double const waveLength)
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
