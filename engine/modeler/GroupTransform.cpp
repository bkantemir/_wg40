#include "GroupTransform.h"
#include "utils.h"

extern float degrees2radians;

int GroupTransform::executeGroupTransform(ModelBuilder1base* pMB, GroupTransform* pGT) {
	flagSelection(pGT, &pMB->vertices, &pMB->triangles);
	transformFlagged(pGT, &pMB->vertices);
	return 1;
}
void GroupTransform::limit2mark(GroupTransform* pGT, std::string mark0) {
	std::string outStr;
	outStr.append("<" + mark0 + ">");
	myStrcpy_s(pGT->mark, 32, outStr.c_str());
}
int GroupTransform::flagSelection(GroupTransform* pGT, std::vector<Vertex01*>* pVertices, std::vector<Triangle01*>* pTriangles) {
	if (pVertices == NULL)
		return 0;
	bool checkSize = false;
	for (int i = 0; i < 3; i++) {
		if (pGT->pMin[i] > -1000000) {
			checkSize = true;
			break;
		}
		if (pGT->pMax[i] < 1000000) {
			checkSize = true;
			break;
		}
	}
	bool checkRads = false;
	for (int i = 0; i < 3; i++) {
		if (pGT->rMin[i] > 0) {
			checkRads = true;
			break;
		}
		if (pGT->rMax[i] < 1000000) {
			checkRads = true;
			break;
		}
	}
	int totalN = pVertices->size();
	for (int vN = 0; vN < totalN; vN++) {
		Vertex01* pV = pVertices->at(vN);
		if (pGT->pGroup != NULL) {
			if (vN < pGT->pGroup->fromVertexN) {
				pV->flag = -1;
				continue;
			}
		}
		if (strcmp(pGT->mark, "") != 0) {
			if (strstr(pV->marks, pGT->mark) == nullptr) {
				pV->flag = -1;
				continue;
			}
		}
		bool fits = true;
		if (checkSize) {
			for (int i = 0; i < 3; i++) {
				if (pV->aPos[i] < pGT->pMin[i]) {
					fits = false;
					break;
				}
				if (pV->aPos[i] > pGT->pMax[i]) {
					fits = false;
					break;
				}
			}
			if (!fits) {
				pV->flag = -1;
				continue;
			}
		}
		if (checkRads) {
			bool fits = true;
			for (int i = 0; i < 3; i++) {
				if (pGT->rMin[i] <= 0 && pGT->rMin[i] == 1000000)
					continue;
				float vRad = 0;
				for (int j = 0; j < 3; j++)
					if (j != i) {
						float d = pV->aPos[j] - pGT->refPoint[j];
						vRad += (d * d);
					}
				vRad = sqrtf(vRad);
				if (vRad < pGT->rMin[i]) {
					fits = false;
					break;
				}
				if (vRad > pGT->rMax[i]) {
					fits = false;
					break;
				}
			}
			if (!fits) {
				pV->flag = -1;
				continue;
			}
		}
		pV->flag = 0;
	}
	if (pTriangles == NULL)
		return 1;
	totalN = pTriangles->size();
	for (int tN = 0; tN < totalN; tN++) {
		Triangle01* pT = pTriangles->at(tN);
		if (pGT->pGroup != NULL) {
			if (tN < pGT->pGroup->fromTriangleN) {
				pT->flag = -1;
				continue;
			}
		}
		if (strcmp(pGT->mark, "") != 0) {
			if (strstr(pT->marks, pGT->mark) == nullptr) {
				pT->flag = -1;
				continue;
			}
		}
		pT->flag = 0;
	}
	return 1;
}

int GroupTransform::cloneFlagged(ModelBuilder1base* pMB,
	std::vector<Vertex01*>* pVxDst, std::vector<Triangle01*>* pTrDst,
	std::vector<Vertex01*>* pVxSrc, std::vector<Triangle01*>* pTrSrc) {

	int vertsNsrc = pVxSrc->size();
	int vertsN0dst = pVxDst->size();
	int trianglesNsrc = 0;
	int trianglesN0dst = 0;
	if (pTrSrc != NULL) {
		trianglesNsrc = pTrSrc->size();
		trianglesN0dst = pTrDst->size();
	}
	for (int i = 0; i < vertsNsrc; i++) {
		Vertex01* pV0 = pVxSrc->at(i);
		if (pV0->flag < 0)
			continue;
		pV0->altN = pVxDst->size();
		Vertex01* pV = new Vertex01(*pV0);
		pVxDst->push_back(pV);
		//overwrite marks
		if (pMB != NULL) {
			myStrcpy_s(pV->marks, 128, pMB->pCurrentGroup->marks);
			pV->subjN = pMB->usingSubjN;
		}
		pV->flag = -1;
	}
	if (pTrSrc != NULL) {
		//have triangles
		for (int i = 0; i < trianglesNsrc; i++) {
			Triangle01* pT0 = pTrSrc->at(i);
			if (pT0->flag < 0)
				continue;
			Triangle01* pT = new Triangle01(*pT0);
			pTrDst->push_back(pT);
			//overwrite marks
			if (pMB != NULL) {
				myStrcpy_s(pT->marks, 128, pMB->pCurrentGroup->marks);
				pT->subjN = pMB->usingSubjN;
			}
			pT->flag = -1;
		}
		refactorTriangles(pTrDst, trianglesN0dst, pVxSrc);
	}
	return 1;
}



int GroupTransform::refactorTriangles(std::vector<Triangle01*>* pTrDst, int trianglesN0dst, std::vector<Vertex01*>* pVxSrc){
	//re-factor triangles idx, adjusting triangles verts #s
	int trianglesNdst = pTrDst->size();
	for (int tN = trianglesN0dst; tN < trianglesNdst; tN++) {
		Triangle01* pT = pTrDst->at(tN);
		for (int i = 0; i < 3; i++) {
			int vN = pT->idx[i];
			Vertex01* pV = pVxSrc->at(vN);
			pT->idx[i] = pV->altN;
		}
	}
	return 1;
}
int GroupTransform::buildTransformMatrix(GroupTransform* pGT, mat4x4* pTransformMatrix, float rate) {
	//returns 0 if no transformation
	int hasTransform = 0;
	pGT->normalsToo = false;
	mat4x4_identity(*pTransformMatrix);
	if (strlen(pGT->onThe) != 0) {
		hasTransform = 1;
		pGT->normalsToo = true;
		if (strcmp(pGT->onThe,"back") == 0)
			mat4x4_rotate_Y(*pTransformMatrix, *pTransformMatrix, degrees2radians * 180);
		else if (strcmp(pGT->onThe,"left") == 0)
			mat4x4_rotate_Y(*pTransformMatrix, *pTransformMatrix, -degrees2radians * 90);
		else if (strcmp(pGT->onThe,"right") == 0)
			mat4x4_rotate_Y(*pTransformMatrix, *pTransformMatrix, degrees2radians * 90);
		else if (strcmp(pGT->onThe,"top") == 0) {
			mat4x4_rotate_Y(*pTransformMatrix, *pTransformMatrix, degrees2radians * 180);
			mat4x4_rotate_X(*pTransformMatrix, *pTransformMatrix, -degrees2radians * 90);
		}
		else if (strcmp(pGT->onThe,"bottom") == 0)
			mat4x4_rotate_X(*pTransformMatrix, *pTransformMatrix, degrees2radians * 90);
	}
	if (rate == 0)
		return hasTransform;

	if (!v3equals(pGT->shift, 0)) {
		hasTransform = 1;
		mat4x4_translate_in_place(*pTransformMatrix, pGT->shift[0] * rate, pGT->shift[1] * rate, pGT->shift[2] * rate);
	}
	if (!v3equals(pGT->spinDg, 0)) {
		hasTransform = 1;
		pGT->normalsToo = true;
		//rotation order: Z-X-Y
		if (pGT->spinDg[1] != 0)
			mat4x4_rotate_Y(*pTransformMatrix, *pTransformMatrix, degrees2radians * pGT->spinDg[1] * rate);
		if (pGT->spinDg[0] != 0)
			mat4x4_rotate_X(*pTransformMatrix, *pTransformMatrix, degrees2radians * pGT->spinDg[0] * rate);
		if (pGT->spinDg[2] != 0)
			mat4x4_rotate_Z(*pTransformMatrix, *pTransformMatrix, degrees2radians * pGT->spinDg[2] * rate);
	}
	if (!v3equals(pGT->scale, 1)) {
		hasTransform = 1;
		float outScale[3] = { 1,1,1 };
		if (rate == 1)
			v3copy(outScale, pGT->scale);
		else
			for (int i = 0; i < 3; i++)
				outScale[i] = 1.0 + (pGT->scale[i] - 1.0) * rate;
		mat4x4_scale_aniso(*pTransformMatrix, *pTransformMatrix, outScale[0], outScale[1], outScale[2]);
	}
	return hasTransform;
}
int GroupTransform::transformFlagged(GroupTransform* pGT, std::vector<Vertex01*>* pVx) {
	//moves and rotates vertex group
	//rotation angles are set in degrees
	mat4x4 transformMatrix;
	if (strlen(pGT->applyTo)==0) {
		if (buildTransformMatrix(pGT, &transformMatrix, 1) > 0)
			transformFlaggedMx(pVx, &transformMatrix,pGT->normalsToo);
	}
	else { //applyTo NOT empty
		transformFlaggedRated(pGT, pVx);
	}
	if (!v3equals(pGT->headZto, 0)) {
		float pitchRd = v3pitchRd(pGT->headZto);
		float yawRd = v3yawRd(pGT->headZto);
		mat4x4_identity(transformMatrix);
		//rotation order: Z-X-Y
		if (v3yawRd != 0)
			mat4x4_rotate_Y(transformMatrix, transformMatrix, yawRd);
		if (pitchRd != 0)
			mat4x4_rotate_X(transformMatrix, transformMatrix, pitchRd);
		transformFlaggedMx(pVx, &transformMatrix, true);
	}
	if (strlen(pGT->align) > 0) {
		buildBoundingBoxFlagged(pGT, pVx);
		float xyz[3] = { 0,0,0 };
		if (strstr(pGT->align,"left") != NULL)
			xyz[0] = -pGT->bbMin[0];
		else if (strstr(pGT->align,"right") != NULL)
			xyz[0] = -pGT->bbMax[0];
		if (strstr(pGT->align,"bottom") != NULL)
			xyz[1] = -pGT->bbMin[1];
		else if (strstr(pGT->align,"top") != NULL)
			xyz[1] = -pGT->bbMax[1];
		if (strstr(pGT->align,"back") != NULL)
			xyz[2] = -pGT->bbMin[2];
		else if (strstr(pGT->align,"front") != NULL)
			xyz[2] = -pGT->bbMax[2];
		mat4x4_translate(transformMatrix, xyz[0], xyz[1], xyz[2]);
		transformFlaggedMx(pVx, &transformMatrix,false);
	}
	return 1;
}


int GroupTransform::transformFlaggedMx(std::vector<Vertex01*>* pVx, mat4x4* pTransformMatrix, bool normalsToo) {
	//moves and rotates vertex group
	int vertsN = pVx->size();
	for (int i = 0; i < vertsN; i++) {
		Vertex01* pV = pVx->at(i);
		if (pV->flag < 0)
			continue;
		mat4x4_mul_vec4plus(pV->aPos, *pTransformMatrix, pV->aPos, 1);
		if (normalsToo) {
			mat4x4_mul_vec4plus(pV->aNormal, *pTransformMatrix, pV->aNormal, 0);
			vec3_norm(pV->aNormal, pV->aNormal);
		}
	}
	return 1;
}
void GroupTransform::flagAll(std::vector<Vertex01*>* pVx, std::vector<Triangle01*>* pTr) {
	//set flags
	for (int i = pVx->size() - 1; i >= 0; i--)
		pVx->at(i)->flag = 0;
	if(pTr != NULL)
	for (int i = pTr->size() - 1; i >= 0; i--)
		pTr->at(i)->flag = 0;
}
void GroupTransform::flagGroup(Group01* pGroup, std::vector<Vertex01*>* pVx, std::vector<Triangle01*>* pTr) {
	if (pGroup == NULL)
		return flagAll(pVx, pTr);
	//set flags
	for (int i = pVx->size() - 1; i >= 0; i--) {
		Vertex01* pV = pVx->at(i);
		if(i < pGroup->fromVertexN)
			pV->flag = -1;
		else
			pV->flag = 0;
	}
	if(pTr != NULL)
	for (int i = pTr->size() - 1; i >= 0; i--){
		Triangle01* pT = pTr->at(i);
		if (i < pGroup->fromTriangleN)
			pT->flag = -1;
		else
			pT->flag = 0;
	}
}
int GroupTransform::buildBoundingBoxFlagged(GroupTransform* pGT, std::vector<Vertex01*>* pVx) {
	//v3copy(pGT->bbMin, pVx->at(0)->aPos);
	//v3copy(pGT->bbMax, pVx->at(0)->aPos);
	v3set(pGT->bbMin, 1000000, 1000000, 1000000);
	v3set(pGT->bbMax, -1000000, -1000000, -1000000);
	//scan all flagged except #0
	for (int vN = pVx->size() - 1; vN >= 0; vN--) {
		Vertex01* pV = pVx->at(vN);
		if (pV->flag < 0)
			continue;
		for(int i=0;i<3;i++){
			if (pGT->bbMin[i] > pV->aPos[i])
				pGT->bbMin[i] = pV->aPos[i];
			if (pGT->bbMax[i] < pV->aPos[i])
				pGT->bbMax[i] = pV->aPos[i];
		}
	}
	for (int i = 0; i < 3; i++) {
		pGT->bbSize[i] = pGT->bbMax[i] - pGT->bbMin[i];
		pGT->bbMid[i] = (pGT->bbMax[i] + pGT->bbMin[i])/2;
	}
	return 1;
}

int GroupTransform::transformFlaggedRated(GroupTransform* pGT, std::vector<Vertex01*>* pVx) {
	//moves and rotates vertex group
	buildBoundingBoxFlagged(pGT, pVx);
	float ratePow =1.0f;
	if (strstr(pGT->applyTo,"pow2") != NULL)
		ratePow = 2.0f;
	else if (strstr(pGT->applyTo,"pow3") != NULL)
		ratePow = 3.0f;
	else if (strstr(pGT->applyTo,"root2") != NULL)
		ratePow = 0.5f;
	else if (strstr(pGT->applyTo,"root3") != NULL)
		ratePow = 0.33f;
	mat4x4 transformMatrix = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
	int vertsN = pVx->size();
	for (int i = 0; i < vertsN; i++) {
		Vertex01* pV = pVx->at(i);
		if (pV->flag < 0)
			continue;
		float rate = getTransformRate(pGT, pV, ratePow);
		buildTransformMatrix(pGT, &transformMatrix, rate);
		mat4x4_mul_vec4plus(pV->aPos, transformMatrix, pV->aPos, 1);
		//normal
		mat4x4_mul_vec4plus(pV->aNormal, transformMatrix, pV->aNormal, 0);
		vec3_norm(pV->aNormal, pV->aNormal);
	}
	return 1;
}
float GroupTransform::dist2rate(float dist, float ratePow) {
	float rate = 1.0 - abs(dist);
	if (rate == 1) return rate;
	if (rate == 0) return rate;
	if (ratePow != 1)
		return pow(rate, ratePow);
	return rate;
}
float GroupTransform::getTransformRate(GroupTransform* pGT, Vertex01* pV, float ratePow) {
	if (strlen(pGT->applyTo)==0)
		return 1;
	float rate = 1;
	if (pGT->bbSize[0] != 0) {
		if (strstr(pGT->applyTo,"left") != NULL)
			rate *= dist2rate((pV->aPos[0] - pGT->bbMin[0]) / pGT->bbSize[0], ratePow);
		else if (strstr(pGT->applyTo,"right") != NULL)
			rate *= dist2rate((pV->aPos[0] - pGT->bbMax[0]) / pGT->bbSize[0], ratePow);
		else if (strstr(pGT->applyTo,"midX") != NULL)
			rate *= dist2rate((pV->aPos[0] - pGT->bbMid[0]) / (pGT->bbSize[0] / 2), ratePow);
	}
	if (pGT->bbSize[1] != 0) {
		if (strstr(pGT->applyTo,"bottom") != NULL)
			rate *= dist2rate((pV->aPos[1] - pGT->bbMin[1]) / pGT->bbSize[1], ratePow);
		else if (strstr(pGT->applyTo,"top") != NULL)
			rate *= dist2rate((pV->aPos[1] - pGT->bbMax[1]) / pGT->bbSize[1], ratePow);
		else if (strstr(pGT->applyTo,"midY") != NULL)
			rate *= dist2rate((pV->aPos[1] - pGT->bbMid[1]) / (pGT->bbSize[1] / 2), ratePow);
	}
	if (pGT->bbSize[2] != 0) {
		if (strstr(pGT->applyTo,"back") != NULL)
			rate *= dist2rate((pV->aPos[2] - pGT->bbMin[2]) / pGT->bbSize[2], ratePow);
		else if (strstr(pGT->applyTo,"front") != NULL)
			rate *= dist2rate((pV->aPos[2] - pGT->bbMax[2]) / pGT->bbSize[2], ratePow);
		else if (strstr(pGT->applyTo,"midZ") != NULL)
			rate *= dist2rate((pV->aPos[2] - pGT->bbMid[2]) / (pGT->bbSize[2] / 2), ratePow);
	}
	if (strstr(pGT->applyTo,"only") != NULL)
		if (rate < 0.999f)
			rate = 0.0f;
		else
			rate = 1.0f;
	return rate;
}


int GroupTransform::invertFlagged(std::vector<Vertex01*>* pVx, std::vector<Triangle01*>* pTr) {
	invertFlaggedTriangles(pTr);
	//invert normals
	for (int vN = pVx->size() - 1; vN >= 0; vN--) {
		Vertex01* pV = pVx->at(vN);
		if (pV->flag < 0)
			continue;
		for (int i = 0; i < 3; i++)
			pV->aNormal[i] = -pV->aNormal[i];
	}
	return 1;
}
int GroupTransform::invertFlaggedTriangles(std::vector<Triangle01*>* pTr) {
	//invert triangles
	for (int tN = pTr->size() - 1; tN >= 0; tN--) {
		Triangle01* pT = pTr->at(tN);
		if (pT->flag < 0)
			continue;
		int i0 = pT->idx[0];
		pT->idx[0] = pT->idx[1];
		pT->idx[1] = i0;
	}
	return 1;
}
void GroupTransform::clear(GroupTransform* pGT) {
	GroupTransform gt00;
	memcpy(pGT, &gt00, sizeof(GroupTransform));
}