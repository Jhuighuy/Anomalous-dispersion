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

DEFINE_SHARED_PTR(ScBasicCamera)
DEFINE_SHARED_PTR(ScProjectionCamera)
DEFINE_SHARED_PTR(ScOrbitalCamera)

/*!
 * The basic camera class.
 */
class ScBasicCamera
{
    DEFINE_CREATE_FUNC(ScBasicCamera)

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

    virtual QMatrix4x4 viewMatrix() const;
    virtual QMatrix4x4 projectionMatrix() const;

    virtual void beginScene() const;
    virtual void endScene() const;

private:
    float mWidth = 800.0f, mHeight = 600.0f;
    QRectF mViewport = {0.0f, 0.0f, 1.0f, 1.0f};
    QColor mClearColor;
};

/*!
 * The projection camera class.
 */
//! @todo Drive this class from QTransform.
class ScProjectionCamera final : public ScBasicCamera
{
    DEFINE_CREATE_FUNC(ScProjectionCamera)

public:
    const QVector3D& position() const { return mPosition; }
    ScProjectionCamera& setPosition(const QVector3D& position)
    {
        mPosition = position;
        return *this;
    }

    const QQuaternion& rotation() const { return mRotation; }
    ScProjectionCamera& setRotation(const QQuaternion& rotation)
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

    QMatrix4x4 viewMatrix() const override;
    QMatrix4x4 projectionMatrix() const override;

    /*
    void beginScene() const override;
    void endScene() const override;
    */

private:
    float mSize = 5.0f;
    QVector3D mPosition;
    QQuaternion mRotation;
};

/*!
 * The orbital camera class.
 */
class ScOrbitalCamera final : public ScBasicCamera
{
    DEFINE_CREATE_FUNC(ScOrbitalCamera)

public:
    const QVector2D& rotation() const { return mRotation; }
    ScOrbitalCamera& rotate(const QVector2D& deltaRotation);

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

    QMatrix4x4 viewMatrix() const override;

private:
    QVector2D mRotation, mRotationMinBound = { -90.0f, -360.0f }, mRotationMaxBound = { 90.0f, 360.0f };
    QVector3D mRotationCenter, mRotationOrbit = { 0.0f, 0.0f, 5.0f };
};

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

DEFINE_SHARED_PTR(QOpenGLShaderProgram)
DEFINE_SHARED_PTR(QOpenGLTexture)
DEFINE_SHARED_PTR(ScEditableMesh)
DEFINE_SHARED_PTR(ScMeshRenderer)

extern QOpenGLShaderProgram_p pLoadShaderProgram(const char* vertexShaderPath, const char* pixelShaderPath);
extern QOpenGLTexture_p pLoadTexture(const char* texturePath);

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
//! @todo Drive this class from QOpenGLBuffer directly.
class ScEditableMesh final
{
    DEFINE_CREATE_FUNC(ScEditableMesh)

public:
    ScEditableMesh() {}
    ScEditableMesh(const ScVertexData* vertices, int count, bool cacheVertices = false, bool computeAABB = false)
	{
        load(vertices, count, cacheVertices, computeAABB);
	}

    /*!
     * Loads a vertices of the editable mesh.
     *
     * @param vertices The actual array of the vertex data.
     * @param count The amount of the vertices.
     * @param cacheVertices Do cache vertices inside the mesh object?
     * @param computeAABB Do compute the bounding box for this mesh?
     */
    ScEditableMesh& load(const ScVertexData* vertices, int count, bool cacheVertices = false, bool computeAABB = false);

    /*!
     * Renders this mesh with the specified shader program.
     * @param shaderProgram Program to be used while rendering.
     */
    void render(QOpenGLShaderProgram& shaderProgram);


private:
    int mVerticesCount = 0;
    QOpenGLBuffer mVertexBuffer;
};

/*!
 * The mesh renderer class.
 */
//! @todo Drive this class from QTransform.
class ScMeshRenderer
{
    DEFINE_CREATE_FUNC(ScMeshRenderer)

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

    virtual QMatrix4x4 modelMatrix() const;
    virtual QMatrix3x3 normalMatrix() const;

    ScEditableMesh_p mesh() const { return mMesh; }
    ScMeshRenderer& setMesh(const ScEditableMesh_p& mesh)
    {
        mMesh = mesh;
        return *this;
    }

    QOpenGLShaderProgram_p shaderProgram() const { return mShaderProgram; }
    ScMeshRenderer& setShaderProgram(const QOpenGLShaderProgram_p& shaderProgram)
    {
        mShaderProgram = shaderProgram;
        return *this;
    }

    QOpenGLTexture_p texture() const { return mTexture; }
    ScMeshRenderer& setTexture(const QOpenGLTexture_p& texture)
    {
        mTexture = texture;
        return *this;
    }

    /*!
     * Renders this object with the specified camera.
     * @param camera The camera to be used while rendering.
     */
    virtual void render(const ScBasicCamera& camera);

private:
    bool mEnabled = true;
    QQuaternion mRotation;
    QVector3D mPosition, mOffset;
    QVector3D mScale = { 1.0f, 1.0f, 1.0f };
    ScEditableMesh_p mMesh;
    QOpenGLShaderProgram_p mShaderProgram;
    QOpenGLTexture_p mTexture;
};

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
