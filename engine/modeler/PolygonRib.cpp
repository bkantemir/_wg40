#include "PolygonRib.h"

PolygonRib::PolygonRib(std::vector<Vertex01*>* pVxSrc, int idx0) {
	bool shortLine = (pVxSrc->size() == 2);
	//2 points
	i0 = idx0;
	p0 = pVxSrc->at(idx0)->aPos;
	int ribsN = pVxSrc->size();
	int idx1 = (idx0 + 1) % ribsN;
	i1 = idx1;
	p1 = pVxSrc->at(idx1)->aPos;
	//3-rd "inner" ref point
	float* p2 = NULL;
	if (!shortLine) {
		int idx2 = (idx0 + 2) % ribsN;
		p2 = pVxSrc->at(idx2)->aPos;
	}
	//find line equation
	if (p0[0] == p1[0]) {
		isVertical = true;
		x_vertical = p0[0];
		//"inner" side
		if (!shortLine) {
			if (p2[0] < x_vertical)
				xDirIn = -1;
			else
				xDirIn = 1;
		}
	}
	else if (p0[1] == p1[1]) {
		isHorizontal = true;
		a_slope = 0;
		b_intercept = p0[1];
		//"inner" side
		if (!shortLine) {
			if (p2[1] < b_intercept)
				yDirIn = -1;
			else
				yDirIn = 1;
		}
	}
	else {
		a_slope = (p1[1]-p0[1]) / (p1[0] - p0[0]);
		b_intercept = p0[1] - a_slope * p0[0];
		//"inner" side
		if (!shortLine) {
			float y = a_slope * p2[0] + b_intercept;
			if (p2[1] < y)
				yDirIn = -1;
			else
				yDirIn = 1;
			float x = (p2[1] - b_intercept) / a_slope;
			if (p2[0] < x)
				xDirIn = -1;
			else
				xDirIn = 1;
		}
	}
	return;
}
bool PolygonRib::matchingLines(PolygonRib* pRibFrame, PolygonRib* pRibSrc) {
	if (!parallelLines(pRibFrame, pRibSrc))
		return false;
	if (pRibFrame->b_intercept != pRibSrc->b_intercept)
		return false;
	if (pRibFrame->x_vertical != pRibSrc->x_vertical)
		return false;
	return true;
}
bool PolygonRib::parallelLines(PolygonRib* pRibFrame, PolygonRib* pRibSrc) {
	if (pRibFrame->isVertical != pRibSrc->isVertical)
		return false;
	if (pRibFrame->a_slope != pRibSrc->a_slope)
		return false;
	return true;
}
