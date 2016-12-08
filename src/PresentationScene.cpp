/****************************************************************************
**
** Copyright (C) 2016 Plaxin Gleb, Butakov Oleg.
** License: MIT.
**
****************************************************************************/

#include "PresentationScene.h"
#include "PresentationGeometry.h"
#include "widgets/SceneWidgetAdvanced.h"
#include "SceneMeshes.h"

#include <typeinfo>

static QOpenGLShaderProgram_p prUnlitColoredShaderProgram()
{
	static const QOpenGLShaderProgram_p unlitColoredShaderProgram = scLoadShaderProgram(":/shaders/vertex.glsl",
																						":/shaders/fragUnlit.glsl");
	return unlitColoredShaderProgram;
}
static QOpenGLShaderProgram_p prUnlitTexturedShaderProgram()
{
	static const QOpenGLShaderProgram_p unlitTexturedShaderProgram = scLoadShaderProgram(":/shaders/vertex.glsl", 
																						 ":/shaders/fragUnlitTextured.glsl");
	return unlitTexturedShaderProgram;
}

static QOpenGLShaderProgram_p prLitColoredShaderProgram()
{
	static const QOpenGLShaderProgram_p litColoredShaderProgram = scLoadShaderProgram(":/shaders/vertex.glsl",
																					  ":/shaders/fragLit.glsl");
	return litColoredShaderProgram;
}
static QOpenGLShaderProgram_p prLitRefrShaderProgram()
{
	static const QOpenGLShaderProgram_p litRefrShaderProgram = scLoadShaderProgram(":/shaders/vertex.glsl",
																				   ":/shaders/fragRefract.glsl");
	return litRefrShaderProgram;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

float PrPrismRenderer::sLegHeight = 0.0f;
float PrPrismRenderer::sGimbalHeight = 0.0f;

PrPrismRenderer::PrPrismRenderer()
{
	static const QOpenGLTexture_p prismEnvironmentCubemap = scLoadCubemap(":/gfx/roomCubemap", ".png");
	static const ScEditableMesh_p prismMesh = ScEditableMesh::create(prismVertices, _countof(prismVertices), true);
	// ----------------------
	static const ScEditableMesh_p prismHolderBaseMesh = ScEditableMesh::create(prismHolderBaseVertices, _countof(prismHolderBaseVertices));
	static const ScEditableMesh_p prismHolderLegMesh = ScEditableMesh::create(prismHolderLegVertices, _countof(prismHolderLegVertices), false, true);
	static const ScEditableMesh_p prismHolderGimbalMesh = ScEditableMesh::create(prismHolderGimbalVertices, _countof(prismHolderGimbalVertices), false, true);

	if (sLegHeight == 0.0f)
	{
		sLegHeight = prismHolderLegMesh->maxBound().y() - prismHolderLegMesh->minBound().y();
		sGimbalHeight = prismHolderGimbalMesh->maxBound().y() - prismHolderGimbalMesh->minBound().y();
	}

	mPrismHolderBase = ScMeshRenderer::create();
	mPrismHolderBase
		->enable()
		.setMesh(prismHolderBaseMesh)
		.setShaderProgram(prLitColoredShaderProgram());
	// ----------------------
	mPrismHolderLeg = ScMeshRenderer::create();
	mPrismHolderLeg
		->enable()
		.setMesh(prismHolderLegMesh)
		.setShaderProgram(prLitColoredShaderProgram());
	// ----------------------
	mPrismHolderGimbal = ScMeshRenderer::create();
	mPrismHolderGimbal
		->enable()
		.setMesh(prismHolderGimbalMesh)
		.setShaderProgram(prLitColoredShaderProgram());

	enable();
	setMesh(prismMesh);
	setShaderProgram(prLitRefrShaderProgram());
	setDiffuseTexture(prismEnvironmentCubemap);
	// ----------------------

	//! @todo
	ScOffsetTransform::setOffset({ 0.0f, -sGimbalHeight / 6.0f, 0.0f, });
	setAngle(mAngle);
	setMaterial(mMaterial);
}

PrPrismRenderer& PrPrismRenderer::setEnabled(bool enabled)
{
	ScMeshRenderer::setEnabled(enabled);
	mPrismHolderBase->setEnabled(enabled);
	mPrismHolderLeg->setEnabled(enabled);
	mPrismHolderGimbal->setEnabled(enabled);
	return *this;
}
PrPrismRenderer& PrPrismRenderer::setPosition(const QVector3D& position)
{
	ScMeshRenderer::setPosition(position);
	mPrismHolderBase->setPosition({ position.x(), 0.0f, position.z() });
	mPrismHolderLeg->setPosition({ position.x(), position.y() - sLegHeight - sGimbalHeight, position.z() });
	mPrismHolderGimbal->setPosition(position);
	mPlanesSynced = false;
	return *this;
}
PrPrismRenderer& PrPrismRenderer::setRotation(const QQuaternion& rotation)
{
	ScMeshRenderer::setRotation(rotation);
	mPrismHolderGimbal->setRotationDegrees({ 0.0f, 0.0f, rotationDegrees().z() });
	mPlanesSynced = false;
	return *this;
}
PrPrismRenderer& PrPrismRenderer::setScale(const QVector3D& scale)
{
	ScMeshRenderer::setScale(scale);
	setAngle(mAngle);
	return *this;
}
PrPrismRenderer& PrPrismRenderer::setOffset(const QVector3D&)
{
	Q_ASSERT(!"Offsetting should not be manipulated directly.");
	return *this;
}

QMatrix4x4 PrPrismRenderer::modelMatrix() const
{
	QVector3D anglingScale(1.0f, 1.0f, tanf(qDegreesToRadians(angle() / 2.0f)));

	QMatrix4x4 model;
	model.setToIdentity();
	model.translate(position());
	model.rotate(rotation());
	model.translate(offset());
	model.scale(scale() * anglingScale);
	return model;
}

PrPrismRenderer& PrPrismRenderer::setAbsorptionIndexCenter(qreal center)
{
	PhComplexIndexFunction_p refractiveIndex = mFirstPlane.refractiveIndex();
	Q_ASSERT(refractiveIndex != nullptr);
	Q_ASSERT(refractiveIndex->imaginaryPart() != nullptr);

	PhGaussianIndexFunction_p abspIndex = refractiveIndex->imaginaryPart().dynamicCast<PhGaussianIndexFunction>();
	Q_ASSERT(abspIndex != nullptr);

	abspIndex->setCenter(center);
	mIndexSynced = false;
	return *this;
}
PrPrismRenderer& PrPrismRenderer::setAbsorptionIndexWidth(qreal width)
{
	PhComplexIndexFunction_p refractiveIndex = mFirstPlane.refractiveIndex();
	Q_ASSERT(refractiveIndex != nullptr);
	Q_ASSERT(refractiveIndex->imaginaryPart() != nullptr);

	PhGaussianIndexFunction_p abspIndex = refractiveIndex->imaginaryPart().dynamicCast<PhGaussianIndexFunction>();
	Q_ASSERT(abspIndex != nullptr);

	abspIndex->setWidth(width);
	mIndexSynced = false;
	return *this;
}
PrPrismRenderer& PrPrismRenderer::setAbsorptionIndexHeight(qreal height)
{
	PhComplexIndexFunction_p refractiveIndex = mFirstPlane.refractiveIndex();
	Q_ASSERT(refractiveIndex != nullptr);
	Q_ASSERT(refractiveIndex->imaginaryPart() != nullptr);

	PhGaussianIndexFunction_p abspIndex = refractiveIndex->imaginaryPart().dynamicCast<PhGaussianIndexFunction>();
	Q_ASSERT(abspIndex != nullptr);

	abspIndex->setHeight(height);
	mIndexSynced = false;
	return *this;
}

PrPrismRenderer& PrPrismRenderer::setAngle(float angle)
{
	mAngle = angle;
	mPlanesSynced = false;
	return *this;
}
PrPrismRenderer& PrPrismRenderer::setMaterial(PrPrismMaterial material)
{
	PhComplexIndexFunction_p refrIndex = refractiveIndex();
	if (refrIndex == nullptr)
	{
		refrIndex = PhComplexIndexFunction::create();
	}

	if (prPrismMaterialAnomalous(material))
	{
		refrIndex->setImaginaryPart(PhGaussianIndexFunction::create());
		refrIndex->computeRealPartKramersKronig();
	}
	else
	{
		refrIndex->setRealPart(PhParabolicIndexFunction::create());
		refrIndex->setImaginaryPart(PhIndexFunction_p(nullptr));
	}
	
	mFirstPlane.setRefractiveIndex(refrIndex);
	mSecondPlane.setRefractiveIndex(refrIndex);
	mMaterial = material;
	return *this;
}

void PrPrismRenderer::render(const ScBasicCamera& camera)
{
	mPrismHolderBase->render(camera);
	mPrismHolderLeg->render(camera);
	mPrismHolderGimbal->render(camera);
	ScTransparentMeshRenderer::render(camera);
}

void PrPrismRenderer::refractBeam(PhBeam& beam, bool) const
{
	if (!enabled())
	{
		return;
	}

	if (!mPlanesSynced)
	{
		const_cast<PrPrismRenderer*>(this)->syncRefractivePlanes();
	}
	if (!mIndexSynced)
	{
		const_cast<PrPrismRenderer*>(this)->syncRefractiveIndex();
	}

	mFirstPlane.refractBeam(beam, true);
	mSecondPlane.refractBeam(beam, false);
}

void PrPrismRenderer::syncRefractivePlanes()
{
	QMatrix4x4 model = modelMatrix();

	//! @todo Get this values out of the mesh.
	QVector3D fp1 = model * QVector3D(-0.1f, -0.1f, +0.0f);
	QVector3D fp2 = model * QVector3D(+0.1f, -0.1f, +0.0f);
	QVector3D fp3 = model * QVector3D(+0.1f, +0.1f, +0.2f);
	mFirstPlane
		.setMinBound(fp1)
		.setMaxBound(fp3)
		.setNormal(-QVector3D::normal(fp1, fp2, fp3));

	QVector3D sp1 = model * QVector3D(-0.1f, -0.1f, +0.0f);
	QVector3D sp2 = model * QVector3D(+0.1f, -0.1f, +0.0f);
	QVector3D sp3 = model * QVector3D(-0.1f, +0.1f, -0.2f);
	mSecondPlane
		.setMinBound(sp2)
		.setMaxBound(sp3)
		.setNormal(-QVector3D::normal(sp1, sp2, sp3));

	mPlanesSynced = true;
}
void PrPrismRenderer::syncRefractiveIndex()
{
	if (prPrismMaterialAnomalous(material()))
	{
		Q_ASSERT(refractiveIndex() != nullptr);
		Q_ASSERT(refractiveIndex()->imaginaryPart() != nullptr);
		refractiveIndex()->computeRealPartKramersKronig();
	}

	mIndexSynced = true;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

PrRoomRenderer::PrRoomRenderer()
{
	static const QOpenGLTexture_p roomLightingTexture = scLoadTexture(":/gfx/roomLightMap.png");
	static const ScEditableMesh_p roomMesh = ScEditableMesh::create(roomVertices, _countof(roomVertices));

	setMesh(roomMesh);
	setShaderProgram(prUnlitTexturedShaderProgram());
	setDiffuseTexture(roomLightingTexture);
}

// ----------------------

PrScreenRenderer::PrScreenRenderer()
{
	static const QOpenGLTexture_p screenLightingTexture = scLoadTexture(":/gfx/screenLightMap.png");
	static const ScEditableMesh_p screenMesh = ScEditableMesh::create(screenVertices, _countof(screenVertices));

	setMesh(screenMesh);
	setShaderProgram(prUnlitTexturedShaderProgram());
	setDiffuseTexture(screenLightingTexture);

	mPlane
		.setOpaque(true)
		.setNormal({ 0.0f, 0.0f, -1.0f });
}

PrScreenRenderer& PrScreenRenderer::setPosition(const QVector3D& position)
{
	static const QVector3D defaultMinBound(-1.77f, 0.485f, -1.49f);
	static const QVector3D defaultMaxBound(+1.77f, 2.070f, -1.49f);

	mPlane
		.setMinBound(defaultMinBound + position)
		.setMaxBound(defaultMaxBound + position);

	ScMeshRenderer::setPosition(position);
	return *this;
}
PrScreenRenderer& PrScreenRenderer::setRotation(const QQuaternion&)
{
	Q_ASSERT(!"Rotation should not be manipulated directly.");
	return *this;
}
PrScreenRenderer& PrScreenRenderer::setScale(const QVector3D&)
{
	Q_ASSERT(!"Scaling should not be manipulated directly.");
	return *this;
}
PrScreenRenderer& PrScreenRenderer::setOffset(const QVector3D&)
{
	Q_ASSERT(!"Offsetting should not be manipulated directly.");
	return *this;
}

void PrScreenRenderer::refractBeam(PhBeam& beam, bool) const
{
	mPlane.refractBeam(beam);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

PrBeamConeRenderer::PrBeamConeRenderer()
{
	static const QOpenGLTexture_p colorMaskTexture = scLoadTexture(":/gfx/color_mask.png");

	setMesh(ScEditableMesh::create());
	setShaderProgram(prUnlitColoredShaderProgram());
	// ----------------------
	mProjectionRenderer = ScMeshRenderer::create();
	mProjectionRenderer
		->setMesh(ScEditableMesh::create())
		.setShaderProgram(prUnlitColoredShaderProgram())
		.setDiffuseTexture(colorMaskTexture);
}

PrBeamConeRenderer& PrBeamConeRenderer::setRotation(const QQuaternion&)
{
	Q_ASSERT(!"Rotation should not be manipulated directly.");
	return *this;
}
PrBeamConeRenderer& PrBeamConeRenderer::setScale(const QVector3D&)
{
	Q_ASSERT(!"Scaling should not be manipulated directly.");
	return *this;
}
PrBeamConeRenderer& PrBeamConeRenderer::setOffset(const QVector3D&)
{
	Q_ASSERT(!"Offsetting should not be manipulated directly.");
	return *this;
}

void PrBeamConeRenderer::recalculateMesh(const QVector<PrPrismRenderer_p>& prismRenderers, 
									     const PrScreenRenderer_p& screenRenderer,
										 const ScProjectionCamera_p& projCamera)
{
	PhBeamCone beamCone;
	beamCone
		.setPartitioning(sRaysCount)
		.setStartPosition(position())
		.setStartDirection(screenRenderer->normal());

	for (const PrPrismRenderer_p& prismRenderer : prismRenderers)
	{
		beamCone.collide(*prismRenderer);
	}
	beamCone.collide(*screenRenderer);

	QVector<ScVertexData> vertices;
	QVector<ScVertexData> projVertices;

	PresentationGeometry::generateBeamMesh(beamCone, vertices);
	PresentationGeometry::generateBeamProjMesh(beamCone, screenRenderer->normal(), projVertices);
	
	mesh()->setVertices(vertices.data(), vertices.size(), true);
	mProjectionRenderer->mesh()->setVertices(projVertices.data(), projVertices.size(), false, true);

	const QVector3D& projMinBound = mProjectionRenderer->mesh()->minBound();
	const QVector3D& projMaxBound = mProjectionRenderer->mesh()->maxBound();

	float projBoundWidth = projMaxBound.x() - projMinBound.x();
	float projBoundHeight = projMaxBound.y() - projMinBound.y();

	projCamera->setPosition(0.5f * (projMaxBound + projMinBound) + QVector3D{0.0f, 0.0f, 1.0f});
	projCamera->setSize(1.1f * qMax(projBoundWidth, projBoundHeight));
}

void PrBeamConeRenderer::render(const ScBasicCamera& camera)
{
	glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	mProjectionRenderer->render(camera);
	if (typeid(camera) == typeid(ScOrbitalCamera))
	{
		// We want our mesh been rendered only by main camera.
        ScTransparentMeshRenderer::render(camera);
	}

	glDisable(GL_BLEND);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

void PrScene::init()
{
	mMainCamera = ScOrbitalCamera::create();
	mMainCamera
		->setRotationMinBound({ -24.0f, -145.0f })
		.setRotationMaxBound({ +15.0f, +145.0f })
		.setRotationCenter({ 0.0f, 1.0f, -2.0f })
		.setRotationOrbit({ 0.0f, 0.0f, 1.8f })
		.rotate({ 0.0f, 45.0f })
		.setClearColor({ 1.0f, 1.0f, 1.0f, 1.0f });
	mProjCamera = ScProjectionCamera::create();
	mProjCamera
		->setClearColor({ 0.0f, 0.0f, 0.0f, 1.0f })
		.setViewport({ 1.0f - 0.35f - 0.05f, 0.05f, 0.35f, 0.35f });
	// ----------------------
	mRoomRenderer = PrRoomRenderer::create();
	mRoomRenderer
		->enable()
		.setPosition({ 0.0f, 0.0f, -2.0f })
		.setScale({ -1.0f, 1.0f, 1.0f });
	mScreenRenderer = PrScreenRenderer::create();
	mScreenRenderer
		->enable()
		.setPosition({ 0.0f, 0.0f, -2.0f });
	// ----------------------
	mPrismRenderers = { PrPrismRenderer::create(), PrPrismRenderer::create() };
	setOnePrismScene();
	// ----------------------
	mBeamsRenderer = PrBeamConeRenderer::create();
	mBeamsRenderer
		->enable()
		.setPosition({ 0.0f, 0.72f, 2.0f });

	recalculateBeams();
}
void PrScene::render()
{
	mMainCamera->beginScene();
	{
		mRoomRenderer->render(*mMainCamera);
		mScreenRenderer->render(*mMainCamera);
		mBeamsRenderer->render(*mMainCamera);

		for (const PrPrismRenderer_p& prismRenderer : mPrismRenderers)
		{
			prismRenderer->render(*mMainCamera);
		}
	}
	mMainCamera->endScene();
	// ----------------------
	mProjCamera->beginScene();
	{
		mBeamsRenderer->render(*mProjCamera);
	}
	mProjCamera->endScene();
}

void PrScene::setOnePrismScene()
{
	mPrismRenderers.first()
		->setAngle(60.0f)
		.setMaterial(PrPrismMaterial::NormGlass)
		.enable()
		.setPosition({ 0.0f, 0.75f, -1.3f })
		.setRotationDegrees({ 0.0f, 0.0f, 0.0f });
	mPrismRenderers.last()
		->disable();
}
void PrScene::setTwoPrismsScene()
{
	mPrismRenderers.first()
		->setAngle(60.0f)
		.setMaterial(PrPrismMaterial::NormGlass)
		.enable()
		.setPosition({ 0.0f, 0.75f, -1.4f })
		.setRotationDegrees({ 0.0f, 0.0f, 0.0f });
	mPrismRenderers.last()
		->setAngle(60.0f)
		.setMaterial(PrPrismMaterial::NormGlass)
		.enable()
		.setPosition({ 0.0f, 0.95f, -2.0f })
		.setRotationDegrees({ 0.0f, 0.0f, 90.0f });
}
void PrScene::recalculateBeams()
{
	mBeamsRenderer->recalculateMesh(mPrismRenderers, mScreenRenderer, mProjCamera);
}

void PrScene::onMouseDrag(const QVector2D& dragDeltaCoords)
{
	mMainCamera->rotate(dragDeltaCoords);
}
void PrScene::onResize(float width, float height)
{
	mMainCamera->setWidth(width);
	mProjCamera->setWidth(width);
	mMainCamera->setHeight(height);
	mProjCamera->setHeight(height);
}
