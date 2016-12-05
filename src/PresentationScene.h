#ifndef PRESENTATIONSCENE_H
#define PRESENTATIONSCENE_H

#include <QtMath>
#include <QVector>

#include "SceneMeshes.h"
#include "widgets/SceneWidgetAdvanced.h"
#include "PresentationPhysics.h"
#include "PresentationGeometry.h"
#include <QChart>
#include <QChartView>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

/*!
 * The prism material enum.
 */
enum class PrPrismMaterial
{
    NormAir,
    AnomNatrium,
    AnomCyanine,
};

/*!
 * The prism renderer class.
 */
class PrPrismRenderer final : public ScTransparentMeshRenderer, public OpRefractiveObject
{
public:
    float angle() const { return mAngle; }
    PrPrismRenderer& setAngle(float angle)
    {
        mAngle = angle;
		return *this;
    }

    PrPrismMaterial material() const { return mMaterial; }
    PrPrismRenderer& setMaterial(PrPrismMaterial material)
    {
        mMaterial = material;
		return *this;
    }

    ScMeshRenderer& prismHolderBase() { return mPrismHolderBase; }
    const ScMeshRenderer& prismHolderBase() const { return mPrismHolderBase; }
    PrPrismRenderer& setPrismHolderBase(ScMeshRenderer const& prismHolderBase)
    {
        mPrismHolderBase = prismHolderBase;
        return *this;
    }

    ScMeshRenderer& prismHolderLeg() { return mPrismHolderLeg; }
    const ScMeshRenderer& prismHolderLeg() const { return mPrismHolderLeg; }
    PrPrismRenderer& setPrismHolderLeg(ScMeshRenderer const& prismHolderLeg)
    {
        mPrismHolderLeg = prismHolderLeg;
        return *this;
    }

    ScMeshRenderer& prismHolderGimbal() { return mPrismHolderGimbal; }
    const ScMeshRenderer& prismHolderGimbal() const { return mPrismHolderGimbal; }
    PrPrismRenderer& setPrismHolderGimbal(ScMeshRenderer const& prismHolderGimbal)
    {
        mPrismHolderGimbal = prismHolderGimbal;
        return *this;
    }

    void render(const ScBasicCamera& camera) override final
    {
        //! @todo compute this values.
        static const float legHeight = 1.5f;
        static const float gimbalHeight = 0.2f;

        setOffset({ 0.0f, 0.2f / 3 - 0.1f, 0.0f, });
        setScale({ 1.0f, 1.0f, tanf(qDegreesToRadians(angle() / 2.0f)) });

        prismHolderBase()
                .setEnabled(enabled())
                .setPosition({ position().x(), 0.0f, position().z() });
        prismHolderLeg()
                .setEnabled(enabled())
                .setPosition({ position().x(), position().y() - legHeight - gimbalHeight, position().z() });
        prismHolderGimbal()
                .setEnabled(enabled())
                .setPosition(position())
                .setRotation({ rotation().x(), rotation().y(), rotation().z() });

        ScTransparentMeshRenderer::render(camera);
		prismHolderBase().render(camera);
        prismHolderLeg().render(camera);
        prismHolderGimbal().render(camera);
    }

	void refractBeam(OpBeam& beam, bool) const override
	{
		if (!mPlanesSynced)
		{
			syncRefractivePlanes();
			mPlanesSynced = true;
		}

        mFirstPlane.refractBeam(beam, true);
        mSecondPlane.refractBeam(beam, false);
    }

public:
	void syncRefractivePlanes() const
	{
		QMatrix4x4 model = modelMatrix();

		PhComplexIndexFunction_p refractiveIndex = mFirstPlane.refractiveIndex();
		if (refractiveIndex == nullptr)
		{
			refractiveIndex = PhComplexIndexFunction::create();
			if (material() == PrPrismMaterial::NormAir)
			{
				refractiveIndex->setRealPart(PhParabolicIndexFunction::create());
				refractiveIndex->setImaginaryPart(PhIndexFunction::create());
			}
			else
			{
				refractiveIndex->setImaginaryPart(PhGaussianIndexFunction::create());
				refractiveIndex->computeRealPartKramersKronig(0.38, 0.78);
			}
		}
		else
		{
			if (material() != PrPrismMaterial::NormAir)
			{
				refractiveIndex->computeRealPartKramersKronig(0.38, 0.78);
			}
		}
		
		//! @todo Get this values out of the mesh.
		//! @todo Cache the planes somehow.
		QVector3D fp1 = model * QVector3D(-0.1f, -0.1f, +0.0f);
		QVector3D fp2 = model * QVector3D(+0.1f, -0.1f, +0.0f);
		QVector3D fp3 = model * QVector3D(+0.1f, +0.1f, +0.2f);
        mFirstPlane
                .setMinBound(fp1)
                .setMaxBound(fp3)
				.setRefractiveIndex(refractiveIndex)
    			.setNormal(-QVector3D::normal(fp1, fp2, fp3));

		QVector3D sp1 = model * QVector3D(-0.1f, -0.1f, +0.0f);
		QVector3D sp2 = model * QVector3D(+0.1f, -0.1f, +0.0f);
		QVector3D sp3 = model * QVector3D(-0.1f, +0.1f, -0.2f);
		mSecondPlane
				.setMinBound(sp2)
				.setMaxBound(sp3)
				.setRefractiveIndex(refractiveIndex)
				.setNormal(-QVector3D::normal(sp1, sp2, sp3));
	}

private:
    float mAngle = 30.0f;
    PrPrismMaterial mMaterial = PrPrismMaterial::NormAir;
    ScMeshRenderer mPrismHolderBase, mPrismHolderLeg, mPrismHolderGimbal;
public:
	mutable bool mPlanesSynced = false;
	mutable OpRefractivePlane mFirstPlane;
	mutable OpRefractivePlane mSecondPlane;
};

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

/*!
 * @param The light beam renderer class.
 */
class PrBeamsRenderer final : public ScMeshRenderer
{
public:
    template<int N>
    void recalculate(PrPrismRenderer (&prismRenderers)[N])
    {
		OpBeamCone beamCone;
		beamCone
				.setPartitioning(1000)
				.setStartPosition(position())
				.setStartDirection(QVector3D(0.0f, 0.0f, -1.0f));

		for (PrPrismRenderer& prismRenderer : prismRenderers)
		{
			if (!prismRenderer.enabled())
			{
				continue;
			}
			beamCone.collide(prismRenderer);
		}

		PhComplexIndexFunction_p index2 = PhComplexIndexFunction::create();
		index2->setRealPart(PhIndexFunction::create());
		index2->setImaginaryPart(PhIndexFunction::create());
		OpRefractivePlane screenPlane;
		screenPlane
				.setOpaque(true)
				.setMinBound({ -1.77f, 0.485f, -3.49f })
				.setMaxBound({ 1.77f, 2.07f, -3.49f })
				.setNormal({ 0.0f, 0.0f, -1.0f })
				.setRefractiveIndex(index2);
		beamCone.collide(screenPlane);

		QVector<ScVertexData> vertices;
		PresentationGeometry::generateBeamMesh(beamCone, vertices);
		mesh()->setVertices(vertices.data(), vertices.size());

		QVector<ScVertexData> projVertices;
        PresentationGeometry::generateBeamProjMesh(beamCone, screenPlane.normal(), projVertices);
		mProjectionRenderer.setMesh(ScEditableMesh_p(new ScEditableMesh()));
		mProjectionRenderer.mesh()->setVertices(projVertices.data(), projVertices.size());
    }

	QMatrix4x4 modelMatrix() const override
    {
		QMatrix4x4 model;
		model.setToIdentity();
		return model;
    }

    void render(const ScBasicCamera& camera) override final
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        ScMeshRenderer::render(camera);
        mProjectionRenderer.render(camera);
        glDisable(GL_BLEND);
    }

private:
    static const int sRaysCount = 1000u;
public:
    ScMeshRenderer mProjectionRenderer;
};

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

/*!
 * @param The presentation scene class.
 */
class PrScene final : public ScScene
{
public:
	QtCharts::QChartView* mCharts;
    ScOrbitalCamera mMainCamera;
    ScProjectionCamera mProjCamera;

    ScMeshRenderer mRoomRenderer;
    ScMeshRenderer mScreenRenderer;
    PrPrismRenderer mPrismRenderers[2];

    QSharedPointer<ScEditableMesh> mRaysMesh, mRaysProjMesh;
    PrBeamsRenderer mRaysRenderer;

public:
    PrScene(QtCharts::QChartView* chart): mCharts(chart)
    {
        auto unlitTexturedShaderProgram = scLoadShaderProgram(":/shaders/vertex.glsl", ":/shaders/fragUnlitTextured.glsl");
        auto unlitColoredShaderProgram = scLoadShaderProgram(":/shaders/vertex.glsl", ":/shaders/fragUnlit.glsl");
		auto litColoredShaderProgram = scLoadShaderProgram(":/shaders/vertex.glsl", ":/shaders/fragLit.glsl");
		auto litRefrShaderProgram = scLoadShaderProgram(":/shaders/vertex.glsl", ":/shaders/fragRefract.glsl");

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        mMainCamera
                .setRotationMinBound({ -24.0f, -145.0f })
                .setRotationMaxBound({ +15.0f, +145.0f })
                .setRotationCenter({ 0.0f, 1.0f, -2.0f })
                .setRotationOrbit({0.0f, 0.0f, 1.8f})
                .rotate({0.0f, 45.0f})
                .setClearColor(QColor(255, 255, 255, 255));
        mProjCamera
                .setClearColor(QColor(0, 0, 0, 255))
                .setViewport(QRectF(1.0f - 0.35f - 0.05f, 0.05f, 0.35f, 0.35f));

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        mRoomRenderer
                .setMesh(QSharedPointer<ScEditableMesh>(new ScEditableMesh(roomVertices, _countof(roomVertices))))
                .setShaderProgram(unlitTexturedShaderProgram)
                .setDiffuseTexture(scLoadTexture(":/gfx/roomLightMap.png"))
				.enable()
                .setPosition({0.0f, 0.0f, -2.0f})
                .setScale({-1.0f, 1.0f, 1.0f});
        mScreenRenderer
                .setMesh(QSharedPointer<ScEditableMesh>(new ScEditableMesh(screenVertices, _countof(screenVertices))))
                .setShaderProgram(unlitTexturedShaderProgram)
                .setDiffuseTexture(scLoadTexture(":/gfx/screenLightMap.png"))
				.enable()
                .setPosition({0.0f, 0.0f, -2.0f});

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        QSharedPointer<ScEditableMesh> prismMesh(new ScEditableMesh(prismVertices, _countof(prismVertices), true));
        ScMeshRenderer prismHolderBaseRenderer;
        prismHolderBaseRenderer
                .enable()
                .setMesh(QSharedPointer<ScEditableMesh>(new ScEditableMesh(prismHolderBaseVertices, _countof(prismHolderBaseVertices))))
                .setShaderProgram(litColoredShaderProgram);
        ScMeshRenderer prismHolderLegRenderer;
        prismHolderLegRenderer
                .enable()
                .setMesh(QSharedPointer<ScEditableMesh>(new ScEditableMesh(prismHolderLegVertices, _countof(prismHolderLegVertices))))
                .setShaderProgram(litColoredShaderProgram);
        ScMeshRenderer prismHolderGimbalRenderer;
        prismHolderGimbalRenderer
                .enable()
                .setMesh(QSharedPointer<ScEditableMesh>(new ScEditableMesh(prismHolderGimbalVertices, _countof(prismHolderGimbalVertices))))
                .setShaderProgram(litColoredShaderProgram);
        for (PrPrismRenderer& prismRenderer : mPrismRenderers)
        {
            prismRenderer
                    .setPrismHolderBase(prismHolderBaseRenderer)
                    .setPrismHolderLeg(prismHolderLegRenderer)
                    .setPrismHolderGimbal(prismHolderGimbalRenderer)
                    .setMesh(prismMesh)
                    .setShaderProgram(litRefrShaderProgram)
					.setDiffuseTexture(scLoadCubemap(":/gfx/roomCubemap", ".png"))
                    .enable();
			prismRenderer.render(mMainCamera);
        }
		setTwoPrismLayout();
		mPrismRenderers[0].render(mMainCamera);
		mPrismRenderers[1].render(mMainCamera);

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        mRaysMesh = QSharedPointer<ScEditableMesh>(new ScEditableMesh());
        mRaysProjMesh = QSharedPointer<ScEditableMesh>(new ScEditableMesh());
		mRaysRenderer
				.setMesh(mRaysMesh)
				.setShaderProgram(unlitColoredShaderProgram)
				.enable()
				.setPosition({ 0.0f, 0.72f, 2.0f });
		mRaysRenderer.mProjectionRenderer
    			.setShaderProgram(unlitTexturedShaderProgram)
                .setDiffuseTexture(scLoadTexture(":/gfx/color_mask.png"));

        recalculateRays();
    }

    void setOnePrismLayout()
    {
		mPrismRenderers[0]
			.setAngle(60.0f)
			.setMaterial(PrPrismMaterial::NormAir)
			.enable()
			.setPosition({ 0.0f, 0.75f, -1.3f })
			.setRotation({ 0.0f, 0.0f, 0.0f });
        mPrismRenderers[1]
                .disable();
    }
    void setTwoPrismLayout()
    {
        mPrismRenderers[0]
                .setAngle(60.0f)
                .setMaterial(PrPrismMaterial::NormAir)
                .enable()
                .setPosition({ 0.0f, 0.75f, -1.4f })
                .setRotation({ 0.0f, 0.0f, 0.0f });
        mPrismRenderers[1]
                .setAngle(60.0f)
                .setMaterial(PrPrismMaterial::AnomCyanine)
				.enable()
                .setPosition({ 0.0f, 0.95f, -2.0f })
                .setRotation({ 0.0f, 0.0f, 90.0f });

		mPrismRenderers[0].syncRefractivePlanes();
		mPrismRenderers[1].syncRefractivePlanes();

		/*QtCharts::QChart* chart = new QtCharts::QChart();
		chart->addSeries(mPrismRenderers[1].mFirstPlane.refractiveIndex()->realPart()->visualize());
		chart->addSeries(mPrismRenderers[1].mFirstPlane.refractiveIndex()->imaginaryPart()->visualize());
		chart->createDefaultAxes();
		mCharts->setChart(chart);*/
	}

    void onMouseDrag(const QVector2D& dragDeltaCoords) override final
    {
        mMainCamera.rotate(dragDeltaCoords);
    }
	void onResize(float width, float height) override final
	{
		mMainCamera.setWidth(width);
		mMainCamera.setHeight(height);
		mProjCamera.setWidth(width);
		mProjCamera.setHeight(height);
    }

    void render() override final
    {
        mMainCamera.beginScene();
        {
            mRoomRenderer.render(mMainCamera);
            mScreenRenderer.render(mMainCamera);
            
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            //mRaysProjRenderer.render(mMainCamera);
            mRaysRenderer.render(mMainCamera);
            glDisable(GL_BLEND);

			for (PrPrismRenderer& prismRenderer : mPrismRenderers)
			{
				prismRenderer.render(mMainCamera);
			}
        }
        mMainCamera.endScene();

        mProjCamera.beginScene();
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            mRaysRenderer.render(mProjCamera);
            glDisable(GL_BLEND);
        }
        mProjCamera.endScene();
    }

    void recalculateRays()
    {
		setTwoPrismLayout();
		mPrismRenderers[1].mPlanesSynced = false;
		mPrismRenderers[1].syncRefractivePlanes();
		mRaysRenderer.recalculate(mPrismRenderers);
    }
};

#endif // PRESENTATIONSCENE_H
