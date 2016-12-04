#ifndef MESHES_H
#define MESHES_H

#ifndef _countof
#define _countof(a) (sizeof(a) / sizeof(a[0]))
#endif

#include "widgets/SceneWidget.h"

extern const ScVertexData triangleVertices[3];

extern const ScVertexData screenVertices[2106];
extern const ScVertexData roomVertices[5298];

extern const QVector3D prismMinBound, prismMaxBound;
extern const ScVertexData prismVertices[24];

extern const QVector3D prismHolderLegMinBound, prismHolderLegMaxBound;
extern const ScVertexData prismHolderLegVertices[168];

extern const QVector3D prismHolderGimbalMinBound, prismHolderGimbalMaxBound;
extern const ScVertexData prismHolderGimbalVertices[696];

extern const QVector3D prismHolderBaseMinBound, prismHolderBaseMaxBound;
extern const ScVertexData prismHolderBaseVertices[564];

#endif
