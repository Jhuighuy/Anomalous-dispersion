/****************************************************************************
**
** Copyright (C) 2016 Plaxin Gleb, Butakov Oleg.
** License: MIT.
**
****************************************************************************/

#include "SceneWidget.h"
#include "forms/SceneWindow.h"

#include <QSharedPointer>
#include <QMouseEvent>
#include <QtMath>

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLShader>
#include <QOpenGLTexture>

#ifndef _countof
#define _countof(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

QMatrix4x4 ScTransform::modelMatrix() const
{
	QMatrix4x4 model;
	model.setToIdentity();
	model.translate(position());
	model.rotate(rotation());
	model.scale(scale());
	return model;
}
QMatrix3x3 ScTransform::normalMatrix() const
{
	QMatrix4x4 model = modelMatrix();
	QMatrix3x3 normal = model.normalMatrix();
	return normal;
}

// ----------------------

QMatrix4x4 ScOffsetTransform::modelMatrix() const
{
	QMatrix4x4 model;
	model.setToIdentity();
	model.translate(position());
	model.rotate(rotation());
	model.translate(offset());
	model.scale(scale());
	return model;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

QMatrix4x4 ScBasicCamera::viewMatrix() const
{
    QMatrix4x4 view;
    view.setToIdentity();
    return view;
}
QMatrix4x4 ScBasicCamera::projectionMatrix() const
{
    QMatrix4x4 projection;
    projection.perspective(60.0f, width() / height(), 0.01f, 100.0f);
    return projection;
}

void ScBasicCamera::beginScene() const
{
    float x = width() * viewport().x(), y = height() * viewport().y();
    float w = width() * viewport().width(), h = height() * viewport().height();

    glScissor(x, y, w, h);
    glViewport(x, y, w, h);
    glClearColor(clearColor().x(), clearColor().y(), clearColor().z(), clearColor().w());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void ScBasicCamera::endScene() const
{
}

// ----------------------

QMatrix4x4 ScProjectionCamera::viewMatrix() const
{
    QMatrix4x4 world;
    world.setToIdentity();
    world.translate(position());
    world.rotate(rotation());
    return world.inverted();
}
QMatrix4x4 ScProjectionCamera::projectionMatrix() const
{
    float left = -0.5f * size();
    float bottom = left * width() / height();

    QMatrix4x4 projection;
    projection.ortho(left, -left, bottom, -bottom, 0.01f, 100.0f);
    return projection;
}

void ScProjectionCamera::beginScene() const
{
    float a = height() / width();
	float x = (a * viewport().x() + (1.0f - a)) * width(), y = height() * viewport().y();
	float w = a * width() * viewport().width(), h = height() * viewport().height();

    glScissor(x, y, w, h);
    glViewport(x, y, w, h);
    glClearColor(clearColor().x(), clearColor().y(), clearColor().z(), clearColor().w());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void ScProjectionCamera::endScene() const
{
}

// ----------------------

ScOrbitalCamera& ScOrbitalCamera::rotate(const QVector2D& deltaRotation)
{
    float newX = fmod(rotation().x() + deltaRotation.x(), 360.0f);
    float newY = fmod(rotation().y() + deltaRotation.y(), 360.0f);

    mRotation.setX(qBound(rotationMinBound().x(), newX, rotationMaxBound().x()));
    mRotation.setY(qBound(rotationMinBound().y(), newY, rotationMaxBound().y()));
    return *this;
}

QMatrix4x4 ScOrbitalCamera::viewMatrix() const
{
    QMatrix4x4 cameraRotation;
    cameraRotation.setToIdentity();
    cameraRotation.rotate(QQuaternion::fromEulerAngles(rotation().x(), rotation().y(), 0.0f));

    QMatrix4x4 view;
    view.setToIdentity();
    view.lookAt(cameraRotation * rotationOrbit() + rotationCenter(), rotationCenter(),
                cameraRotation * QVector3D(0.0f, 1.0f, 0.0f));
    return view;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

QOpenGLShaderProgram_p scLoadShaderProgram(const char* vertexShaderPath, const char* pixelShaderPath)
{
    QOpenGLShaderProgram_p shaderProgram(new QOpenGLShaderProgram());

    if (!shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, vertexShaderPath))
    {
        qDebug() << "Failed to load a vertex shader " << vertexShaderPath;
        abort();
    }
    if (!shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, pixelShaderPath))
    {
        qDebug() << "Failed to load a pixel shader " << pixelShaderPath;
        abort();
    }
    if (!shaderProgram->link())
    {
        qDebug() << "Failed to load link a shader program " << pixelShaderPath << " + " << pixelShaderPath;
        abort();
    }
    return shaderProgram;
}
QOpenGLTexture_p scLoadTexture(const char* texturePath)
{
    //! @todo Validate the texture path.
    QOpenGLTexture_p texture(new QOpenGLTexture(QImage(texturePath)));
    return texture;
}

// ----------------------

/*!
 * Loads a vertices of the editable mesh.
 *
 * @param vertices The actual array of the vertex data.
 * @param count The amount of the vertices.
 * @param cacheVertices Do cache vertices inside the mesh object?
 * @param computeAABB Do compute the bounding box for this mesh?
 */
ScEditableMesh& ScEditableMesh::setVertices(const ScVertexData* vertices, int count, bool cacheVertices, bool computeAABB)
{
	// Caching the vertices.
	if (cacheVertices)
	{
		resize(count);
		qCopy(vertices, vertices + count, begin());
	}
	else
	{
		clear();
	}

	// Computing the AABB.
	if (computeAABB && count != 0)
	{
		mMinBound = vertices[0].vertexCoord;
		mMaxBound = vertices[0].vertexCoord;
		for (int i = 1; i < count; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				mMinBound[j] = qMin(mMinBound[j], vertices[i].vertexCoord[j]);
				mMaxBound[j] = qMax(mMaxBound[j], vertices[i].vertexCoord[j]);
			}
		}
	}
	else
	{
		mMinBound = QVector3D();
		mMaxBound = QVector3D();
	}

	// Loading the mesh.
    int newSize = count * sizeof(*vertices);
    if (!mVertexBuffer.isCreated())
    {
		mVertexBuffer.create();
		mVertexBuffer.bind();
		mVertexBuffer.allocate(vertices, newSize);
    }
    else
    {
        int existingSize = mVerticesCount * sizeof(*vertices);

		mVertexBuffer.bind();
        if (newSize < existingSize)
        {
			mVertexBuffer.write(0, vertices, newSize);
        }
        else
        {
			mVertexBuffer.allocate(vertices, newSize);
        }
    }
	
	mVerticesCount = count;
    return *this;
}

/*!
 * Renders this mesh with the specified shader program.
 * @param shaderProgram Program to be used while rendering.
 */
void ScEditableMesh::render(QOpenGLShaderProgram& shaderProgram, int firstVertex, int count)
{
    if (!mVertexBuffer.isCreated() || count == 0)
    {
        return;
    }

    shaderProgram.bind();
	mVertexBuffer.bind();

    int vertexLocation = shaderProgram.attributeLocation("in_VertexCoordMS");
    if (vertexLocation != -1)
    {
        shaderProgram.enableAttributeArray(vertexLocation);
        shaderProgram.setAttributeBuffer(vertexLocation, GL_FLOAT, offsetof(ScVertexData, vertexCoord),
                                         sizeof(ScVertexData::vertexCoord) / sizeof(ScVertexData::vertexCoord.x()),
                                         sizeof(ScVertexData));
    }

    int texCoordLocation = shaderProgram.attributeLocation("in_TexCoord");
    if (texCoordLocation != -1)
    {
        shaderProgram.enableAttributeArray(texCoordLocation);
        shaderProgram.setAttributeBuffer(texCoordLocation, GL_FLOAT, offsetof(ScVertexData, textureCord),
                                         sizeof(ScVertexData::textureCord) / sizeof(ScVertexData::textureCord.x()),
                                         sizeof(ScVertexData));
    }

    int normalLocation = shaderProgram.attributeLocation("in_NormalMS");
    if (normalLocation != -1)
    {
        shaderProgram.enableAttributeArray(normalLocation);
        shaderProgram.setAttributeBuffer(normalLocation, GL_FLOAT, offsetof(ScVertexData, normal),
                                         sizeof(ScVertexData::normal) / sizeof(ScVertexData::normal.x()),
                                         sizeof(ScVertexData));
    }

    int colorLocation = shaderProgram.attributeLocation("in_Color");
    if (colorLocation != -1)
    {
        shaderProgram.enableAttributeArray(colorLocation);
        shaderProgram.setAttributeBuffer(colorLocation, GL_FLOAT, offsetof(ScVertexData, color),
                                         sizeof(ScVertexData::color) / sizeof(ScVertexData::color.x()),
                                         sizeof(ScVertexData));
    }

    glDrawArrays(GL_TRIANGLES, firstVertex, count);
}

// ----------------------

/*!
 * Renders this object with the specified camera.
 * @param camera The camera to be used while rendering.
 */
void ScMeshRenderer::render(const ScBasicCamera& camera)
{
	if (!enabled())
	{
		return;
	}

	beginRender(camera);
    mesh()->render(*shaderProgram());
	endRender();
}

void ScMeshRenderer::beginRender(const ScBasicCamera& camera) const
{
	Q_ASSERT(mesh() != nullptr);
	Q_ASSERT(shaderProgram() != nullptr);

	if (diffuseTexture() != nullptr)
	{
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_TEXTURE_2D);
		diffuseTexture()->bind(0);
	}

	QMatrix4x4 view = camera.viewMatrix();
	QMatrix4x4 projection = camera.projectionMatrix();
	QVector3D cameraPosition(view.inverted().column(3));

	shaderProgram()->bind();
	shaderProgram()->setUniformValue("un_DiffuseTexture", 0);
#if USE_ADVANCED_RENDERING
	shaderProgram()->setUniformValue("un_DisplacementTexture", 1);
	shaderProgram()->setUniformValue("un_NormalTexture", 2);
#endif
	shaderProgram()->setUniformValue("un_ModelMatrix", modelMatrix());
	shaderProgram()->setUniformValue("un_NormalMatrix", normalMatrix());
	shaderProgram()->setUniformValue("un_ViewMatrix", view);
	shaderProgram()->setUniformValue("un_ProjectionMatrix", projection);
	shaderProgram()->setUniformValue("un_CameraPositionWS", cameraPosition);
}
void ScMeshRenderer::endRender() const
{
	shaderProgram()->release();
	if (diffuseTexture() != nullptr)
	{
		diffuseTexture()->release(0);
		glDisable(GL_TEXTURE_2D);
	}
}

// ----------------------

/*!
 * Renders this transparent object with the specified camera.
 * @param camera The camera to be used while rendering.
 */
void ScTransparentMeshRenderer::render(const ScBasicCamera& camera)
{
	if (!enabled())
	{
		return;
	}

	Q_ASSERT(mesh()->size() != 0);

    QMatrix4x4 modelView = camera.viewMatrix() * modelMatrix();
	
	struct ScTraingleInfo
	{
		int vertexIndex;
        float distance;
	};

	// Building the mesh triangle info.
	QVector<ScTraingleInfo> trianglesInfo(mesh()->size() / 3);
#pragma omp parallel for
	for (int i = 0; i < trianglesInfo.size(); ++i)
	{
		QVector3D trianglePositionVS;
		for (int j = 0; j < 3; ++j)
		{
			const ScVertexData& vertex = mesh()->at(3 * i + j);
			QVector3D vertexPositionVS = modelView * vertex.vertexCoord * vertex.color.w();
			trianglePositionVS += vertexPositionVS;
		}
		trianglesInfo[i] = { 3 * i, trianglePositionVS.length() };
	}

	// Sorting the triangles further to the nearest.
    qSort(trianglesInfo.begin(), trianglesInfo.end(), [](const ScTraingleInfo& a,
                                                         const ScTraingleInfo& b)
    {
        return a.distance > b.distance;
    });

    // Rendering the sorted triangles.
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	beginRender(camera);
	for (int i = 0; i < trianglesInfo.size(); ++i)
	{
		int triangleIndex = trianglesInfo[i].vertexIndex;
		mesh()->render(*shaderProgram(), triangleIndex, 3);
	}
	endRender();

	glDisable(GL_BLEND);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

void ScOpenGLWidget::mousePressEvent(QMouseEvent* mouseEvent)
{
	mMouseDragBegan = true;
	mMousePressPosition = mouseEvent->localPos();
}
void ScOpenGLWidget::mouseMoveEvent(QMouseEvent* mouseEvent)
{
	QPointF deltaMouseCoords = mouseEvent->localPos() - mMousePressPosition;
	mMousePressPosition = mouseEvent->localPos();
    mScene->onMouseDrag(QVector2D(deltaMouseCoords.y() / -40.0f, deltaMouseCoords.x() / -40.0f));
}
void ScOpenGLWidget::mouseReleaseEvent(QMouseEvent*)
{
	mMouseDragBegan = false;
}
void ScOpenGLWidget::wheelEvent(QWheelEvent* /*wheelEvent*/)
{
	//wheelEvent->
}

void ScOpenGLWidget::initializeGL()
{
    QSurfaceFormat format;
    format.setDepthBufferSize(32);
    format.setMajorVersion(2);
    format.setMajorVersion(1);
    format.setSamples(4);
	format.setAlphaBufferSize(0);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);

    context()->setFormat(format);
    context()->functions()->initializeOpenGLFunctions();

	Q_ASSERT(scene() != nullptr);
	scene()->init();
	gScene->setSecondPrismEnabled(false);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
}

void ScOpenGLWidget::resizeGL(int w, int h)
{
	Q_ASSERT(scene() != nullptr);
#ifndef __APPLE__
	scene()->onResize(static_cast<float>(w), static_cast<float>(h));
#else
    scene()->onResize(static_cast<float>(2 * w), static_cast<float>(2 * h));
#endif
}

void ScOpenGLWidget::paintGL()
{
	Q_ASSERT(scene() != nullptr);
	scene()->render();
    update();
}
