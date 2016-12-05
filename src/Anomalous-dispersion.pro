#/****************************************************************************
#**
#** Copyright (C) 2016 Plaxin Gleb, Butakov Oleg.
#** Contact: kandidov_i_chickishev@ublidky.com
#** License: MIT.
#**
#****************************************************************************/

QT += core gui widgets opengl charts

TARGET = Anomalous-dispresion
TEMPLATE = app

!macx {
    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_LFLAGS *= -fopenmp
    win32 {
        LIBS += -lOpengl32
    }
}

SOURCES += \
    widgets/SceneWidget.cpp \
    widgets/SceneWidgetAdvanced.cpp \
    widgets/PlotViewWidget.cpp \
    forms/MenuWindow.cpp \
    forms/AuthorsWindow.cpp \
    forms/SceneWindow.cpp \
    SceneMeshes.cpp \
    PresentationScene.cpp \
    PresentationPhysics.cpp \
    PresentationGeometry.cpp \
    main.cpp \

HEADERS  += \
    widgets/SceneWidget.h \
    widgets/SceneWidgetAdvanced.h \
    widgets/PlotViewWidget.h \
    forms/MenuWindow.h \
    forms/AuthorsWindow.h \
    forms/SceneWindow.h \
    SceneMeshes.h \
    PresentationScene.h \
    PresentationPhysics.h \
    PresentationGeometry.h \

FORMS += \
    forms/MenuWindow.ui \
    forms/AuthorsWindow.ui \
    forms/SceneWindow.ui \

DISTFILES += \
    shaders/vertex.glsl \
    shaders/vertexRefract.glsl \
    shaders/fragLit.glsl \
    shaders/fragUnlitTextured.glsl \
    shaders/fragUnlit.glsl \
    shaders/fragRefract.glsl

RESOURCES += \
    resources/Scene.qrc \
    resources/Images.qrc \
