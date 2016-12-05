#include "SceneWidget.h"
#include "SceneMeshes.h"
#include "PresentationScene.h"

#include <QSharedPointer>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLShader>
#include <QOpenGLTexture>

#include <QtMath>
#include <QMouseEvent>
#include <QWheelEvent>

#ifndef _countof
#define _countof(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

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
    glClearColor(clearColor().redF(), clearColor().greenF(), clearColor().blueF(), clearColor().alphaF());
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

/*
void ScProjectionCamera::beginScene() const
{
    float a = height() / width();
    float x = a * width() * viewport().x(), y = height() * viewport().y();
    float w = a * width() * viewport().width(), h = height() * viewport().height();

    glScissor(x, y, w, h);
    glViewport(x, y, w, h);
    glClearColor(clearColor().redF(), clearColor().greenF(), clearColor().blueF(), clearColor().alphaF());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void ScProjectionCamera::endScene() const
{
}
*/

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

QOpenGLShaderProgram_p pLoadShaderProgram(const char* vertexShaderPath, const char* pixelShaderPath)
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
QOpenGLTexture_p pLoadTexture(const char* texturePath)
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
ScEditableMesh& ScEditableMesh::load(const ScVertexData* vertices, int count, bool cacheVertices, bool computeAABB)
{
    int size = count * sizeof(*vertices);
    mVerticesCount = count;
    if (!mVertexBuffer.isCreated())
    {
        mVertexBuffer.create();
        mVertexBuffer.bind();
        mVertexBuffer.allocate(vertices, size);
    }
    else
    {
        int existingSize = mVertexBuffer.size();

        mVertexBuffer.bind();
        if (size < existingSize)
        {
            mVertexBuffer.write(0, vertices, size);
        }
        else
        {
            mVertexBuffer.allocate(vertices, size);
        }
    }
    return *this;
}

/*!
 * Renders this mesh with the specified shader program.
 * @param shaderProgram Program to be used while rendering.
 */
void ScEditableMesh::render(QOpenGLShaderProgram& shaderProgram)
{
    if (!mVertexBuffer.isCreated() || mVerticesCount == 0)
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

    glDrawArrays(GL_TRIANGLES, 0, mVerticesCount);
}

// ----------------------

QMatrix4x4 ScMeshRenderer::modelMatrix() const
{
    QMatrix4x4 model;
    model.setToIdentity();
    model.translate(position());
    model.rotate(rotation());
    model.translate(offset());
    model.scale(scale());
    return model;
}
QMatrix3x3 ScMeshRenderer::normalMatrix() const
{
    QMatrix4x4 model = modelMatrix();
    QMatrix3x3 normal(model.normalMatrix());
    return normal;
}

/*!
 * Renders this object with the specified camera.
 * @param camera The camera to be used while rendering.
 */
void ScMeshRenderer::render(const ScBasicCamera& camera)
{
    Q_ASSERT(mesh() != nullptr);
    Q_ASSERT(shaderProgram() != nullptr);

    if (!enabled())
    {
        return;
    }

    if (texture() != nullptr)
    {
        glEnable(GL_TEXTURE_2D);
        texture()->bind(0);
    }

    shaderProgram()->bind();
    shaderProgram()->setUniformValue("un_Texture", 0);
    shaderProgram()->setUniformValue("un_ModelMatrix", modelMatrix());
    shaderProgram()->setUniformValue("un_NormalMatrix", normalMatrix());
    shaderProgram()->setUniformValue("un_ViewProjectionMatrix", camera.projectionMatrix() * camera.viewMatrix());
    shaderProgram()->release();
    mesh()->render(*shaderProgram());

    if (texture() != nullptr)
    {
        texture()->release(0);
        glDisable(GL_TEXTURE_2D);
    }
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
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);

    context()->setFormat(format);
    context()->functions()->initializeOpenGLFunctions();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    //glEnable(GL_CULL_FACE);

    mScene = new PrScene(mChart);
}

void ScOpenGLWidget::resizeGL(int w, int h)
{
	mScene->onResize(static_cast<float>(w), static_cast<float>(h));
}

void ScOpenGLWidget::paintGL()
{
    mScene->render();
    update();
}
