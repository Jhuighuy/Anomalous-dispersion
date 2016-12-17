/****************************************************************************
**
** Copyright (C) 2016 Plaxin Gleb, Butakov Oleg.
** License: MIT.
**
****************************************************************************/

#pragma once

#include <QtMath>

#include <QSharedPointer>
#include <QRectF>
#include <QOpenGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QChartView>

#include <cmath>

#define USE_ADVANCED_RENDERING 0

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

//! @todo rewrite using SSE.
struct ScVector3D
{
	union {
		struct { qreal x, y, z; };
		qreal v[3];
	};

	ScVector3D() : x(0.0), y(0.0), z(0.0) {}
	ScVector3D(const QVector3D& vector) :
		x(vector.x()), y(vector.y()), z(vector.z())
	{

	}

	static qreal dotProduct(const ScVector3D& a, const ScVector3D& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}
	qreal length() const
	{
		return qSqrt(dotProduct(*this, *this));
	}

	ScVector3D& operator+= (const ScVector3D& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}
};

//! @todo rewrite using SSE.
struct ScMatrix4x4
{
	qreal m[4][4];

	ScMatrix4x4() : m{} {}
	ScMatrix4x4(const QMatrix4x4& matrix)
	{
		for (int row = 0; row < 4; ++row)
		{
			for (int column = 0; column < 4; ++column)
			{
				m[row][column] = matrix(row, column);
			}
		}
	}

	ScVector3D operator* (const ScVector3D& vector) const
	{
		qreal v[4] = { vector.x, vector.y, vector.z, 1.0 };
		qreal result[4] = {0.0f};
		for (int row = 0; row < 4; ++row)
		{
			for (int column = 0; column < 4; ++column)
			{
				result[row] += m[row][column] * v[row];
			}
		}

		ScVector3D r;
		r.x = result[0] / result[3];
		r.y = result[1] / result[3];
		r.z = result[2] / result[3];
		return r;
	}
};

/*!
 * The basic transform class. 
 */
class ScTransform
{
public:
	virtual ~ScTransform()
	{
	}

	const QVector3D& position() const { return mPosition; }
	virtual ScTransform& setPosition(const QVector3D& position)
	{
		mPosition = position;
		return *this;
	}

	const QQuaternion& rotation() const { return mRotation; }
	virtual ScTransform& setRotation(const QQuaternion& rotation)
	{
		mRotation = rotation;
		return *this;
	}

    QVector3D rotationDegrees() const
    {
        QQuaternion rotationQuat = rotation();
        return rotationQuat.toEulerAngles();
    }
    ScTransform& setRotationDegrees(const QVector3D& rotation)
	{
		return setRotation(QQuaternion::fromEulerAngles(rotation));
	}

	const QVector3D& scale() const { return mScale; }
	virtual ScTransform& setScale(const QVector3D& scale)
	{
		mScale = scale;
		return *this;
	}

	virtual QMatrix4x4 modelMatrix() const;
	virtual QMatrix3x3 normalMatrix() const;

public:
	QQuaternion mRotation;
	QVector3D mPosition, mScale = { 1.0f, 1.0f, 1.0f };
};

/*!
 * The transform with an offset class. 
 */
class ScOffsetTransform : public ScTransform
{
public:
	const QVector3D& offset() const { return mOffset; }
	virtual ScTransform& setOffset(const QVector3D& offset)
	{
		mOffset = offset;
		return *this;
	}

	QMatrix4x4 modelMatrix() const override;

private:
	QVector3D mOffset;
};

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

    const QVector4D& clearColor() const { return mClearColor; }
    ScBasicCamera& setClearColor(const QVector4D& clearColor)
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
    QVector4D mClearColor;
};

/*!
 * The projection camera class.
 */
class ScProjectionCamera final : public ScBasicCamera, public ScTransform
{
    DEFINE_CREATE_FUNC(ScProjectionCamera)

public:
    float size() const { return mSize; }
    ScProjectionCamera& setSize(float size)
    {
        mSize = size;
        return *this;
    }

    QMatrix4x4 viewMatrix() const override;
    QMatrix4x4 projectionMatrix() const override;

    void beginScene() const override;
    void endScene() const override;

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
        mRotationMinBound.setX(x);
        return *this;
    }
    float rotationMinBoundY() const { return rotationMinBound().y(); }
    ScOrbitalCamera& setRotationMinBoundY(float y)
    {
        mRotationMinBound.setY(y);
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
        mRotationMaxBound.setX(x);
        return *this;
    }
    float rotationMaxBoundY() const { return rotationMaxBound().y(); }
    ScOrbitalCamera& setRotationMaxBoundY(float y)
    {
        mRotationMaxBound.setY(y);
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
DEFINE_SHARED_PTR(ScTransparentMeshRenderer)

extern QOpenGLShaderProgram_p scLoadShaderProgram(const char* vertexShaderPath, const char* pixelShaderPath);
extern QOpenGLTexture_p scLoadTexture(const char* texturePath);

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
class ScEditableMesh final : public QVector<ScVertexData>
{
    DEFINE_CREATE_FUNC(ScEditableMesh)

public:
    ScEditableMesh() {}
    ScEditableMesh(const ScVertexData* vertices, int count, bool cacheVertices = false, bool computeAABB = false)
	{
        setVertices(vertices, count, cacheVertices, computeAABB);
	}

	const QVector3D& minBound() const { return mMinBound; }
	const QVector3D& maxBound() const { return mMaxBound; }

	int uncachedVerticesCount() const { return mVerticesCount; }

    /*!
     * Loads a vertices of the editable mesh.
     *
     * @param vertices The actual array of the vertex data.
     * @param count The amount of the vertices.
     * @param cacheVertices Do cache vertices inside the mesh object?
     * @param boundsDim How many dimension use while computing the AABB.
     */
    ScEditableMesh& setVertices(const ScVertexData* vertices, int count, bool cacheVertices = false, bool computeAABB = false);

    /*!
     * Renders part of the mesh with the specified shader program.
     * 
     * @param shaderProgram Program to be used while rendering.
     * @param firstVertex First vertex to render.
     * @param count Amount of the vertices to render.
     */
	void render(QOpenGLShaderProgram& shaderProgram, int firstVertex, int count);

	/*!
	 * Renders this mesh with the specified shader program.
	 * @param shaderProgram Program to be used while rendering.
	 */
	void render(QOpenGLShaderProgram& shaderProgram)
	{
		render(shaderProgram, 0, uncachedVerticesCount());
	}

private:
	int mVerticesCount = 0;
	QOpenGLBuffer mVertexBuffer;
	QVector3D mMinBound, mMaxBound;
};

/*!
 * The mesh renderer class.
 */
class ScMeshRenderer : public ScOffsetTransform
{
    DEFINE_CREATE_FUNC(ScMeshRenderer)

public:
    virtual ~ScMeshRenderer()
	{
	}

    bool enabled() const { return mEnabled; }
    virtual ScMeshRenderer& setEnabled(bool enabled)
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

    QOpenGLTexture_p diffuseTexture() const { return mDiffuseTexture; }
    ScMeshRenderer& setDiffuseTexture(const QOpenGLTexture_p& diffuseTexture)
    {
        mDiffuseTexture = diffuseTexture;
        return *this;
    }

	const QVector4D& color() const { return mColor; }
	ScMeshRenderer& setColor(const QVector4D& color)
	{
		mColor = color;
		return *this;
	}

    /*!
     * Renders this object with the specified camera.
     * @param camera The camera to be used while rendering.
     */
    virtual void render(const ScBasicCamera& camera);

protected:
	void beginRender(const ScBasicCamera& camera) const;
	void endRender() const;

private:
    bool mEnabled = true;
    ScEditableMesh_p mMesh;
    QOpenGLShaderProgram_p mShaderProgram;
    QOpenGLTexture_p mDiffuseTexture;
	QVector4D mColor = { 1.0f, 1.0f, 1.0f, 1.0f };
};

/*!
 * The transparent mesh renderer class.
 */
class ScTransparentMeshRenderer : public ScMeshRenderer
{
	DEFINE_CREATE_FUNC(ScTransparentMeshRenderer)

public:

	/*!
	 * Renders this transparent object with the specified camera.
	 * @param camera The camera to be used while rendering.
	 */
	void render(const ScBasicCamera& camera) override;
};

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

DEFINE_SHARED_PTR(ScScene)

/*!
 * The abstract scene class.
 */
class ScScene
{
	DEFINE_CREATE_FUNC(ScScene)

public:
    virtual ~ScScene() { }

	virtual void init() {}
	virtual void render() {}

    virtual void onMouseDrag(const QVector2D&) {}
	virtual void onResize(float, float) {}
};

/*!
 * The OpenGL context widget class.
 */
class ScOpenGLWidget final : public QOpenGLWidget
{
public:
    ScOpenGLWidget(QWidget* parent = nullptr):
        QOpenGLWidget(parent)
    {
    }

public:
	ScScene_p scene() const { return mScene; }
	ScOpenGLWidget& setScene(const ScScene_p& scene)
	{
		mScene = scene;
		return *this;
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
    ScScene_p mScene;
};
