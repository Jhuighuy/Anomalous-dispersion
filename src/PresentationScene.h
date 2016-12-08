/****************************************************************************
**
** Copyright (C) 2016 Plaxin Gleb, Butakov Oleg.
** License: MIT.
**
****************************************************************************/

#pragma once

#include <QtMath>
#include <QVector>

#include "widgets/SceneWidget.h"
#include "PresentationPhysics.h"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

DEFINE_SHARED_PTR(PrPrismRenderer)
DEFINE_SHARED_PTR(PrScreenRenderer)

/*!
 * The prism material enum.
 */
enum class PrPrismMaterial
{
    NormGlass = 1,
	AnomCyanine = -1,
	AnomModel = -2,
};

static bool prPrismMaterialAnomalous(PrPrismMaterial material)
{
	return static_cast<int>(material) < 0;
}

/*!
 * The prism renderer class.
 */
class PrPrismRenderer final : public ScTransparentMeshRenderer, public IPhRefractiveObject
{
    DEFINE_CREATE_FUNC(PrPrismRenderer)

public:
	PrPrismRenderer();

	PrPrismRenderer& setEnabled(bool enabled) override;
	PrPrismRenderer& setPosition(const QVector3D& position) override;
	PrPrismRenderer& setRotation(const QQuaternion& rotation) override;
	PrPrismRenderer& setScale(const QVector3D& scale) override;
	PrPrismRenderer& setOffset(const QVector3D& offset) override;

	QMatrix4x4 modelMatrix() const override;

	PhComplexIndexFunction_p refractiveIndex() const { return mFirstPlane.refractiveIndex(); }
	PrPrismRenderer& setAbsorptionIndexCenter(qreal center);
	PrPrismRenderer& setAbsorptionIndexWidth(qreal width);
	PrPrismRenderer& setAbsorptionIndexHeight(qreal height);

    float angle() const { return mAngle; }
	PrPrismRenderer& setAngle(float angle);

    PrPrismMaterial material() const { return mMaterial; }
	PrPrismRenderer& setMaterial(PrPrismMaterial material);

	bool anomaluos() const { return prPrismMaterialAnomalous(material()); }
	PrPrismRenderer& setAnomaluos(bool anomaluos)
	{
		return setMaterial(anomaluos ? PrPrismMaterial::AnomCyanine : PrPrismMaterial::NormGlass);
	}

	void render(const ScBasicCamera& camera) override;
	void refractBeam(PhBeam& beam, bool) const override;

private:
	void syncRefractivePlanes();
	void syncRefractiveIndex();

private:
	static float sLegHeight;
	static float sGimbalHeight;

    float mAngle = 60.0f;
    PrPrismMaterial mMaterial = PrPrismMaterial::NormGlass;
    ScMeshRenderer_p mPrismHolderBase, mPrismHolderLeg, mPrismHolderGimbal;
	
	bool mPlanesSynced = false;
	bool mIndexSynced = false;
	PhRefractivePlane mFirstPlane;
	PhRefractivePlane mSecondPlane;
};

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

DEFINE_SHARED_PTR(PrRoomRenderer)
DEFINE_SHARED_PTR(PrScreenRenderer)

/*!
 * The room renderer class.
 */
class PrRoomRenderer final : public ScMeshRenderer
{
	DEFINE_CREATE_FUNC(PrRoomRenderer)

public:
	PrRoomRenderer();
};

/*!
 * The screen renderer class.
 */
class PrScreenRenderer final : public ScMeshRenderer, public IPhRefractiveObject
{
	DEFINE_CREATE_FUNC(PrScreenRenderer)

public:
	PrScreenRenderer();
    
	PrScreenRenderer& setPosition(const QVector3D& position) override;
	PrScreenRenderer& setRotation(const QQuaternion& rotation) override;
	PrScreenRenderer& setScale(const QVector3D& scale) override;
	PrScreenRenderer& setOffset(const QVector3D& offset) override;

	const QVector3D& normal() const { return mPlane.normal(); }

	void refractBeam(PhBeam& beam, bool) const override;

private:
	PhRefractivePlane mPlane;
};

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

DEFINE_SHARED_PTR(PrBeamConeRenderer)

/*!
 * The light beam renderer class.
 */
class PrBeamConeRenderer final : public ScTransparentMeshRenderer
{
	DEFINE_CREATE_FUNC(PrBeamConeRenderer)

public:
	PrBeamConeRenderer();

	PrBeamConeRenderer& setRotation(const QQuaternion& rotation) override;
	PrBeamConeRenderer& setScale(const QVector3D& scale) override;
	PrBeamConeRenderer& setOffset(const QVector3D& offset) override;

	QMatrix4x4 modelMatrix() const override
	{
		QMatrix4x4 model;
		model.setToIdentity();
		return model;
	}

	void render(const ScBasicCamera& camera) override;
	void recalculateMesh(const QVector<PrPrismRenderer_p>& prismRenderers, 
						 const PrScreenRenderer_p& screenRenderer, 
						 const ScProjectionCamera_p& projCamera);

private:
    static const int sRaysCount = 1000u;
    ScMeshRenderer_p mProjectionRenderer;
};

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

DEFINE_SHARED_PTR(PrScene)

/*!
 * The presentation scene class.
 */
class PrScene final : public ScScene
{
	DEFINE_CREATE_FUNC(PrScene)

public:
	void init() override;
	void render() override;

	void setOnePrismScene();
	void setTwoPrismsScene();
	void recalculateBeams();

	PrPrismRenderer_p firstPrism() const { return mPrismRenderers.first(); }
	PrPrismRenderer_p secondPrism() const { return mPrismRenderers.last(); }

private:
	void onMouseDrag(const QVector2D& dragDeltaCoords) override final;
	void onResize(float width, float height) override final;

private:
	ScOrbitalCamera_p mMainCamera;
	ScProjectionCamera_p mProjCamera;

	PrRoomRenderer_p mRoomRenderer;
	PrScreenRenderer_p mScreenRenderer;

	QVector<PrPrismRenderer_p> mPrismRenderers;
	PrBeamConeRenderer_p mBeamsRenderer;
};
