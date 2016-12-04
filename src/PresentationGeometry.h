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
	
	/*!
	 * Generates a mesh the represents the projection of the beam cone to some surface.
	 * 
	 * @param beamCone The actual beam cone.
	 * @param[out] vertices Output for generated vertices.
	 * @param thickness The thickness of the projection line.
	 */
	static void generateBeamProjMesh(const OpBeamCone& beamCone, QVector<ScVertexData>& vertices, float thickness = 0.01f);

	/*!
	 * Generates a mesh the represents the the beam cone.
	 * 
	 * @param beamCone The actual beam cone.
	 * @param[out] vertices Output for generated vertices.
	 */
	static void generateBeamMesh(const OpBeamCone& beamCone, QVector<ScVertexData>& vertices);

};
