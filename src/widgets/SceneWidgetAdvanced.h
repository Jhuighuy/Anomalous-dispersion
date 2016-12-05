/****************************************************************************
**
** Copyright (C) 2016 Plaxin Gleb, Butakov Oleg.
** Contact: kandidov_i_chickishev@ublidky.com
** License: MIT.
**
****************************************************************************/

#pragma once

#include "SceneWidget.h"

extern QOpenGLTexture_p scLoadCubemap(const char* texturePath, const char* extension,
									  const char* leftSuffix = "_LF", const char* rightSuffix = "_RT",
									  const char* upSuffix = "_UP", const char* downSuffix = "_DN",
									  const char* frontSuffix = "_FR", const char* backSuffix = "_BK");
