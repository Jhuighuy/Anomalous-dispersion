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
#include <GL/GL.h>

#ifndef _countof
#define _countof(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif



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

