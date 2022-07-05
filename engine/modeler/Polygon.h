#pragma once
#include "Vertex01.h"
#include "Triangle01.h"
#include "PolygonRib.h"
#include <vector>

class Polygon
{
public:
	std::vector<Vertex01*> vertices;
	std::vector<PolygonRib*> ribs;
	std::vector<Triangle01*> triangles;
	float normal[3] = {0,0,0};
	int ribsN = 0;
	//bounding box
	float bbMin[3] = { 0,0,0 };
	float bbMax[3] = { 0,0,0 };
public:
	virtual ~Polygon() { clearAll(this); };
	static void clearAll(Polygon* pPL);
	static int addVertex(Polygon* pPL, Vertex01* pV);
	static int addVertex(Polygon* pPL, float x, float y, float z);
	static int finalizePolygon(Polygon* pPL);
	static int finalizeLinePolygon(Polygon* pPL);
	static int setTriangle(Polygon* pPL, Triangle01* pT, std::vector<Vertex01*>* pVxSrc);
	static int setRectangle(Polygon* pPL, float w, float h);
	static int xyIntersection(Polygon* pDst, Polygon* pFrame, Polygon* pSrc);
	static int xy2pointsLineIntersection(Polygon* pDst, Polygon* pFrame, Polygon* pSrc);
	static int xyPointProjection(Polygon* pDst, Vertex01* pV, Polygon* pSrc);
	static int addLinesIntersection(Polygon* pDst, Polygon* pFrame, int ribNframe, Polygon* pSrc, int ribNsrc);

	static bool dotFits(float* p0, PolygonRib* pPR);
	static bool dotFits(float* p0, Polygon* pPL);
	static bool correctSide(float* p0, PolygonRib* pPR);
	static int addVert(Polygon* pDst, float* p0, Vertex01* pV0, Vertex01* pV1);
	static int addVert(Polygon * pDst, float* p0, Polygon * pSrc);
	static int buildTriangles(Polygon* pPL);
};

