/****************************************************************************
**
** Copyright (C) 2016 Plaxin Gleb, Butakov Oleg.
** Contact: kandidov_i_chickishev@ublidky.com
** License: MIT.
**
****************************************************************************/

#include "PresentationPhysics.h"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

/*!
 * Converts wave length to RGBA color.
 * 
 * @param waveLengthMcm Wave length in micrometers.
 * @param alpha The alpha channel value for the converted color.
 */
QVector4D PhSpectrum::convertWavelengthToRGBA(qreal waveLengthMcm, qreal alpha)
{
	qreal waveLength = waveLengthMcm * 1000.0f;

	qreal red, green, blue;
	if (waveLength >= VioletMinWaveLength && waveLength < VioletMaxWaveLength)
	{
		red = -(waveLength - VioletMaxWaveLength) / (VioletMaxWaveLength - VioletMinWaveLength);
		green = 0.0;
		blue = 1.0;
	}
	else if (waveLength >= BlueMinWaveLength && waveLength < BlueMaxWaveLength)
	{
		red = 0.0;
		green = (waveLength - BlueMinWaveLength) / (BlueMaxWaveLength - BlueMinWaveLength);
		blue = 1.0;
	}
	else if (waveLength >= CyanMinWaveLength && waveLength < CyanMaxWaveLength)
	{
		red = 0.0;
		green = 1.0;
		blue = -(waveLength - CyanMaxWaveLength) / (CyanMaxWaveLength - CyanMinWaveLength);
	}
	else if (waveLength >= GreenMinWaveLength && waveLength < GreenYellowMaxWaveLength)
	{
		red = (waveLength - GreenMinWaveLength) / (GreenYellowMaxWaveLength - GreenMinWaveLength);
		green = 1.0;
		blue = 0.0;
	}
	else if (waveLength >= YellowMinWaveLength && waveLength < OrangeMaxWaveLength)
	{
		red = 1.0;
		green = -(waveLength - OrangeMaxWaveLength) / (OrangeMaxWaveLength - YellowMinWaveLength);
		blue = 0.0;
	}
	else if (waveLength >= RedMinWaveLength && waveLength <= RedMaxWaveLength)
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

	qreal factor;
	if (waveLength >= VisibleSpectrumMinWaveLength && waveLength < VisibleSpectrumIntensityRise)
	{
		factor = 0.3 + 0.7 * (waveLength - VisibleSpectrumMinWaveLength) / (VisibleSpectrumIntensityRise - VisibleSpectrumMinWaveLength);
	}
	else if (waveLength >= VisibleSpectrumIntensityRise && waveLength < VisibleSpectrumIntensityFall)
	{
		factor = 1.0;
	}
	else if (waveLength >= VisibleSpectrumIntensityFall && waveLength <= VisibleSpectrumMaxWaveLength)
	{
		factor = 0.3 + 0.7 * (VisibleSpectrumMaxWaveLength - waveLength) / (VisibleSpectrumMaxWaveLength - 700);
	}
	else
	{
		factor = 0.0;
	}

	const qreal gamma = 0.9;
	QVector4D color;
	color.setX(red == 0.0 ? 0.0 : qPow(red * factor, gamma));
	color.setY(green == 0.0 ? 0.0 : qPow(green * factor, gamma));
	color.setZ(blue == 0.0 ? 0.0 : qPow(blue * factor, gamma));
	color.setW(alpha);
	return color;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

qreal PhUniformMeshIndexFunction::real(qreal x) const
{
	qreal inf = almostZero();
	if (x < mXmin)
	{
		if (x < -inf)
		{
			return 0.0;
		}

		// Performing a trivial lerp between -infinity and first mesh node.
		// This way function would be continuous.
		qreal Xminus = -inf, Xplus = mXmin;
		qreal Yminus = 0.0, Yplus = mY.first();

		return lerp(x, Xminus, Xplus, Yminus, Yplus);
	}
	if (mXmax < x)
	{
		if (x > +inf)
		{
			return 0.0;
		}

		// Performing a trivial lerp between last mesh node and +infinity.
		// This way function would be continuous.
		qreal Xminus = mXmax, Xplus = +inf;
		qreal Yminus = mY.last(), Yplus = 0.0;

		return lerp(x, Xminus, Xplus, Yminus, Yplus);
	}

	// Performing a trivial lerp between nodes of a mesh.
	qreal deltaX = mXmax - mXmin;

	qreal v = (x - mXmin) / deltaX;
	qreal Iminus = qFloor(v * (mY.size() - 1)), Iplus = qCeil(v * (mY.size() - 1));
	qreal Xminus = mXmin + deltaX * Iminus / mY.size(), Xplus = mXmin + deltaX * Iplus / mY.size();
	qreal Yminus = mY[static_cast<int>(Iminus)], Yplus = mY[static_cast<int>(Iplus)];

	return Xminus != Xplus ? lerp(x, Xminus, Xplus, Yminus, Yplus) : Yminus;
}

qreal PhUniformMeshIndexFunction::lerp(qreal x, qreal Xminus, qreal Xplus, qreal Yminus, qreal Yplus)
{
	qreal y = Yminus + (x - Xminus) / (Xplus - Xminus) * (Yplus - Yminus);
	return y;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

/*!
 * Computes a real part of this complex index function using the Kramers-Kronig relations.
 * 
 * @param Xmin Starting point of the integration range.
 * @param Xmax Ending point of the integration range.
 * @param partitioning The integration range partitioning.
 */
PhComplexIndexFunction& PhComplexIndexFunction::computeRealPartKramersKronig(qreal Xmin, qreal Xmax, int partitioning)
{
	Q_ASSERT(imaginaryPart() != nullptr);

	// Using Kramers-Kronig relations and numerical integration to compute this shit.
	QVector<qreal> realPartMesh;
	realPartMesh.resize(partitioning);

	qreal inf = imaginaryPart()->almostZero();

#if !QT_NO_DEBUG
	QTime time;
	time.start();
#endif
	
	//! @todo
	PhParabolicIndexFunction baseRefractiveIndex;
	//! @todo

#pragma omp parallel for
	for (int i = 0; i < partitioning; ++i)
	{
		static const qreal delta = 1e-8;

		qreal x = Xmin + i * (Xmax - Xmin) / partitioning;
		qreal y = baseRefractiveIndex.real(x) + simpsonIntegrate(delta, inf, partitioning * 10, [&](qreal z)
		                                 {
			                                 return 1.0 / M_PI * (imaginary(x + z) / -z + imaginary(x - z) / +z);
		                                 });
		realPartMesh[i] = y;
	}

#if !QT_NO_DEBUG
	qDebug() << "Kramers-Kronig computation elapsed: " << time.elapsed();
#endif

	PhUniformMeshIndexFunction_p realPart = PhUniformMeshIndexFunction::create();
	realPart->setMesh(realPartMesh).setXRange(Xmin, Xmax);
	return setRealPart(realPart);
}

template<typename T>
qreal PhComplexIndexFunction::simpsonIntegrate(qreal Xmin, qreal Xmax, int partitioning, const T& F)
{
	// Numerical integration using the Simpson's rule.
	qreal Xrange = Xmax - Xmin;
	qreal step = Xrange / partitioning;
	qreal halfStep = 0.5 * step;

	qreal value = 0.0;
	for (int i = 0; i < partitioning - 1; ++i)
	{
		qreal Xim = Xmin + i * step, Xiz = Xim + halfStep, Xip = Xiz + halfStep;
		qreal Yim = F(Xim), Yiz = F(Xiz), Yip = F(Xip);

		value += Yim + 4.0 * Yiz + Yip;
	}
	return step * value / 6.0;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

/*!
 * Refracts the beam.
 * 
 * @param beam The actual beam to refract.
 * @param enters Determines whether the beam enters or leaves the object. 
 */
void PhRefractivePlane::refractBeam(PhBeam& beam, bool enters) const
{
	Q_ASSERT(!beam.empty());

	const PhBeamPart& prevPart = beam.last();
	PhBeamPart newPart = prevPart;
	if (intersect(newPart, prevPart))
	{
		if (!opaque())
		{
			if (!refract(beam.waveLengthMcm(), newPart, prevPart, enters))
			{
				qDebug() << "Fuck this shit. I'm out. (Ray has been internally reflected, but it is not exactly.)";
				newPart.bad = true;
			}
		}
	}
/*	else
	{
		newPart.bad = true;
    } */
	beam.push_back(newPart);
}

bool PhRefractivePlane::refract(qreal waveLength, PhBeamPart& newBeam, const PhBeamPart& beam, bool enters) const
{
	Q_ASSERT(refractiveIndex() != nullptr);
	Q_ASSERT(refractiveIndex()->realPart() != nullptr);

	qreal refrIndex = refractiveIndex()->real(waveLength);
	if (!enters)
	{
		refrIndex = 1.0 / refrIndex;
	}

	qreal cosAngleBefore = QVector3D::dotProduct(beam.direction, normal()) / beam.direction.length() / normal().length();
	qreal angleBefore = qAcos(cosAngleBefore);
	qreal sinAngleAfter = 1.0 / refrIndex * qSin(angleBefore);
	if (-1.0 > sinAngleAfter || sinAngleAfter > 1.0)
	{
		newBeam.direction = beam.direction;
		return false;
	}

	qreal angleAfter = qAsin(sinAngleAfter);
	qreal angleDelta = angleBefore - angleAfter;

	// Rotating the direction towards the ray normal.
	QVector3D rayNormal = QVector3D::crossProduct(beam.direction, normal());
	QMatrix4x4 rayRotationMatrix;
	rayRotationMatrix.setToIdentity();
	rayRotationMatrix.rotate(static_cast<float>(qRadiansToDegrees(angleDelta)), rayNormal);
	newBeam.direction = rayRotationMatrix * beam.direction;

	// Absorbing the color.
	if (!enters && refractiveIndex()->imaginaryPart() != nullptr)
	{
        static const qreal magicConstant = 200.5;

		qreal abspIndex = refractiveIndex()->imaginary(waveLength);
        qreal distance = newBeam.position.distanceToPoint(beam.position);
        newBeam.alpha *= static_cast<float>(qExp(-abspIndex * magicConstant * distance));
	}

	return true;
}

bool PhRefractivePlane::intersect(PhBeamPart& newBeam, const PhBeamPart& beam) const
{
	float dp1 = QVector3D::dotProduct(point() - beam.position, normal());
	float dp2 = QVector3D::dotProduct(beam.direction, normal());
	if (dp2 != 0.0f)
	{
		float v = dp1 / dp2;
		newBeam.position = beam.position + v * beam.direction;
		return isInBound(newBeam.position);
	}
	return false;
}

bool PhRefractivePlane::isInBound(const QVector3D& v) const
{
	for (int i = 0; i < 3; ++i)
	{
		float min = qMin(minBound()[i], maxBound()[i]);
		float max = qMax(minBound()[i], maxBound()[i]);
		if (min > v[i] || v[i] > max)
		{
			return false;
		}
	}
	return true;
}

PhBeamCone& PhBeamCone::setPartitioning(int partitioning)
{
	Q_ASSERT(size() == 0 && "Partitioning was already set!");
	resize(partitioning);

	qreal step = PhSpectrum::VisibleSpectrumRange / 1000.0 / partitioning;
	first().setWaveLengthMcm(PhSpectrum::VisibleSpectrumMinWaveLength / 1000.0);
	for (int i = 1; i < size(); ++i)
	{
		qreal const waveLengthMcm = (*this)[i - 1].waveLengthMcm() + step;
		(*this)[i].setWaveLengthMcm(waveLengthMcm);
	}
	return *this;
}

PhBeamCone& PhBeamCone::setStartPosition(const QVector3D& startPosition)
{
	Q_ASSERT(partitioning() != 0 && "Partitioning was not set!");
	for (PhBeam& beam : *this)
	{
		beam.setStartPosition(startPosition);
	}
	return *this;
}

PhBeamCone& PhBeamCone::setStartDirection(const QVector3D& startDirection)
{
	Q_ASSERT(partitioning() != 0 && "Partitioning was not set!");
	for (PhBeam& beam : *this)
	{
		beam.setStartDirection(startDirection);
	}
	return *this;
}

/*!
 * Collides a beam cone with the object.
 * @param object The actual object to refract.
 */
void PhBeamCone::collide(const IPhRefractiveObject& object)
{
#pragma omp parallel for
	for (int i = 0; i < size(); ++i)
	{
		PhBeam& beam = (*this)[i];
		object.refractBeam(beam);
	}
}

/*!
 * Extracts the collision data from the cone.
 *
 * @param[out] levelSlice Output for the collision level.
 * @param levelIndex Index of the collision.
 * @param alphaMultiplier Alpha multiplier for generated colors.
 * @param scaleMultiplier Scale multiplier for collision vertices. 
 */
void PhBeamCone::getCollisionLevel(PhBeamCollisionInfo& levelSlice, int levelIndex, 
								   float alphaMultiplier, const QVector3D& scaleMultiplier) const
{
	Q_ASSERT(partitioning() != 0 && "Partitioning was not set!");
	Q_ASSERT(first().size() > levelIndex && "Level index was out of bounds!");

	levelSlice.clear();
    levelSlice.push_back({ first().at(levelIndex).position, PhSpectrum::convertWavelengthToRGBA(first().waveLengthMcm(),
                           first().at(levelIndex).alpha * alphaMultiplier) });
	for (int i = 1; i < partitioning(); ++i)
	{
		if (at(i).at(levelIndex).bad)
		{
			continue;
		}

		PhBeamInfo& prevInfo = levelSlice.last();
        PhBeamInfo info = { at(i).at(levelIndex).position, PhSpectrum::convertWavelengthToRGBA(at(i).waveLengthMcm(),
                            at(i).at(levelIndex).alpha * alphaMultiplier) };

		// Merging to nearby points because of both optimization and color blending.
		const float eps = 1e-3f / partitioning();
		if (info.position.distanceToPoint(prevInfo.position) < eps)
		{
			prevInfo.position = 0.5f * (prevInfo.position + info.position);
			prevInfo.color = prevInfo.color + 0.2f * info.color;
            prevInfo.color.setW(alphaMultiplier);
		}
		else
		{
			levelSlice.push_back(info);
		}
	}

	bool hasScaleMultiplier = scaleMultiplier.x() != 1.0f || scaleMultiplier.y() != 1.0f || scaleMultiplier.z() != 0.0f;
	if (hasScaleMultiplier && levelSlice.size() > 1)
	{
		//! @todo Move this code to some template function.
		QVector3D minBound = levelSlice[0].position;
		QVector3D maxBound = levelSlice[0].position;
		for (int i = 1; i < levelSlice.size(); ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				minBound[j] = qMin(minBound[j], levelSlice[i].position[j]);
				maxBound[j] = qMax(maxBound[j], levelSlice[i].position[j]);
			}
		}

		QVector3D midPoint = 0.5f * (minBound + maxBound);
		for (int i = 0; i < levelSlice.size(); ++i)
		{
			QVector3D& position = levelSlice[i].position;
			QVector3D deltaPosition = position - midPoint;
			QVector3D scaledDeltaPosition = scaleMultiplier * deltaPosition;
			position = midPoint + scaledDeltaPosition;
		}
	}
}
