/****************************************************************************
**
** Copyright (C) 2016 Plaxin Gleb, Butakov Oleg.
** Contact: kandidov_i_chickishev@ublidky.com
** License: MIT.
**
****************************************************************************/

#pragma once

#include "PresentationPhysics.h"
#include "widgets/SceneWidget.h"

class PresentationGeometry
{
public:
    static const float defaultThickness;

	/*!
	 * Generates a mesh the represents the projection of the beam cone to some surface.
	 * 
	 * @param beamCone The actual beam cone.
     * @param screenNormal The normal of the projection plane.
	 * @param[out] vertices Output for generated vertices.
	 * @param thickness The thickness of the projection line.
	 */
    static void generateBeamProjMesh(const PhBeamCone& beamCone, const QVector3D& screenNormal,
                                     QVector<ScVertexData>& vertices, float thickness = defaultThickness);

	/*!
	 * Generates a mesh the represents the the beam cone.
	 * 
	 * @param beamCone The actual beam cone.
	 * @param[out] vertices Output for generated vertices.
	 */
	static void generateBeamMesh(const PhBeamCone& beamCone, QVector<ScVertexData>& vertices);

};
