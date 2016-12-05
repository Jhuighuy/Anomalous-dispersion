/****************************************************************************
**
** Copyright (C) 2016 Plaxin Gleb, Butakov Oleg.
** Contact: kandidov_i_chickishev@ublidky.com
** License: MIT.
**
****************************************************************************/

#ifndef PSCENEWIDGET_H
#define PSCENEWIDGET_H

#include <QSharedPointer>
#include <QRectF>
#include <QColor>
#include <QOpenGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QChartView>

#include <cmath>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#define DEFINE_SHARED_PTR(Class) \
	typedef QSharedPointer<class Class> Class##_p; \
	typedef QSharedPointer<const Class> Class##_cp;
#define DEFINE_CREATE_FUNC(Class) \
	public: \
		template<typename... T>\
		static Class##_p create(T&&... args) { return Class##_p(new Class(qForward<T>(args)...)); }\
	private:

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

/*!
 * The basic camera class.
 */
class ScBasicCamera
{
public:
    virtual ~ScBasicCamera()
	{
	}

	float width() const { return mWidth; }
    ScBasicCamera& setWidth(float width)
    {
        mWidth = width;
        return *this;
    }

    float height() const { return mHeight; }
    ScBasicCamera& setHeight(float height)
    {
        mHeight = height;
        return *this;
    }

    const QRectF& viewport() const { return mViewport; }
    ScBasicCamera& setViewport(const QRectF& viewport)
    {
        mViewport = viewport;
        return *this;
    }

    const QColor& clearColor() const { return mClearColor; }
    ScBasicCamera& setClearColor(const QColor& clearColor)
    {
        mClearColor = clearColor;
        return *this;
    }

    virtual QMatrix4x4 viewMatrix() const
    {
        QMatrix4x4 view;
        view.setToIdentity();
        return view;
    }
    virtual QMatrix4x4 projectionMatrix() const
    {
        QMatrix4x4 projection;
        projection.perspective(60.0f, width() / height(), 0.01f, 100.0f);
        return projection;
    }

    virtual void beginScene() const
    {
        float x = width() * viewport().x(), y = height() * viewport().y();
        float w = width() * viewport().width(), h = height() * viewport().height();

        glScissor(x, y, w, h);
        glViewport(x, y, w, h);
		glClearColor(clearColor().redF(), clearColor().greenF(), clearColor().blueF(), clearColor().alphaF());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    virtual void endScene() const
    {
    }

private:
    float mWidth = 800.0f, mHeight = 600.0f;
    QRectF mViewport = {0.0f, 0.0f, 1.0f, 1.0f};
    QColor mClearColor;
};

typedef QSharedPointer<ScBasicCamera> ScBasicCamera_p;
typedef QSharedPointer<const ScBasicCamera> ScBasicCamera_cp;

/*!
 * The projection camera class.
 */
class ScProjectionCamera final : public ScBasicCamera
{
public:
    const QVector3D& position() const { return mPosition; }
    ScProjectionCamera& setPosition(const QVector3D& position)
    {
        mPosition = position;
        return *this;
    }

    const QQuaternion& rotation() const { return mRotation; }
    ScProjectionCamera& setScale(const QQuaternion& rotation)
    {
        mRotation = rotation;
        return *this;
    }

    float size() const { return mSize; }
    ScProjectionCamera& setSize(float size)
    {
        mSize = size;
        return *this;
    }

	QMatrix4x4 viewMatrix() const override
    {
        QMatrix4x4 world;
        world.setToIdentity();
        world.translate(position());
        world.rotate(rotation());
        return world.inverted();
    }
	QMatrix4x4 projectionMatrix() const override
    {
        float left = -0.5f * size();
        float bottom = left * width() / height();

        QMatrix4x4 projection;
        projection.ortho(left, -left, bottom, -bottom, 0.01f, 100.0f);
        return projection;
    }

    /*virtual void beginScene() const
    {
        float a = height() / width();
        float x = a * width() * viewport().x(), y = height() * viewport().y();
        float w = a * width() * viewport().width(), h = height() * viewport().height();

        glScissor(x, y, w, h);
        glViewport(x, y, w, h);
        glClearColor(clearColor().redF(), clearColor().greenF(), clearColor().blueF(), clearColor().alphaF());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }*/

private:
    float mSize = 5.0f;
    QVector3D mPosition;
    QQuaternion mRotation;
};

typedef QSharedPointer<ScProjectionCamera> ScProjectionCamera_p;
typedef QSharedPointer<const ScProjectionCamera> ScProjectionCamera_cp;

/*!
 * The orbital camera class.
 */
class ScOrbitalCamera final : public ScBasicCamera
{
public:
    const QVector2D& rotation() const { return mRotation; }
    ScOrbitalCamera& rotate(const QVector2D& deltaRotation)
    {
		float newX = fmod(rotation().x() + deltaRotation.x(), 360.0f);
		float newY = fmod(rotation().y() + deltaRotation.y(), 360.0f);

        mRotation.setX(qBound(rotationMinBound().x(), newX, rotationMaxBound().x()));
        mRotation.setY(qBound(rotationMinBound().y(), newY, rotationMaxBound().y()));
        return *this;
    }

    const QVector2D& rotationMinBound() const { return mRotationMinBound; }
    ScOrbitalCamera& setRotationMinBound(const QVector2D& rotationMinBound)
    {
        mRotationMinBound = rotationMinBound;
        return *this;
    }
    float rotationMinBoundX() const { return rotationMinBound().x(); }
    ScOrbitalCamera& setRotationMinBoundX(float x)
    {
        setRotationMinBound({ x, rotationMinBoundY() });
        return *this;
    }
    float rotationMinBoundY() const { return rotationMinBound().y(); }
    ScOrbitalCamera& setRotationMinBoundY(float y)
    {
        setRotationMinBound({ rotationMinBoundX(), y });
        return *this;
    }

    const QVector2D& rotationMaxBound() const { return mRotationMaxBound; }
    ScOrbitalCamera& setRotationMaxBound(const QVector2D& rotationMaxBound)
    {
        mRotationMaxBound = rotationMaxBound;
        return *this;
    }
    float rotationMaxBoundX() const { return rotationMaxBound().x(); }
    ScOrbitalCamera& setRotationMaxBoundX(float x)
    {
        setRotationMaxBound({ x, rotationMaxBoundY() });
        return *this;
    }
    float rotationMaxBoundY() const { return rotationMaxBound().y(); }
    ScOrbitalCamera& setRotationMaxBoundY(float y)
    {
        setRotationMaxBound({ rotationMaxBoundX(), y });
        return *this;
    }

    const QVector3D& rotationCenter() const { return mRotationCenter; }
    ScOrbitalCamera& setRotationCenter(const QVector3D& rotationCenter)
    {
        mRotationCenter = rotationCenter;
        return *this;
    }

    const QVector3D& rotationOrbit() const { return mRotationOrbit; }
    ScOrbitalCamera& setRotationOrbit(const QVector3D& rotationOrbit)
    {
        mRotationOrbit = rotationOrbit;
        return *this;
    }

	QMatrix4x4 viewMatrix() const override
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

private:
    QVector2D mRotation, mRotationMinBound = { -90.0f, -360.0f }, mRotationMaxBound = { 90.0f, 360.0f };
    QVector3D mRotationCenter, mRotationOrbit = { 0.0f, 0.0f, 5.0f };
};

typedef QSharedPointer<ScOrbitalCamera> ScOrbitalCamera_p;
typedef QSharedPointer<const ScOrbitalCamera> ScOrbitalCamera_cp;

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

/*!
 * The vertex data structure.
 */
struct ScVertexData
{
	QVector3D vertexCoord;
	QVector2D textureCord;
	QVector3D normal;
    QVector4D color;
};

/*!
 * The editable mesh class.
 */
class ScEditableMesh final
{
public:
    ScEditableMesh() {}
    ScEditableMesh(const ScVertexData* vertices, int count)
	{
		load(vertices, count);
	}

    void load(const ScVertexData* vertices, int count)
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
    }

    void render(QOpenGLShaderProgram& shaderProgram)
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

private:
    int mVerticesCount = 0;
    QOpenGLBuffer mVertexBuffer;
};

typedef QSharedPointer<ScEditableMesh> ScEditableMesh_p;
typedef QSharedPointer<const ScEditableMesh> ScEditableMesh_cp;

typedef QSharedPointer<QOpenGLShaderProgram> QOpenGLShaderProgram_p;
typedef QSharedPointer<const QOpenGLShaderProgram> QOpenGLShaderProgram_cp;

typedef QSharedPointer<QOpenGLTexture> QOpenGLTexture_p;
typedef QSharedPointer<const QOpenGLTexture> QOpenGLTexture_cp;

static QOpenGLShaderProgram_p pLoadShaderProgram(const char* vertexShaderPath, const char* pixelShaderPath)
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
static QOpenGLTexture_p pLoadTexture(const char* texturePath)
{
    QOpenGLTexture_p texture(new QOpenGLTexture(QImage(texturePath)));
    return texture;
}

/*!
 * The mesh renderer class.
 */
class ScMeshRenderer
{
public:
    virtual ~ScMeshRenderer()
	{
	}

    bool enabled() const { return mEnabled; }
    ScMeshRenderer& setEnabled(bool enabled)
    {
        mEnabled = enabled;
        return *this;
    }
    ScMeshRenderer& enable()
    {
        return setEnabled(true);
    }
    ScMeshRenderer& disable()
    {
        return setEnabled(false);
    }

    const QVector3D& position() const { return mPosition; }
	virtual ScMeshRenderer& setPosition(const QVector3D& position)
    {
        mPosition = position;
        return *this;
    }

    const QVector3D& offset() const { return mOffset; }
	virtual ScMeshRenderer& setOffset(const QVector3D& offset)
    {
        mOffset = offset;
        return *this;
    }

    const QQuaternion& rotation() const { return mRotation; }
	virtual ScMeshRenderer& setRotation(const QQuaternion& rotation)
    {
        mRotation = rotation;
        return *this;
    }
	virtual ScMeshRenderer& setRotation(const QVector3D& rotation)
    {
        return setRotation(QQuaternion::fromEulerAngles(rotation));
    }

    const QVector3D& scale() const { return mScale; }
	virtual ScMeshRenderer& setScale(const QVector3D& scale)
    {
        mScale = scale;
        return *this;
    }

    virtual QMatrix4x4 modelMatrix() const
    {
        QMatrix4x4 model;
        model.setToIdentity();
        model.translate(position());
        model.rotate(rotation());
        model.translate(offset());
        model.scale(scale());
        return model;
    }
    virtual QMatrix3x3 normalMatrix() const
    {
        QMatrix4x4 model = modelMatrix();
        QMatrix3x3 normal(model.normalMatrix());
        return normal;
    }

    ScEditableMesh_p mesh() { return mMesh; }
    ScEditableMesh_cp mesh() const { return mMesh; }
    ScMeshRenderer& setMesh(const ScEditableMesh_p& mesh)
    {
        mMesh = mesh;
        return *this;
    }

    QOpenGLShaderProgram_p shaderProgram() { return mShaderProgram; }
    QOpenGLShaderProgram_cp shaderProgram() const { return mShaderProgram; }
    ScMeshRenderer& setShaderProgram(const QOpenGLShaderProgram_p& shaderProgram)
    {
        mShaderProgram = shaderProgram;
        return *this;
    }

    QOpenGLTexture_p texture() { return mTexture; }
    QOpenGLTexture_cp texture() const { return mTexture; }
    ScMeshRenderer& setTexture(const QOpenGLTexture_p& texture)
    {
        mTexture = texture;
        return *this;
    }

    virtual void render(const ScBasicCamera& camera)
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

private:
    bool mEnabled = true;
    QQuaternion mRotation;
    QVector3D mPosition, mOffset;
    QVector3D mScale = { 1.0f, 1.0f, 1.0f };
    ScEditableMesh_p mMesh;
    QOpenGLShaderProgram_p mShaderProgram;
    QOpenGLTexture_p mTexture;
};

typedef QSharedPointer<ScMeshRenderer> ScMeshRenderer_p;
typedef QSharedPointer<const ScMeshRenderer> ScMeshRenderer_cp;

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

/*!
 * The abstract scene class.
 */
class ScScene
{
public:
    virtual ~ScScene() { }

    virtual void onMouseDrag(const QVector2D&) {}
	virtual void onResize(float, float) {}
    virtual void render() {}
};

/*!
 * The OpenGL context widget class.
 */
class ScOpenGLWidget final : public QOpenGLWidget
{
public:
    ScOpenGLWidget(QWidget* parent = Q_NULLPTR):
        QOpenGLWidget(parent)
    {
    }

protected:
	void mousePressEvent(QMouseEvent* event) override final;
	void mouseMoveEvent(QMouseEvent* event) override final;
	void mouseReleaseEvent(QMouseEvent* event) override final;
	void wheelEvent(QWheelEvent *event) override final;

    void initializeGL() override final;
    void resizeGL(int w, int h) override final;
    void paintGL() override final;

private:
	bool mMouseDragBegan = false;
	QPointF mMousePressPosition;
public:
    ScScene* mScene = Q_NULLPTR;
	QtCharts::QChartView* mChart;
};

#endif // POPENGLWIDGET_H
