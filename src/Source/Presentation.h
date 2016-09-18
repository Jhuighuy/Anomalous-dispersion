#pragma once
#define _USE_MATH_DEFINES 1
#define GLM_FORCE_LEFT_HANDED 1
#include <math.h>
#include <d3d9.h>
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
		currentCoefficient += /*0.3 **/ (1.540f - 1.510f) / rayNumber;
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

	/**
	 * \brief A 3D plane, defined with normal and point on it.
	 */
	struct Plane
	{
		Vector3 Point;
		Vector3 Normal;
		std::array<double, 100> RefractiveIndex;
	}; // struct Plane

	//class PrismRenderer final : public yx2::engine::TriangleMeshRenderer
	//{
	//};	// class PrismRenderer

	struct Prism
	{
		Mesh* mesh;
		Vector3 Point;
		float angle;
		float zRotation;
		glm::mat4 Trans;
	};

private:
	yx2::engine::OrbitalCamera m_Camera;

	/**
	 * \brief A vertex of the mesh.
	 */
	LineMesh m_rays;
	Mesh m_Kommunalks;

	Mesh m_PrismMesh;
	std::vector<Prism> m_Prisms;

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
		, m_Camera(this) 
	{
	//	D3DLIGHT9 light = {};
	//	light.Type = D3DLIGHT_POINT;
	//	light.Diffuse.r = 1.0f;
	//	light.Diffuse.g = 1.0f;
	//	light.Diffuse.b = 1.0f;
	//	light.Ambient.r = 0.5f;
	//	light.Ambient.g = 0.5f;
	//	light.Ambient.b = 0.5f;
	//	light.Specular.r = 1.0f;
	//	light.Specular.g = 1.0f;
	//	light.Specular.b = 1.0f;
	//	light.Position.x = 0.5f;
	//	light.Position.y = 2.3f;
	//	light.Position.z = 1.5f;
	//	light.Attenuation0 = 0.01f;
	//	light.Range =3;
	//	device->SetLight(0, &light);
	//	device->LightEnable(0, TRUE);

	//	D3DMATERIAL9 material = {};
	//	material.Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f }; // set diffuse color to white
	//	material.Ambient = { 1.0f, 1.0f, 1.0f, 1.0f };
	////	device->SetMaterial(&material);


		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		ImportozameshenieBJD("../Kommunalks_yx2.bjd", m_Kommunalks);
		m_Kommunalks.SetupVerticesBuffer(device);

		ImportozameshenieBJD("../Prizbma.bjd", m_PrismMesh);
		m_PrismMesh.SetupVerticesBuffer(device);

		m_Prisms = {
			{ &m_PrismMesh, { 0.0f, 1.2f, 0.75f }, F_PI / 3 },
			{ &m_PrismMesh, { 0.0f, 0.6f, 2.0f }, F_PI / 3, F_PI / 2 },
		};

		for (auto& firstPrism : m_Prisms)
		{
			firstPrism.Trans = glm::translate(firstPrism.Point) * glm::yawPitchRoll(0.0f, 0.0f, firstPrism.zRotation) *
				glm::scale(glm::vec3(1.0f, 1.0f, tanf(firstPrism.angle / 2)));

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

	std::vector<Plane> planes;
	PRESENTATION_API void GenerateRaysMesh(unsigned partitioning, LineMesh& mesh)
	{
		// double refractiveIndex1;
		for (auto i = 0u; i < partitioning; ++i)
		{
			/// @todo Move initial values outside somewhere.
			Vector3 coord = {0.0f, 1.0f, 0.0f};
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

public:

	PRESENTATION_API void Update()
	{
		BeginRender();

		m_Camera.Update();

	//	m_Kommunalks.Render(m_Device);

	//	m_Device->SetRenderState(D3DRS_LIGHTING, FALSE);
		m_rays.Render(m_Device);
	//	m_Device->SetRenderState(D3DRS_LIGHTING, TRUE);

		for (auto& prism : m_Prisms)
		{
			m_Device->SetTransform(D3DTS_WORLD, to_d3d(prism.Trans));
			prism.mesh->Render(m_Device);
		}
		m_Device->SetTransform(D3DTS_WORLD, to_d3d(glm::mat4()));

<<<<<<< HEAD
		{
			glm::vec4 vector1 = firstPrism.Trans * glm::vec4(-0.2f, 0.2, 0.0f, 1.0);
			glm::vec4 vector2 = firstPrism.Trans * glm::vec4(0.2f, 0.2, 0.0f, 1.0);
			glm::vec4 vector3 = firstPrism.Trans * glm::vec4(-0.2f, -0.2, 0.4f, 1.0);
			glm::vec3 currentNormal =
				glm::normalize(glm::cross(glm::vec3(vector3 - vector1), glm::vec3(vector2 - vector1)));
			planes.push_back({glm::vec3(vector1), currentNormal, glassAir});
		}

		planes.push_back({{0.0f, 0.0f, 3.0f}, {0.0f, 0.0f, 1.0f}});

		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		// m_rays.m_Vertices.push_back(LineMeshVertex({ 0.0f, 0.0f, 0.0f }, D3DCOLOR_RGBA(0xFF, 0, 0, 0XFF))); // x
		// m_rays.m_Vertices.push_back(LineMeshVertex({1000.0f, 0.0f, 0.0f}, D3DCOLOR_RGBA(0xFF, 0, 0, 0XFF)));
		// m_rays.m_Vertices.push_back(LineMeshVertex({0.0, 0.0, 0.0}, D3DCOLOR_RGBA(0, 0xFF, 0, 0XFF))); // y
		// m_rays.m_Vertices.push_back(LineMeshVertex({0.0, 1000.0, 0.0}, D3DCOLOR_RGBA(0, 0xFF, 0, 0XFF)));
		// m_rays.m_Vertices.push_back(LineMeshVertex({0.0, 0.0, 0.0}, D3DCOLOR_RGBA(0, 0, 0xFF, 0XFF))); // 2
		// m_rays.m_Vertices.push_back(LineMeshVertex({0.0, 0.0, 1000.0}, D3DCOLOR_RGBA(0, 0, 0xFF, 0XFF)));
		GenerateRaysMesh(100, m_rays);
		m_rays.SetupVerticesBuffer(device);

		ImportozameshenieBJD("Kommunalks_yx2.bjd", m_Kommunalks);
		m_Kommunalks.SetupVerticesBuffer(device);

		ImportozameshenieBJD("Prizbma.bjd", firstPrism.mesh);
		firstPrism.mesh.SetupVerticesBuffer(device);
=======
		EndRender();
>>>>>>> origin/master
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
