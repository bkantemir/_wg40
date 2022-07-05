#include "Polygon.h"
#include "linmath.h"
#include "utils.h"
#include "platform.h"
#include <algorithm>

extern float radians2degrees;

void Polygon::clearAll(Polygon* pPL) {
	for (int i = pPL->vertices.size() - 1; i >= 0; i--)
		delete pPL->vertices.at(i);
	pPL->vertices.clear();

	for (int i = pPL->ribs.size() - 1; i >= 0; i--)
		delete pPL->ribs.at(i);
	pPL->ribs.clear();

	for (int i = pPL->triangles.size() - 1; i >= 0; i--)
		delete pPL->triangles.at(i);
	pPL->triangles.clear();
}
int Polygon::addVertex(Polygon* pPL, float x, float y, float z) {
	Vertex01* pV = new Vertex01();
	pV->aPos[0] = x;
	pV->aPos[1] = y;
	pV->aPos[2] = z;
	pPL->vertices.push_back(pV);
	return 1;
}
int Polygon::addVertex(Polygon* pPL, Vertex01* pV0) {
	Vertex01* pV = new Vertex01(*pV0);
	pPL->vertices.push_back(pV);
//mylog("====Adding vertexs %f x %f\n",pV->aPos[0], pV->aPos[1]);
	return 1;
}
int Polygon::setTriangle(Polygon* pPL, Triangle01* pT, std::vector<Vertex01*>* pVxSrc) {
	clearAll(pPL);
	for (int i = 0; i < 3; i++) {
		int vN = pT->idx[i];
		Vertex01* pV = pVxSrc->at(vN);
		addVertex(pPL, pV);
	}
	finalizePolygon(pPL);
	return 1;
}
int Polygon::setRectangle(Polygon* pPL, float w, float h) {
	clearAll(pPL);
	w /= 2;
	h /= 2;
	//CCW
	addVertex(pPL, -w,  h, 0); //NW
	addVertex(pPL, -w, -h, 0); //SW
	addVertex(pPL,  w, -h, 0); //SE
	addVertex(pPL,  w,  h, 0); //NE
	finalizePolygon(pPL);
	return 1;
}
int Polygon::finalizePolygon(Polygon* pPL) {
	pPL->ribsN = pPL->vertices.size();
	for (int i = 0; i < pPL->ribsN; i++) {
		PolygonRib* pPR = new PolygonRib(&pPL->vertices,i);
		pPL->ribs.push_back(pPR);
	}
	//calculate polygon's normal
	float v0[3];
	float v2[3];
	for (int i = 0; i < 3; i++) {
		v0[i] = pPL->vertices.at(1)->aPos[i] - pPL->vertices.at(0)->aPos[i];
		v2[i] = pPL->vertices.at(2)->aPos[i] - pPL->vertices.at(0)->aPos[i];
	}
	vec3_mul_cross(pPL->normal, v0, v2);
	vec3_norm(pPL->normal, pPL->normal);
	//bounding box
	Vertex01* pV = pPL->vertices.at(0);
	v3copy(pPL->bbMin, pV->aPos);
	v3copy(pPL->bbMax, pV->aPos);
	for (int vN = pPL->vertices.size() - 1; vN >= 1; vN--) {
		pV = pPL->vertices.at(vN);
		for (int i = 0; i < 3; i++) {
			if (pPL->bbMin[i] > pV->aPos[i])
				pPL->bbMin[i] = pV->aPos[i];
			if (pPL->bbMax[i] < pV->aPos[i])
				pPL->bbMax[i] = pV->aPos[i];
		}
	}
	return 1;
}

int Polygon::finalizeLinePolygon(Polygon* pPL) {
	pPL->ribsN = pPL->vertices.size()-1;
	for (int i = 0; i < pPL->ribsN; i++) {
		PolygonRib* pPR = new PolygonRib(&pPL->vertices, i);
		pPL->ribs.push_back(pPR);
	}
	//bounding box
	Vertex01* pV = pPL->vertices.at(0);
	v3copy(pPL->bbMin, pV->aPos);
	v3copy(pPL->bbMax, pV->aPos);
	for (int vN = pPL->vertices.size() - 1; vN >= 1; vN--) {
		pV = pPL->vertices.at(vN);
		for (int i = 0; i < 3; i++) {
			if (pPL->bbMin[i] > pV->aPos[i])
				pPL->bbMin[i] = pV->aPos[i];
			if (pPL->bbMax[i] < pV->aPos[i])
				pPL->bbMax[i] = pV->aPos[i];
		}
	}
	return 1;
}

int Polygon::addLinesIntersection(Polygon* pDst, Polygon* pFrame, int ribNframe, Polygon* pSrc, int ribNsrc) {
	PolygonRib* pRibFrame = pFrame->ribs.at(ribNframe);
	PolygonRib* pRibSrc = pSrc->ribs.at(ribNsrc);
	/*
mylog("==addLinesIntersection\n");
mylog("  fr %f x %f to %f x %f v=%d h=%d\n", pRibFrame->p0[0], pRibFrame->p0[1], pRibFrame->p1[0], pRibFrame->p1[1], pRibFrame->isVertical, pRibFrame->isHorizontal);
mylog("  tr %f x %f to %f x %f v=%d h=%d\n", pRibSrc->p0[0], pRibSrc->p0[1], pRibSrc->p1[0], pRibSrc->p1[1], pRibSrc->isVertical, pRibSrc->isHorizontal);
*/
	if (PolygonRib::matchingLines(pRibFrame, pRibSrc)) {
		Vertex01* pV0 = pSrc->vertices.at(pRibSrc->i0);
		Vertex01* pV1 = pSrc->vertices.at(pRibSrc->i1);
		int dstVertsN0 = pDst->vertices.size();
		if (dotFits(pRibFrame->p0, pRibSrc))
			addVert(pDst, pRibFrame->p0, pV0,pV1);
		if (dotFits(pRibFrame->p1, pRibSrc))
			addVert(pDst, pRibFrame->p1, pV0, pV1);
		if (dotFits(pRibSrc->p0, pRibFrame))
			addVertex(pDst, pV0);
		if (dotFits(pRibSrc->p1, pRibFrame))
			addVertex(pDst, pV1);
//mylog("  lines are identical\n");
		return pDst->vertices.size()- dstVertsN0;
	}
	if (PolygonRib::parallelLines(pRibFrame, pRibSrc)) {
//mylog("  lines are parallel\n");
		return 0;
	}
	//find lines intersection, assuming lines are not parallel
	float x,y;
	if (pRibFrame->isVertical) {
		x = pRibFrame->p0[0];
		y = pRibSrc->a_slope * x + pRibSrc->b_intercept;
	}
	else { //pRibFrame not vertical
		if (pRibSrc->isVertical) {
			x = pRibSrc->p0[0];
			y = pRibFrame->a_slope * x + pRibFrame->b_intercept;
		}
		else { //both lines are "normal"
			x = (pRibSrc->b_intercept - pRibFrame->b_intercept) / (pRibFrame->a_slope - pRibSrc->a_slope);
			y = pRibFrame->a_slope * x + pRibFrame->b_intercept;
		}
	}
	//check if belongs to both PolygonRibs
	float xy[2];
	xy[0] = x;
	xy[1] = y;
	if (!dotFits(xy, pRibFrame))
		return 0;
	if (!dotFits(xy, pRibSrc))
		return 0;
	addVert(pDst, xy, pSrc->vertices.at(pRibSrc->i0), pSrc->vertices.at(pRibSrc->i1));
	return 1;
}
bool Polygon::correctSide(float* p0, PolygonRib* pPR) {
	if (pPR->isVertical)
		if ((p0[0] - pPR->x_vertical) * pPR->xDirIn < 0)
			return false;
	if (pPR->isHorizontal)
		if ((p0[1] - pPR->b_intercept) * pPR->yDirIn < 0)
			return false;
	float y = pPR->a_slope * p0[0] + pPR->b_intercept;
	if ((p0[1] - y) * pPR->yDirIn < 0)
		return false;
	return true;
}
int Polygon::addVert(Polygon* pDst, float* p0, Vertex01* pV0, Vertex01* pV1) {
	float d[2];
	for (int i = 0; i < 2; i++)
		d[i] = pV0->aPos[i] - p0[i];
	float dist2v0 = v3lengthXY(d);
	if (dist2v0 == 0)
		return addVertex(pDst, pV0);
	for (int i = 0; i < 2; i++)
		d[i] = pV1->aPos[i] - p0[i];
	float dist2v1 = v3lengthXY(d);
	if (dist2v1 == 0)
		return addVertex(pDst, pV1);
	//if here - find mid values
	float k0 = dist2v1 / (dist2v0 + dist2v1);
	float k1 = dist2v0 / (dist2v0 + dist2v1);
	Vertex01* pVx = new Vertex01(*pV0);
	pVx->aPos[0] = p0[0];
	pVx->aPos[1] = p0[1];
	pVx->aPos[2] = k0 * pV0->aPos[2] + k1 * pV1->aPos[2];
	for (int i = 0; i < 3; i++)
		pVx->aNormal[i] = k0 * pV0->aNormal[i] + k1 * pV1->aNormal[i];
	for (int i = 0; i < 2; i++)
		pVx->aTuv[i] = k0 * pV0->aTuv[i] + k1 * pV1->aTuv[i];
	for (int i = 0; i < 2; i++)
		pVx->aTuv2[i] = k0 * pV0->aTuv2[i] + k1 * pV1->aTuv2[i];
	addVertex(pDst, pVx);
	delete pVx;
	return 0;
}
int Polygon::xyIntersection(Polygon* pDst, Polygon* pFrame, Polygon* pSrc) {
	//check bounding boxes, XY only
	for (int i = 0; i < 2; i++) {
		if (pFrame->bbMin[i] > pSrc->bbMax[i])
			return 0;
		if (pFrame->bbMax[i] < pSrc->bbMin[i])
			return 0;
	}
	//compare normals
	if (v3dotProduct(pFrame->normal, pSrc->normal) <= 0)
		return 0;
/*
mylog(">>>pFrame %fx%f to %fx%f to %fx%f \n",
	pFrame->vertices.at(0)->aPos[0], pFrame->vertices.at(0)->aPos[1],
	pFrame->vertices.at(1)->aPos[0], pFrame->vertices.at(1)->aPos[1],
	pFrame->vertices.at(2)->aPos[0], pFrame->vertices.at(2)->aPos[1]
);
mylog("   pSrc   %fx%f to %fx%f to %fx%f \n",
	pSrc->vertices.at(0)->aPos[0], pSrc->vertices.at(0)->aPos[1],
	pSrc->vertices.at(1)->aPos[0], pSrc->vertices.at(1)->aPos[1],
	pSrc->vertices.at(2)->aPos[0], pSrc->vertices.at(2)->aPos[1]
);
mylog("---SrcVerts\n");
*/
	//if here - have overlap
	int addedSrcVerts = 0;
	for (int vN = 0; vN < pSrc->ribsN; vN++) {
		Vertex01* pV = pSrc->vertices.at(vN);
		if (dotFits(pV->aPos, pFrame))
			addedSrcVerts += addVertex(pDst, pV);
	}
	if (addedSrcVerts == pSrc->ribsN)
		return addedSrcVerts;

//mylog("---FrameVerts\n");
	int addedFrameVerts = 0;
	for (int vN = 0; vN < pFrame->ribsN; vN++) {
		Vertex01* pV = pFrame->vertices.at(vN);
		if (dotFits(pV->aPos, pSrc)) {
			int frameVerts = addVert(pDst, pV->aPos, pSrc);
			addedFrameVerts += frameVerts;
		}
	}
	if (addedFrameVerts == pFrame->ribsN)
		return addedFrameVerts;

//mylog("---CrossVerts\n");
	//check ribs intersections
	int addedCrossVerts = 0;
	for (int ribNframe = 0; ribNframe < pFrame->ribsN; ribNframe++) {
		for (int ribNsrc = 0; ribNsrc < pSrc->ribsN; ribNsrc++) {
			int crossVerts = addLinesIntersection(pDst, pFrame, ribNframe, pSrc, ribNsrc);
			addedCrossVerts += crossVerts;
		}
	}
	return (addedSrcVerts + addedFrameVerts + addedCrossVerts);
}
bool Polygon::dotFits(float* p0, PolygonRib* pPR) {
//mylog("dotFits Rib %f x %f vs %f x %f to %f x %f\n", p0[0], p0[1], pPR->p0[0], pPR->p0[1], pPR->p1[0], pPR->p1[1]);
	//assuming that p0 is on the line
	int dir0;
	int dir1;
	if (pPR->isVertical) {
		if (pPR->p0[1] == p0[1])
			return true;
		else if (pPR->p0[1] < p0[1])
			dir0 = -1;
		else
			dir0 = 1;

		if (pPR->p1[1] == p0[1])
			return true;
		else if (pPR->p1[1] < p0[1])
			dir1 = -1;
		else
			dir1 = 1;
	}
	else{ //"normal" line
		if (pPR->p0[0] == p0[0])
			return true;
		else if (pPR->p0[0] < p0[0])
			dir0 = -1;
		else
			dir0 = 1;

		if (pPR->p1[0] == p0[0])
			return true;
		else if (pPR->p1[0] < p0[0])
			dir1 = -1;
		else
			dir1 = 1;
	}
//mylog("  fits?=%d\n", !(dir0 == dir1));
	if (dir0 == dir1)
		return false;
	return true;
}
bool Polygon::dotFits(float* p0, Polygon* pPL) {
//mylog("dotFits Polygon %f x %f\n",p0[0],p0[1]);

	for (int i = 0; i < pPL->ribsN; i++)
		if (!correctSide(p0, pPL->ribs.at(i))) {
//mylog("  don't Fit side %f x %f to %f x %f\n", pPL->ribs.at(i)->p0[0], pPL->ribs.at(i)->p0[1], pPL->ribs.at(i)->p1[0], pPL->ribs.at(i)->p1[1]);
			return false;
		}
//mylog("  dotFits!\n");
	return true;
}
int Polygon::buildTriangles(Polygon* pPL) {
	int vertsN = pPL->vertices.size();
	//mid point coords
	float p0[2] = { 0,0 };
	for (int vN = 0; vN < vertsN; vN++) {
		float* p1 = pPL->vertices.at(vN)->aPos;
		p0[0] += p1[0];
		p0[1] += p1[1];
	}
	p0[0] /= vertsN;
	p0[1] /= vertsN;
	for (int vN = 0; vN < vertsN; vN++) {
		float* p1 = pPL->vertices.at(vN)->aPos;
		float v1[3] ={0,0,0};
		v1[0] = p1[0] - p0[0];
		v1[1] = p1[1] - p0[1];
		float az = -atan2f(v1[0], v1[1]) * radians2degrees;
		//aTangent is not used at this point, ok to use it to store az
		pPL->vertices.at(vN)->aTangent[0] = az;
	}
	//sort vertices
	std::sort(pPL->vertices.begin(), pPL->vertices.end(), 
		[](Vertex01* pV0, Vertex01* pV1) {
		return pV0->aTangent[0] > pV1->aTangent[0]; });
	//check for redundancy
	for (int vN = pPL->vertices.size() - 1; vN > 0; vN--) {
		Vertex01* pV = pPL->vertices.at(vN);
		Vertex01* pVprev = pPL->vertices.at(vN-1);
		if (pV->aTangent[0] == pVprev->aTangent[0]) {
			delete pV;
			pPL->vertices.erase(pPL->vertices.begin() + vN);
		}
	}
	pPL->ribsN = pPL->vertices.size();
	//build triangles
	Vertex01* pV = pPL->vertices.at(0);
	for (int vN = 2; vN < pPL->ribsN; vN++) {
		Triangle01* pTR = new Triangle01();
		pPL->triangles.push_back(pTR);
		pTR->idx[0] = 0;
		pTR->idx[1] = vN;
		pTR->idx[2] = vN - 1;
		pTR->subjN = pV->subjN;
		pTR->materialN = pV->materialN;
		//mark
		myStrcpy_s(pTR->marks, 128, pV->marks);
	}
	return 1;
}
int Polygon::addVert(Polygon* pDst, float* p0, Polygon* pSrc) {
	//check where horizontal line drawn through p0 crosses polygon's ribs
	Vertex01 vx0;
	Vertex01 vx1;
	int vxN = 0;
	for (int ribN = 0; ribN < pSrc->ribsN; ribN++) {
		PolygonRib* pPR = pSrc->ribs.at(ribN);
		if (pPR->isHorizontal)
			continue;
		float p1[2];
		p1[1] = p0[1];
		if (pPR->isVertical)
			p1[0] = pPR->x_vertical;
		else
			p1[0] = (p1[1] - pPR->b_intercept) / pPR->a_slope;
		if (!dotFits(p1, pPR))
			continue;
		//if here - 1 intersection found
		Vertex01* pVdst = &vx0;
		if(vxN > 0)
			pVdst = &vx1;
		Vertex01* pV0src = pSrc->vertices.at(pPR->i0);
		Vertex01* pV1src = pSrc->vertices.at(pPR->i1);
		memcpy(pVdst, pV0src, sizeof(Vertex01));
		float d[2];
		for (int i = 0; i < 2; i++)
			d[i] = pV0src->aPos[i] - p1[i];
		float dist2v0 = v3lengthXY(d);
		if (dist2v0 == 0)
			memcpy(pVdst, pV0src, sizeof(Vertex01));
		else {
			for (int i = 0; i < 2; i++)
				d[i] = pV1src->aPos[i] - p1[i];
			float dist2v1 = v3lengthXY(d);
			if (dist2v1 == 0)
				memcpy(pVdst, pV1src, sizeof(Vertex01));
			else {
				//if here - find mid values
				float k0 = dist2v1 / (dist2v0 + dist2v1);
				float k1 = dist2v0 / (dist2v0 + dist2v1);
				pVdst->aPos[0] = p1[0];
				pVdst->aPos[1] = p1[1];
				pVdst->aPos[2] = k0 * pV0src->aPos[2] + k1 * pV1src->aPos[2];
				for (int i = 0; i < 3; i++)
					pVdst->aNormal[i] = k0 * pV0src->aNormal[i] + k1 * pV1src->aNormal[i];
				for (int i = 0; i < 2; i++)
					pVdst->aTuv[i] = k0 * pV0src->aTuv[i] + k1 * pV1src->aTuv[i];
				for (int i = 0; i < 2; i++)
					pVdst->aTuv2[i] = k0 * pV0src->aTuv2[i] + k1 * pV1src->aTuv2[i];
			}
		}
		vxN++;
		if (vxN > 1)
			break;
	}
	addVert(pDst, p0, &vx0, &vx1);
	return 1;
}
int Polygon::xy2pointsLineIntersection(Polygon* pDst, Polygon* pLine, Polygon* pSrc) {
	//check bounding boxes, XY only
	for (int i = 0; i < 2; i++) {
		if (pLine->bbMin[i] > pSrc->bbMax[i])
			return 0;
		if (pLine->bbMax[i] < pSrc->bbMin[i])
			return 0;
	}
	//check if triangle faces us
	if (pSrc->normal[2] <= 0)
		return 0;
	//if here - have overlap
				//mylog("---FrameVerts\n");
	int addedLineVerts = 0;
	for (int vN = 0; vN < 2; vN++) {
		Vertex01* pV = pLine->vertices.at(vN);
		if (dotFits(pV->aPos, pSrc)) {
			int frameVerts = addVert(pDst, pV->aPos, pSrc);
			addedLineVerts += frameVerts;
		}
	}
	if (addedLineVerts == 2)
		return addedLineVerts;

	//mylog("---CrossVerts\n");
		//check ribs intersections
	int addedCrossVerts = 0;
	for (int ribNsrc = 0; ribNsrc < pSrc->ribsN; ribNsrc++) {
		int crossVerts = addLinesIntersection(pDst, pLine, 0, pSrc, ribNsrc);
		addedCrossVerts += crossVerts;
	}

	return (addedLineVerts + addedCrossVerts);
}
int Polygon::xyPointProjection(Polygon* pDst, Vertex01* pV, Polygon* pSrc) {
	//check bounding boxes, XY only
	for (int i = 0; i < 2; i++) {
		if (pV->aPos[i] > pSrc->bbMax[i])
			return 0;
		if (pV->aPos[i] < pSrc->bbMin[i])
			return 0;
	}
	//check if triangle faces us
	if (pSrc->normal[2] <= 0)
		return 0;
	//if here - have overlap
	if (dotFits(pV->aPos, pSrc)) {
		addVert(pDst, pV->aPos, pSrc);
		return 1;
	}
	return 0;
}
