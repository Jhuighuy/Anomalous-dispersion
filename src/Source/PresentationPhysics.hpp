// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// Copyright (C) 2016 Butakov Oleg, Plaxin Gleb.
// Computational Methods @ Computational Mathematics & Cybernetics, MSU.
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

#pragma once
#include "PresentationEngine.hpp"

#pragma warning(push, 0)
#include <glm/gtx/rotate_vector.hpp>
#pragma warning(pop)

namespace Presentation2
{
	auto static const g_VioletWaveLength = 380.0;
	auto static const g_RedWaveLength = 780.0;

	static dxm::argb ConvertWaveLengthToRGB(double waveLength)
	{
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
		return D3DCOLOR_RGBA(rgb[0], rgb[1], rgb[2], 30);
	}

	using IndexFunc = DOUBLE(*)(DOUBLE const waveLength);

	static DOUBLE DummyIndex(DOUBLE const waveLength)
	{
		(void)waveLength;
		return 1.0f;
	}

	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
	static DOUBLE GovnoAbsorptionIndex(DOUBLE const waveLength)
	{
		auto const x = waveLength / 1000.0;
		auto y = 0.3 * exp(-15 * M_PI * (x - 0.52) * (x - 0.52));
		y = ((x > 0.63) && (x < 0.691326)) ? dxm::clamp(1.0 - 4.5*y, 0.0, 1.0) * 0.9 : dxm::clamp(1.0 - y, 0.0, 1.0) * 0.9;
		//	y = dxm::clamp(1.0 - y, 0.0, 1.0) * 0.9;
		return y * 2.0 / 0.6;
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

	// -----------------------
	static void AbsorbAlpha(dxm::argb& color, DOUBLE const index)
	{
		// Modifying alpha-based absorbtion.
		auto const alphaPrev = (color >> 24 & 0xFF) / 255.0;
		auto const alpha = alphaPrev * index;
		color &= 0x00FFFFFF;
		color |= DWORD(alpha * 255.0) << 24;
	}

}	// namespace Presentation2
