/****************************************************************************
**
** Copyright (C) 2016 Plaxin Gleb, Butakov Oleg.
** Contact: kandidov_i_chickishev@ublidky.com
** License: MIT.
**
****************************************************************************/

#include "SceneWidgetAdvanced.h"

QOpenGLTexture_p scLoadCubemap(const char* texturePath, const char* extension,
							   const char* leftSuffix, const char* rightSuffix,
							   const char* upSuffix, const char* downSuffix,
							   const char* frontSuffix, const char* backSuffix)
{

	QString format("%1%2%3");
	const char* suffixes[] = {
		rightSuffix, leftSuffix, upSuffix, downSuffix,
		backSuffix, frontSuffix
	};

	QOpenGLTexture_p cubemap(new QOpenGLTexture(QOpenGLTexture::TargetCubeMap));
	for (int i = 0; i < _countof(suffixes); ++i)
	{
		QString cubeSidePath = format.arg(texturePath, suffixes[i], extension);
		QImage cubeSide = QImage(cubeSidePath).convertToFormat(QImage::Format_RGBA8888);
		if (!cubemap->isCreated())
		{
			cubemap->create();
			cubemap->setSize(cubeSide.width(), cubeSide.height(), cubeSide.depth());
			cubemap->setFormat(QOpenGLTexture::RGBA8_UNorm);
			cubemap->allocateStorage();
		}

		cubemap->setData(0, 0, static_cast<QOpenGLTexture::CubeMapFace>(QOpenGLTexture::CubeMapPositiveX + i),
						 QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, cubeSide.constBits());
	}

	return cubemap;
}
