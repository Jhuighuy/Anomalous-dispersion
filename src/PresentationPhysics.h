/****************************************************************************
**
** Copyright (C) 2016 Plaxin Gleb, Butakov Oleg.
** Contact: kandidov_i_chickishev@ublidky.com
** License: MIT.
**
****************************************************************************/

#pragma once

#include <QTime>
#include <QDebug>
#include <QtMath>
#include <QVector>
#include <QSharedPointer>

#include <QVector3D>
#include <QMatrix4x4>
#include <QChart>
#include <QLineSeries>

//! @todo
#ifndef DEFINE_SHARED_PTR
#define DEFINE_SHARED_PTR(Class) \
	typedef QSharedPointer<class Class> Class##_p; \
	typedef QSharedPointer<const Class> Class##_cp;
#define DEFINE_CREATE_FUNC(Class) \
	public: \
		template<typename... T>\
        static Class##_p create(T&&... args) { return Class##_p(new Class(std::forward<T>(args)...)); }\
	private:
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

/*!
 * The spectrum operations class.
 */
class PhSpectrum
{
public:
    enum Spectrum
    {
        VisibleSpectrumMinWaveLength = 380,
        VisibleSpectrumMaxWaveLength = 780,
		VisibleSpectrumRange = VisibleSpectrumMaxWaveLength - VisibleSpectrumMinWaveLength,

		VisibleSpectrumIntensityRise = 402,
        VisibleSpectrumIntensityFall = 701,

        VioletMinWaveLength = VisibleSpectrumMinWaveLength, VioletMaxWaveLength = 440,
        BlueMinWaveLength = VioletMaxWaveLength, BlueMaxWaveLength = 490,
        CyanMinWaveLength = BlueMaxWaveLength, CyanMaxWaveLength = 510,
        GreenMinWaveLength = CyanMaxWaveLength, GreenMaxWaveLength = 550,
        GreenYellowMinWaveLength = GreenMaxWaveLength, GreenYellowMaxWaveLength = 580,
        YellowMinWaveLength = GreenYellowMaxWaveLength, YellowMaxWaveLength = 590,
        OrangeMinWaveLength = YellowMaxWaveLength, OrangeMaxWaveLength = 645,
        RedMinWaveLength = OrangeMaxWaveLength, RedMaxWaveLength = VisibleSpectrumMaxWaveLength
    };

    /*!
     * Converts wave length to RGB color.
     * 
     * @param waveLengthMcm Wave length in micrometers.
     * @param alpha The alpha channel value for the converted color.
     */
    static QVector4D convertWavelengthToRGBA(qreal waveLengthMcm, qreal alpha = 0.7);
};

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

DEFINE_SHARED_PTR(PhIndexFunction)
DEFINE_SHARED_PTR(PhContinuousIndexFunction)
DEFINE_SHARED_PTR(PhParabolicIndexFunction)
DEFINE_SHARED_PTR(PhGaussianIndexFunction)
DEFINE_SHARED_PTR(PhUniformMeshIndexFunction)

/*!
 * The abstract index function class.
 */
class PhIndexFunction
{
	DEFINE_CREATE_FUNC(PhIndexFunction)

public:
    virtual ~PhIndexFunction()
    {
    }

	/*!
	 * Return value of the index function.
	 * @param x 
	 */
    virtual qreal real(qreal x) const
    {
		(void)x;
        return 0.0;
    }

	/*!
	 * Returns a point where function value at this point and minus
	 * this point is almost zero.
	 */
	virtual qreal almostZero() const
    {
		const qreal eps = 0.01;
		qreal f = 5.0;
		while (qAbs(real(f)) > eps || qAbs(real(-f)) > eps)
		{
			f += 2.0;
		}
		return f;
    }
};

/*!
 * The continuous abstract index function class.
 */
class PhContinuousIndexFunction : public PhIndexFunction
{
	DEFINE_CREATE_FUNC(PhContinuousIndexFunction)
};

/*!
 * The continuous parabolic index function class.
 * Formula: a*x^2+b*x+c
 */
class PhParabolicIndexFunction : public PhContinuousIndexFunction
{
	DEFINE_CREATE_FUNC(PhParabolicIndexFunction)

public:
    qreal quadratic() const { return mQuadratic; }
	PhParabolicIndexFunction& setQuadratic(qreal quadratic)
    {
		mQuadratic = quadratic;
        return *this;
    }

    qreal linear() const { return mLinear; }
	PhParabolicIndexFunction& setLinear(qreal linear)
    {
		mLinear = linear;
        return *this;
    }

	qreal constant() const { return mConstant; }
	PhParabolicIndexFunction& setConstant(qreal constant)
	{
		mConstant = constant;
		return *this;
	}

    qreal real(qreal x) const override
    {
        return quadratic() * x * x + linear() * x + constant();
    }

private:
	qreal mQuadratic = 0.27 * 0.9, mLinear = -0.45 * 0.9, mConstant = 1.61 * 0.9;
};

/*!
 * The continuous Gaussian index function class.
 * Formula: h*e^{ -w*pi*(x-c)^2 }
 */
class PhGaussianIndexFunction : public PhContinuousIndexFunction
{
	DEFINE_CREATE_FUNC(PhGaussianIndexFunction)

public:
    qreal center() const { return mCenter; }
    PhGaussianIndexFunction& setCenter(qreal center)
    {
		mCenter = center;
        return *this;
    }

    qreal width() const { return mWidth; }
    PhGaussianIndexFunction& setWidth(qreal width)
    {
		mWidth = width;
        return *this;
    }
	qreal height() const { return mHeight; }
    PhGaussianIndexFunction& setHeight(qreal height)
	{
		mHeight = height;
		return *this;
	}

    qreal real(qreal x) const override
    {
        return height() * qExp(-width() * M_PI * (x - center()) * (x - center()));
    }

private:
	qreal mCenter = 0.58;
	qreal mWidth = 450.0, mHeight = 1.0;
};

/*!
 * The uniform mesh abstract index function class.
 */
class PhUniformMeshIndexFunction : public PhIndexFunction
{
	DEFINE_CREATE_FUNC(PhUniformMeshIndexFunction)

public:
    qreal xMin() const { return mXmin; }
    PhUniformMeshIndexFunction& setXMin(qreal xMin)
    {
        mXmin = xMin;
        return *this;
    }
    qreal xMax() const { return mXmax; }
    PhUniformMeshIndexFunction& setXMax(qreal xMax)
    {
        mXmax = xMax;
        return *this;
    }
    PhUniformMeshIndexFunction& setXRange(qreal xMin, qreal xMax)
    {
        setXMin(xMin);
        return setXMax(xMax);
    }

    const QVector<qreal>& mesh() const { return mY; }
    PhUniformMeshIndexFunction& setMesh(const QVector<qreal>& mesh)
    {
        mY = mesh;
        return *this;
    }

	qreal almostZero() const override
	{
		return qMax(qAbs(mXmin), qAbs(mXmax)) * 5.0f;
	}

	qreal real(qreal x) const override;

private:
	static qreal lerp(qreal x, qreal Xminus, qreal Xplus, qreal Yminus, qreal Yplus);

private:
    qreal mXmin = 0.0, mXmax = 1.0;
    QVector<qreal> mY;
};

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

DEFINE_SHARED_PTR(PhComplexIndexFunction)

/*!
 * The abstract complex index function class.
 */
class PhComplexIndexFunction : public PhIndexFunction
{
	DEFINE_CREATE_FUNC(PhComplexIndexFunction)

public:
	PhIndexFunction_p realPart() const { return mRealPart; }
    PhComplexIndexFunction& setRealPart(const PhIndexFunction_p& realPart)
    {
        mRealPart = realPart;
        return *this;
    }

	PhIndexFunction_p imaginaryPart() const { return mImaginaryPart; }
    PhComplexIndexFunction& setImaginaryPart(const PhIndexFunction_p& imaginaryPart)
    {
        mImaginaryPart = imaginaryPart;
        return *this;
    }

	/*!
	 * Computes a real part of this complex index function using the Kramers-Kronig relations.
	 * 
	 * @param Xmin Starting point of the integration range.
	 * @param Xmax Ending point of the integration range.
	 * @param partitioning The integration range partitioning.
	 */
	PhComplexIndexFunction& computeRealPartKramersKronig(qreal Xmin = PhSpectrum::VisibleSpectrumMinWaveLength / 1000.0,
														 qreal Xmax = PhSpectrum::VisibleSpectrumMaxWaveLength / 1000.0,
														 int partitioning = 100);

    qreal real(qreal x) const override
    {
        Q_ASSERT(realPart() != nullptr);
        return realPart()->real(x);
    }
    virtual qreal imaginary(qreal x) const
    {
        Q_ASSERT(imaginaryPart() != nullptr);
        return imaginaryPart()->real(x);
    }

private:
	template<typename T>
	static qreal simpsonIntegrate(qreal Xmin, qreal Xmax, int partitioning, const T& F);

private:
    PhIndexFunction_p mRealPart, mImaginaryPart;
};

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

/*!
 * The beam part struct.
 */
struct PhBeamPart
{
    QVector3D position;
    QVector3D direction;
	qreal alpha = 1.0;
	bool bad = false;
};

/*!
 * The beam simplified info struct.
 */
struct PhBeamInfo
{
	QVector3D position;
	QVector4D color;
};
typedef QVector<PhBeamInfo> PhBeamCollisionInfo;

/*!
 * The single beam class.
 */
class PhBeam final : public QVector<PhBeamPart>
{
public:
	PhBeam()
	{
		push_back({});
	}

	qreal waveLengthMcm() const { return mWaveLengthMcm; }
	PhBeam& setWaveLengthMcm(qreal waveLength)
	{
		mWaveLengthMcm = waveLength;
		return *this;
	}

	const QVector3D& startPosition() const { return first().position; }
	PhBeam& setStartPosition(const QVector3D& startPosition)
	{
		Q_ASSERT(size() == 1);
		first().position = startPosition;
		return *this;
	}

	const QVector3D& startDirection() const { return first().direction; }
	PhBeam& setStartDirection(const QVector3D& startDirection)
	{
		Q_ASSERT(size() == 1);
		first().direction = startDirection;
		return *this;
	}

private:
	qreal mWaveLengthMcm = 0.0;
};

/*!
 * The base refractive object class.
 */
class IPhRefractiveObject
{
public:
	virtual ~IPhRefractiveObject()
	{
	}

	virtual void refractBeam(PhBeam& beam, bool enters = true) const
	{
		(void)enters;
		const PhBeamPart& prevPart = beam.last();
		PhBeamPart newPart = prevPart;
		beam.push_back(newPart);
	}
};

/*!
 * The refractive plane class.
 */
class PhRefractivePlane : public IPhRefractiveObject
{
public:
	bool opaque() const { return mOpaque; }
	PhRefractivePlane& setOpaque(bool opaque)
	{
		mOpaque = opaque;
		return *this;
	}

    const QVector3D& normal() const { return mNormal; }
    PhRefractivePlane& setNormal(const QVector3D& normal)
    {
        mNormal = normal;
        return *this;
    }

	const QVector3D& point() const { return minBound(); }
    const QVector3D& minBound() const { return mMinBound; }
    PhRefractivePlane& setMinBound(const QVector3D& minBound)
    {
        mMinBound = minBound;
        return *this;
    }
    const QVector3D& maxBound() const { return mMaxBound; }
    PhRefractivePlane& setMaxBound(const QVector3D& maxBound)
    {
        mMaxBound = maxBound;
        return *this;
    }

	PhComplexIndexFunction_p refractiveIndex() const { return mRefractiveIndex; }
	PhRefractivePlane& setRefractiveIndex(const PhComplexIndexFunction_p& refractiveIndex)
    {
        mRefractiveIndex = refractiveIndex;
        return *this;
    }

	/*!
	 * Refracts the beam.
	 * 
	 * @param beam The actual beam to refract.
	 * @param enters Determines whether the beam enters or leaves the object. 
	 */
	void refractBeam(PhBeam& beam, bool enters = true) const override;

private:
	bool refract(qreal waveLength, PhBeamPart& newBeam, const PhBeamPart& beam, bool enters) const;
	bool intersect(PhBeamPart& newBeam, const PhBeamPart& beam) const;
	bool isInBound(const QVector3D& v) const;

private:
	bool mOpaque = false;
    QVector3D mNormal;
    QVector3D mMinBound, mMaxBound;
    PhComplexIndexFunction_p mRefractiveIndex;
};

/*!
 * The beam cone class.
 */
class PhBeamCone final : public QVector<PhBeam>
{
public:
	void reset()
	{
		resize(0);
	}

	int partitioning() const { return size(); }
	PhBeamCone& setPartitioning(int partitioning);

	PhBeamCone& setStartPosition(const QVector3D& startPosition);
	const QVector3D& startPosition() const
	{
		Q_ASSERT(partitioning() != 0 && "Partitioning was not set!");
		return first().startPosition();
	}

	PhBeamCone& setStartDirection(const QVector3D& startDirection);
	const QVector3D& startDirection() const
	{
		Q_ASSERT(partitioning() != 0 && "Partitioning was not set!");
		return first().startDirection();
	}

	/*!
	 * Collides a beam cone with the object.
	 * @param object The actual object to refract.
	 */
	void collide(const IPhRefractiveObject& object);

	/*!
	 * Extracts the collision data from the cone.
	 *
	 * @param[out] levelSlice Output for the collision level.
	 * @param levelIndex Index of the collision.
	 * @param alphaMultiplier Alpha multiplier for the generated colors.
	 */
	void getCollisionLevel(PhBeamCollisionInfo& levelCollision, int levelIndex, float alphaMultiplier = 1.0) const;
	int collisionLevels() const
	{
		Q_ASSERT(partitioning() != 0 && "Partitioning was not set!");
		return first().size();
	}
};
