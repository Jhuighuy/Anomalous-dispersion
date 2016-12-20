/****************************************************************************
**
** Copyright (C) 2016 Plaxin Gleb, Butakov Oleg.
** Contact: kandidov_i_chickishev@ublidky.com
** License: MIT.
**
****************************************************************************/

#include "PresentationGeometry.h"

class PresentationGeometryImpl final
{
public:
	template<typename T>
	static void thickenPoint(const T& point,  const QVector3D& pointNormal, float thickness,  QVector<ScVertexData>& vertices);
	
public:
	template<typename T>
	static void thickenLine(const QVector<T>& line, const QVector3D& lineNormal, float thickness, QVector<ScVertexData>& vertices);
private:
	static void thickenFirstPoint(const QVector3D& lineNormal, const QVector3D& X, const QVector3D& Xplus, float halfThickness, QVector3D& Yplus, QVector3D& Yminus);
	static void thickenLastPoint(const QVector3D& lineNormal, const QVector3D& Xminus, const QVector3D& X, float halfThickness, QVector3D& Yplus, QVector3D& Yminus);
	static void thickenMidPoint(const QVector3D& lineNormal, const QVector3D& Xminus, const QVector3D& X, const QVector3D& Xplus, float halfThickness, QVector3D& Yplus, QVector3D& Yminus);

public:
	template<typename T>
	static void bridgeLines(const QVector<T>& lineA, const QVector<T>& lineB, QVector<ScVertexData>& vertices);
private:
	template<typename T>
	static void bridgeLinesImpl(const QVector<T>& lineA, const QVector<T>& lineB, QVector<ScVertexData>& vertices);
	template<typename T>
	static void bridgePointLine(const T& point, typename QVector<T>::const_iterator lineBegin, typename QVector<T>::const_iterator lineEnd, QVector<ScVertexData>& vertices);
	template<typename T>
	static void bridgePointLine(const T& point, const QVector<T>& line, QVector<ScVertexData>& vertices);
};

const float PresentationGeometry::defaultThickness = 0.022f;
const float PresentationGeometry::defaultProjMultiplier = 1.5f;

/*!
 * Generates a mesh the represents the projection of the beam cone to some surface.
 *
 * @param beamCone The actual beam cone.
 * @param screenNormal The normal of the projection plane.
 * @param[out] vertices Output for generated vertices.
 * @param thickness The thickness of the projection line.
 */
void PresentationGeometry::generateBeamProjMesh(const PhBeamCone& beamCone, const QVector3D& screenNormal,
                                                QVector<ScVertexData>& vertices, float thickness)
{
    Q_ASSERT(beamCone.collisionLevels() > 1);

	PhBeamCollisionInfo beamFinalCollision;
	beamCone.getCollisionLevel(beamFinalCollision, beamCone.collisionLevels() - 1, 
                               1.0f, { 1.2f, defaultProjMultiplier, 1.0f});

	if (beamFinalCollision.size() == 1)
    {
        PresentationGeometryImpl::thickenPoint(beamFinalCollision.first(), screenNormal, thickness, vertices);
	}
	else
    {
        PresentationGeometryImpl::thickenLine(beamFinalCollision, screenNormal, thickness, vertices);
	}
}

/*!
 * Generates a mesh the represents the the beam cone.
 * 
 * @param beamCone The actual beam cone.
 * @param[out] vertices Output for generated vertices.
 */
void PresentationGeometry::generateBeamMesh(const PhBeamCone& beamCone, QVector<ScVertexData>& vertices)
{
	Q_ASSERT(beamCone.collisionLevels() > 1);

	PhBeamCollisionInfo beamPrevCollision;
	beamCone.getCollisionLevel(beamPrevCollision, 0);

	for (int i = 1; i < beamCone.collisionLevels(); ++i)
	{
		bool isLastLevel = i == beamCone.collisionLevels() - 1;
		float alphaMultiplier = static_cast<float>(!isLastLevel);

		PhBeamCollisionInfo beamCollision;
		if (i == beamCone.collisionLevels() - 1)
		{
			beamCone.getCollisionLevel(beamCollision, i, alphaMultiplier, 
                                       { 1.2f, defaultProjMultiplier, 1.0f});
		}
		else
		{
			beamCone.getCollisionLevel(beamCollision, i, alphaMultiplier);
		}

		PresentationGeometryImpl::bridgeLines(beamCollision, beamPrevCollision, vertices);
		beamPrevCollision = qMove(beamCollision);
	}
}

/*!
 * Generates a thick triangle shape from the specified point.
 * 
 * @param line The actual line.
 * @param lineNormal Normal of the resulting shape.
 * @param thickness The thickness of the resulting shape.
 * @param[out] vertices Output for generated vertices.
 */
template<typename T>
void PresentationGeometryImpl::thickenPoint(const T& point,
                                            const QVector3D& projNormal, float thickness,
											QVector<ScVertexData>& vertices)
{
    float halfThickness = 0.5f * thickness;
    static const QVector3D up(0.0f, 1.0f, 0.0f);
    const QVector3D& P = point.position;
    const QVector4D& C = point.color;

    QVector3D tangent = QVector3D::normal(up, projNormal);
    QVector3D adjHor = halfThickness * tangent;
    QVector3D adjVert = halfThickness * up;

    ScVertexData Vpp = { P + adjHor + adjVert, {1.0f, 1.0f}, projNormal, C };
    ScVertexData Vpm = { P + adjHor - adjVert, {1.0f, 0.0f}, projNormal, C };
    ScVertexData Vmp = { P - adjHor + adjVert, {0.0f, 1.0f}, projNormal, C };
    ScVertexData Vmm = { P - adjHor - adjVert, {0.0f, 0.0f}, projNormal, C };

    vertices.push_back(Vpp);
    vertices.push_back(Vpm);
    vertices.push_back(Vmp);
    // ----------------------
    vertices.push_back(Vmp);
    vertices.push_back(Vmm);
    vertices.push_back(Vpm);
}

/*!
 * Generates a thick triangle shape from the specified line.
 * 
 * @param line The actual line.
 * @param projNormal Normal of the resulting shape.
 * @param thickness The thickness of the resulting shape.
 * @param[out] vertices Output for generated vertices.
 */
template<typename T>
void PresentationGeometryImpl::thickenLine(const QVector<T>& line,
                                           const QVector3D& projNormal, float thickness,
										   QVector<ScVertexData>& vertices)
{
	Q_ASSERT(line.size() > 1);

	float halfThickness = 0.5f * thickness;

    auto P = [&](int i) -> const QVector3D& { return line[i].position; };
	auto C = [&](int i) -> const QVector4D& { return line[i].color; };
	int M = line.size() - 1;

	// Part 1: processing first non-bended node.
    ScVertexData Vplus0 = { { }, { 0.0f, 1.0f }, projNormal, C(0) };
    ScVertexData Vminus0 = { { }, { 1.0f, 1.0f }, projNormal, C(0) };
    thickenFirstPoint(projNormal, P(0), P(1), halfThickness, Vplus0.vertexCoord, Vminus0.vertexCoord);
	ScVertexData Vsaved = Vminus0;
	vertices.push_back(Vplus0);
	vertices.push_back(Vminus0);

	// Part 2: processing mid bended nodes.
	for (int i = 1; i < M; ++i)
	{
        ScVertexData Vplus = { { }, { 0.0f, 0.5f }, projNormal, C(i) };
        ScVertexData Vminus = { { }, { 1.0f, 0.5f }, projNormal, C(i) };
        PresentationGeometryImpl::thickenMidPoint(projNormal, P(i - 1), P(i), P(i + 1), halfThickness, Vplus.vertexCoord, Vminus.vertexCoord);

		vertices.push_back(Vplus);
		// ----------------------
		vertices.push_back(Vplus);
		vertices.push_back(Vminus);
		vertices.push_back(Vsaved);
		// ----------------------
		vertices.push_back(Vplus);
		vertices.push_back(Vminus);
		Vsaved = Vminus;
	}

	// Part 3: processing last non-bended node.
    ScVertexData VplusM = { { }, { 0.0f, 0.0f }, projNormal, C(M) };
    ScVertexData VminusM = { { }, { 1.0f, 0.0f }, projNormal, C(M) };
    PresentationGeometryImpl::thickenLastPoint(projNormal, P(M - 1), P(M), halfThickness, VplusM.vertexCoord, VminusM.vertexCoord);
	vertices.push_back(VplusM);
	// ----------------------
	vertices.push_back(VplusM);
	vertices.push_back(VminusM);
	vertices.push_back(Vsaved);
}
void PresentationGeometryImpl::thickenFirstPoint(const QVector3D& projNormal,
												 const QVector3D& X, const QVector3D& Xplus, float halfThickness,
												 QVector3D& Yplus, QVector3D& Yminus)
{
	QVector3D a = Xplus - X;
    QVector3D normal = QVector3D::normal(a, projNormal);
    QVector3D adj = halfThickness * normal;

	Yplus = X + adj;
	Yminus = X - adj;
}
void PresentationGeometryImpl::thickenLastPoint(const QVector3D& projNormal,
												const QVector3D& Xminus, const QVector3D& X, float halfThickness, 
												QVector3D& Yplus, QVector3D& Yminus)
{
	QVector3D a = X - Xminus;
    QVector3D normal = QVector3D::normal(a, projNormal);
    QVector3D adj = halfThickness * normal;

	Yplus = X + adj;
	Yminus = X - adj;
}
void PresentationGeometryImpl::thickenMidPoint(const QVector3D& projNormal,
											   const QVector3D& Xminus, const QVector3D& X, const QVector3D& Xplus, float halfThickness, 
											   QVector3D& Yplus, QVector3D& Yminus)
{
	QVector3D a = X - Xminus;
	QVector3D b = Xplus - X;
    QVector3D averageNormal = 0.5f * (QVector3D::normal(a, projNormal) + QVector3D::normal(b, projNormal));
    QVector3D adj = halfThickness * averageNormal;

	Yplus = X + adj;
	Yminus = X - adj;
}

/*!
 * Connect two lines with triangles.
 * 
 * @param lineA First line to connect.
 * @param lineB Second line to connect.
 * @param[out] vertices Output for generated vertices. 
 */
template<typename T>
void PresentationGeometryImpl::bridgeLines(const QVector<T>& lineA, const QVector<T>& lineB, QVector<ScVertexData>& vertices)
{
	if (lineA.size() < lineB.size())
	{
		PresentationGeometryImpl::bridgeLinesImpl(lineA, lineB, vertices);
	}
	else
	{
		PresentationGeometryImpl::bridgeLinesImpl(lineB, lineA, vertices);
	}
}
template<typename T>
void PresentationGeometryImpl::bridgeLinesImpl(const QVector<T>& lineA, const QVector<T>& lineB, QVector<ScVertexData>& vertices)
{
	Q_ASSERT(lineA.size() <= lineB.size());

	if (lineA.size() == 1)
	{
		if (lineB.size() == 1)
		{
			// In this case we have just two points we need to connect.
            // Lets connect them with thick lines.
            static const QVector3D up(0.0f, 1.0f, 0.0f);
            QVector3D lineDirection = lineB[0].position - lineA[0].position;
            QVector3D lineNormal = QVector3D::normal(lineDirection, up);

            QVector<T> line { lineA[0], lineB[0] };
            static const float thickness = 0.5f * PresentationGeometry::defaultThickness;
            float halfThickness = 0.5f * thickness;
            PresentationGeometryImpl::thickenLine(line, lineNormal, halfThickness, vertices);
		}
		else
		{
			// In this case we have to just connect an only point in line A with line B.
			PresentationGeometryImpl::bridgePointLine(lineA.first(), lineB, vertices);
		}
		return;
	}

	int M = lineA.size() - 1;
	int N = lineB.size() - 1;
	int L = N / M;
	int LhalfMinus = L / 2, LhalfPlus = L - LhalfMinus;

	typename QVector<T>::const_iterator Xi = lineA.constBegin();
	typename QVector<T>::const_iterator Yi = lineB.constBegin();

	// Connecting each segment of line A with max possible amount
	// of segments in line B.
	for (int i = 0; i < M; ++i)
	{
		PresentationGeometryImpl::bridgePointLine(*Xi, Yi, Yi + LhalfPlus + 1, vertices);
		Yi += LhalfPlus;
		// ----------------------
		vertices.push_back({ Xi->position, { }, { }, Xi->color });
		vertices.push_back({ Yi->position, { }, { }, Yi->color });
		++Xi;
		vertices.push_back({ Xi->position, { }, { }, Xi->color });
		// ----------------------
		PresentationGeometryImpl::bridgePointLine(*Xi, Yi, Yi + LhalfMinus + 1, vertices);
		Yi += LhalfMinus;
	}

	// Connecting the last point of the line A with the rest points 
	// of line B.  
	Q_ASSERT(Xi == lineA.constEnd() - 1);
	PresentationGeometryImpl::bridgePointLine(*Xi, Yi, lineB.constEnd(), vertices);
}

/*!
 * Connects a point and a line with triangles.
 * 
 * @param point The point to connect.
 * @param line The line to be connected with the point.
 * @param[out] vertices Output for generated vertices.
 */
template<typename T>
void PresentationGeometryImpl::bridgePointLine(const T& point,
											   const QVector<T>& line, 
											   QVector<ScVertexData>& vertices)
{
	PresentationGeometryImpl::bridgePointLine(point, line.constBegin(), line.constEnd(), vertices);
}
template<typename T>
void PresentationGeometryImpl::bridgePointLine(const T& point,
											   typename QVector<T>::const_iterator lineBegin, 
											   typename QVector<T>::const_iterator lineEnd, 
											   QVector<ScVertexData>& vertices)
{
	if (lineBegin == lineEnd)
	{
		return;
	}

    const QVector3D& Px = point.position;
	const QVector4D& Cx = point.color;
    auto Py = [&](int i) -> const QVector3D& { return (lineBegin + i)->position; };
	auto Cy = [&](int i) -> const QVector4D& { return (lineBegin + i)->color; };
	int N = lineEnd - lineBegin - 1;

	for (int i = 0; i < N; ++i)
    {
        QVector3D normal = QVector3D::normal(Px, Py(i), Py(i + 1));

        vertices.push_back({ Px, { 0.5f, 0.5f }, normal, Cx });
        vertices.push_back({ Py(i), { 0.5f, 0.5f  }, normal, Cy(i) });
        vertices.push_back({ Py(i + 1), { 0.5f, 0.5f }, normal, Cy(i + 1) });
	}
}
