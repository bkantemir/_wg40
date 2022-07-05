#pragma once
#include "Vertex01.h"
#include <vector>

class PolygonRib
{
public:
	int i0;
	int i1;
	float* p0; //rib p0
	float* p1; //rib p1
	//line equation
	float a_slope = 0; //a
	float b_intercept = 0; //b
	bool isVertical = false;
	float x_vertical = 0;
	bool isHorizontal = false;
	//direction to "inner" side
	float xDirIn = 0;
	float yDirIn = 0;
public:
	PolygonRib(std::vector<Vertex01*>* pVxSrc, int idx0);
	static bool matchingLines(PolygonRib* pRibFrame, PolygonRib* pRibSrc);
	static bool parallelLines(PolygonRib* pRibFrame, PolygonRib* pRibSrc);
};


