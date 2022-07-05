#include "ModelBuilder1base.h"
#include "platform.h"
#include "utils.h"
#include "DrawJob.h"
#include "Shader.h"

extern float degrees2radians;

ModelBuilder1base::~ModelBuilder1base() {
	releaseGroup(this);

	//clear all vectors
	int itemsN = vertices.size();
	for (int i = 0; i < itemsN; i++)
		delete vertices.at(i);
	vertices.clear();

	itemsN = triangles.size();
	for (int i = 0; i < itemsN; i++)
		delete triangles.at(i);
	triangles.clear();

	itemsN = vShapesStack.size();
	for (int i = 0; i < itemsN; i++)
		delete vShapesStack.at(i);
	vShapesStack.clear();

	itemsN = groupsStack.size();
	for (int i = 0; i < itemsN; i++)
		delete groupsStack.at(i);
	groupsStack.clear();
	if (pCurrentGroup != NULL)
		delete pCurrentGroup;
	if (pLastClosedGroup != NULL)
		delete pLastClosedGroup;

	itemsN = materialsList.size();
	for (int i = 0; i < itemsN; i++)
		delete materialsList.at(i);
	materialsList.clear();

	itemsN = materialAdjustsList.size();
	for (int i = 0; i < itemsN; i++)
		delete materialAdjustsList.at(i);
	materialAdjustsList.clear();

	itemsN = colorsList.size();
	for (int i = 0; i < itemsN; i++)
		delete colorsList.at(i);
	colorsList.clear();

	subjNumbersList.clear();
}
int ModelBuilder1base::useSubjN(ModelBuilder1base* pMB, int subjN) {
	pMB->usingSubjN = subjN;
	int itemsN = pMB->subjNumbersList.size();
	bool newN = true;
	if (itemsN > 0)
		for (int i = 0; i < itemsN; i++)
			if (pMB->subjNumbersList.at(i) == subjN) {
				newN = false;
				break;
			}
	if (newN)
		pMB->subjNumbersList.push_back(subjN);
	return subjN;
}
int ModelBuilder1base::getMaterialN(ModelBuilder1base* pMB, Material* pMT) {
	int itemsN = pMB->materialsList.size();
	if (itemsN > 0)
		for (int i = 0; i < itemsN; i++)
			if (memcmp(pMB->materialsList.at(i), pMT, sizeof(Material)) == 0) {
				return i;
			}
	//if here - add new material to the list
	Material* pMTnew = new Material(*pMT);
	pMB->materialsList.push_back(pMTnew);
	return itemsN;
}
int ModelBuilder1base::add2triangles(ModelBuilder1base* pMB, int nNW, int nNE, int nSW, int nSE, int n) {
	//indexes: NorthWest, NorthEast, SouthWest,SouthEast
	if (n % 2 == 0) { //even number
		addTriangle(pMB, nNW, nSW, nNE);
		addTriangle(pMB, nNE, nSW, nSE);
	}
	else { //odd number
		addTriangle(pMB, nNW, nSE, nNE);
		addTriangle(pMB, nNW, nSW, nSE);
	}
	return pMB->triangles.size() - 1;
}
int ModelBuilder1base::addTriangle(ModelBuilder1base* pMB, int i0, int i1, int i2) {
	Triangle01* pTR = new Triangle01();
	pMB->triangles.push_back(pTR);
	pTR->idx[0] = i0;
	pTR->idx[1] = i1;
	pTR->idx[2] = i2;
	pTR->subjN = pMB->usingSubjN;
	pTR->materialN = pMB->usingMaterialN;
	//mark
	if (pMB->pCurrentGroup != NULL)
		if (strcmp(pMB->pCurrentGroup->marks, "") != 0)
			myStrcpy_s(pTR->marks, 128, pMB->pCurrentGroup->marks);
	return pMB->triangles.size() - 1;
}
int ModelBuilder1base::addVertex(ModelBuilder1base* pMB, float kx, float ky, float kz, float nx, float ny, float nz) {
	Vertex01* pVX = new Vertex01();
	pMB->vertices.push_back(pVX);
	pVX->aPos[0] = kx;
	pVX->aPos[1] = ky;
	pVX->aPos[2] = kz;
	//normal
	pVX->aNormal[0] = nx;
	pVX->aNormal[1] = ny;
	pVX->aNormal[2] = nz;
	pVX->subjN = pMB->usingSubjN;
	pVX->materialN = pMB->usingMaterialN;
	//mark
	if (pMB->pCurrentGroup != NULL)
		if (strcmp(pMB->pCurrentGroup->marks, "") != 0)
			myStrcpy_s(pVX->marks, 128, pMB->pCurrentGroup->marks);
	return pMB->vertices.size() - 1;
}
int ModelBuilder1base::buildDrawJobs(ModelBuilder1base* pMB, std::vector<GameSubj*>* pGameSubjs) {
	//debug_showNormals(pMB);

	int totalSubjsN = pMB->subjNumbersList.size();
	if (totalSubjsN < 1) {
		pMB->subjNumbersList.push_back(-1);
		totalSubjsN = 1;
	}
	int totalMaterialsN = pMB->materialsList.size();
	if (totalSubjsN < 2 && totalMaterialsN < 2) {
		//simple single DrawJob
		Material* pMT = pMB->materialsList.at(0);
		GameSubj* pGS = NULL;
		int gsN = pMB->subjNumbersList.at(0);
		if (gsN >= 0)
			pGS = pGameSubjs->at(gsN);
		if (pGS != NULL)
			pGS->djStartN = DrawJob::drawJobs.size();
		buildSingleDrawJob(pMT, &pMB->vertices, &pMB->triangles);
		if (pGS != NULL) {
			pGS->djTotalN = DrawJob::drawJobs.size() - pGS->djStartN;
			//save prime material
			memcpy(&pGS->mt0, &DrawJob::drawJobs.at(pGS->djStartN)->mt,sizeof(Material));
			pGS->mt0isSet = 1;
		}
		return 1;
	}
	int totalVertsN = pMB->vertices.size(); 
	int totalTrianglesN = pMB->triangles.size();
	//clear flags
	for (int vN = 0; vN < totalVertsN; vN++) {
		Vertex01* pVX = pMB->vertices.at(vN);
		pVX->flag = 0;
	}
	for (int tN = 0; tN < totalTrianglesN; tN++) {
		Triangle01* pTR = pMB->triangles.at(tN);
		pTR->flag = 0;
	}
	int addedDJs = 0;
	for (int sN = 0; sN < totalSubjsN; sN++) {
		GameSubj* pGS = NULL;
		int gsN = pMB->subjNumbersList.at(sN);
		if (gsN >= 0)
			pGS = pGameSubjs->at(gsN);
		if (pGS != NULL)
			pGS->djStartN = DrawJob::drawJobs.size();
		for (int mtN = 0; mtN < totalMaterialsN; mtN++) {
			Material* pMT = pMB->materialsList.at(mtN);
			std::vector<Vertex01*> useVertices;
			std::vector<Triangle01*> useTriangles;
			for (int vN = 0; vN < totalVertsN; vN++) {
				Vertex01* pVX = pMB->vertices.at(vN);
				if (pVX->flag != 0)
					continue;
				if (pVX->subjN != gsN)
					continue;
				if (pVX->materialN != mtN)
					continue;
				//if here - make a copy
				pVX->altN = useVertices.size();
				Vertex01* pVX2 = new Vertex01(*pVX);
				useVertices.push_back(pVX2);
				pVX->flag = 1;
				if (pVX->endOfSequence > 0) {
					//rearrangeArraysForDrawJob(pMB, pMB->vertices, useVertices, useTriangles);
					buildSingleDrawJob(pMT, &useVertices, &useTriangles);
					addedDJs++;
					//clear and proceed to next sequence
					int useVerticesN = useVertices.size();
					for (int i = 0; i < useVerticesN; i++)
						delete useVertices.at(i);
					useVertices.clear();
				}
			}
			int useVerticesN = useVertices.size();
			if (useVerticesN < 1)
				continue; //to next material
			//pick triangles
			for (int tN = 0; tN < totalTrianglesN; tN++) {
				Triangle01* pTR = pMB->triangles.at(tN);
				if (pTR->flag != 0)
					continue;
				if (pTR->subjN != gsN)
					continue;
				if (pTR->materialN != mtN)
					continue;
				//if here - make a copy
				Triangle01* pTR2 = new Triangle01(*pTR);
				useTriangles.push_back(pTR2);
				pTR->flag = 1;
			}
			rearrangeArraysForDrawJob(&pMB->vertices, &useVertices, &useTriangles);
			buildSingleDrawJob(pMT, &useVertices, &useTriangles);
			useVerticesN = useVertices.size();
			addedDJs++;
			//clear all for next material
			for (int i = 0; i < useVerticesN; i++)
				delete useVertices.at(i);
			useVertices.clear();
			int useTrianglesN = useTriangles.size();
			for (int i = 0; i < useTrianglesN; i++)
				delete useTriangles.at(i);
			useTriangles.clear();
		}
		if (pGS != NULL) {
			pGS->djTotalN = DrawJob::drawJobs.size() - pGS->djStartN;
			//save prime material
			memcpy(&pGS->mt0, &DrawJob::drawJobs.at(pGS->djStartN)->mt, sizeof(Material));
			pGS->mt0isSet = 1;
		}
	}
	return addedDJs;
}
int ModelBuilder1base::buildSingleDrawJob(Material* pMT00, std::vector<Vertex01*>* pVertices, std::vector<Triangle01*>* pTriangles) {
	int totalVertsN = pVertices->size();
	if (totalVertsN < 1)
		return 0;
	Material* pMT = pMT00;
	if (DrawJob::lineWidthIsImportant(pMT->primitiveType)) {
		if (strcmp(pMT->shaderType, "wire") == 0) {
			if (pMT->primitiveType == GL_LINE_STRIP)
				finalizeLine(pVertices);
			else if (pMT->primitiveType == GL_LINES)
				finalizeShorts(pVertices);
		}
	}
	else {
		optimizeMesh(pVertices, pTriangles);
		totalVertsN = pVertices->size();
	}
	if (pMT->uTex2nm >= 0)
		calculateTangentSpace(pVertices, pTriangles);
	pMT->pickShaderNumber();
	DrawJob* pDJ = new DrawJob();
	//copy material to DJ
	memcpy(&pDJ->mt, pMT, sizeof(Material));
	//calculate VBO element size (stride) and variables offsets in VBO
	int VBOid = DrawJob::newBufferId();
	int stride = 0;
	pDJ->setDesirableOffsets(&stride, pDJ->mt.shaderN, VBOid);
	//create an array for VBO
	int bufferSize = totalVertsN * stride;
	float* vertsBuffer = new float[bufferSize];
	//fill vertsBuffer
	Shader* pSh = Shader::shaders.at(pDJ->mt.shaderN);
	int floatSize = sizeof(float);
	for (int vN = 0; vN < totalVertsN; vN++) {
		Vertex01* pVX = pVertices->at(vN);
		int idx = vN * stride / floatSize;
		//pick data from vertex and move to the buffer
		memcpy(&vertsBuffer[idx + pDJ->aPos.offset / floatSize], pVX->aPos, 3 * floatSize);
		if (pSh->l_aNormal >= 0) //normal
			memcpy(&vertsBuffer[idx + pDJ->aNormal.offset / floatSize], pVX->aNormal, 3 * floatSize);
		if (pSh->l_aTuv >= 0) //attribute TUV (texture coordinates)
			memcpy(&vertsBuffer[idx + pDJ->aTuv.offset / floatSize], pVX->aTuv, 2 * floatSize);
		if (pSh->l_aTuv2 >= 0) //attribute TUV2 (normal maps)
			memcpy(&vertsBuffer[idx + pDJ->aTuv2.offset / floatSize], pVX->aTuv2, 2 * floatSize);
		if (pSh->l_aTangent >= 0)
			memcpy(&vertsBuffer[idx + pDJ->aTangent.offset / floatSize], pVX->aTangent, 3 * floatSize);
		if (pSh->l_aBinormal >= 0)
			memcpy(&vertsBuffer[idx + pDJ->aBinormal.offset / floatSize], pVX->aBinormal, 3 * floatSize);
	}
	//buffer is ready, create VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBOid);
	glBufferData(GL_ARRAY_BUFFER, bufferSize * floatSize, vertsBuffer, GL_STATIC_DRAW);
	delete[] vertsBuffer;
	pDJ->pointsN = totalVertsN;

	int totalTrianglesN = pTriangles->size();
	if (totalTrianglesN > 0) {
		//create EBO
		int totalIndexesN = totalTrianglesN * 3;
		//create buffer
		GLushort* indexBuffer = new GLushort[totalIndexesN];
		for (int tN = 0; tN < totalTrianglesN; tN++) {
			Triangle01* pTR = pTriangles->at(tN);
			int idx = tN * 3;
			indexBuffer[idx] = (GLushort)pTR->idx[0];
			indexBuffer[idx + 1] = (GLushort)pTR->idx[1];
			indexBuffer[idx + 2] = (GLushort)pTR->idx[2];
		}
		//buffer is ready, create IBO
		pDJ->glEBOid = DrawJob::newBufferId();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pDJ->glEBOid);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, totalIndexesN * sizeof(GLushort), indexBuffer, GL_STATIC_DRAW);
		delete[] indexBuffer;
		pDJ->pointsN = totalIndexesN;
	}
	//create and fill vertex attributes array (VAO)
	pDJ->buildVAOforShader(pDJ, pDJ->mt.shaderN);
	//shadow mt
	if (pMT00->dropsShadow > 0) {
		pMT = &pDJ->mtShadow;
		memcpy(pMT, pMT00, sizeof(Material));
		pMT->takesShadow = 0;
		pMT->assignShader(pMT, "depthmap");
	}
	//2-nd layer
	if (strlen(pMT00->layer2as) > 0) {
		//have 2-nd layer
		pMT = &pDJ->mtLayer2;
		memcpy(pMT, pMT00, sizeof(Material));
		MaterialAdjust* pMA = MaterialAdjust::findMaterialAdjust(pMT00->layer2as, NULL);
		if (pMA == NULL) {
			mylog("ERROR in ModelBuilder1base::buildSingleDrawJob, 2-nd layer %s not found\n", pMT00->layer2as);
			return -1;
		}
		pMA->adjust(pMT, pMA);
		//assign shader
		pMT->pickShaderNumber(pMT);
	}

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	return 1;
}
int ModelBuilder1base::rearrangeArraysForDrawJob(std::vector<Vertex01*>* pAllVertices, std::vector<Vertex01*>* pUseVertices, std::vector<Triangle01*>* pUseTriangles) {
	int totalTrianglesN = pUseTriangles->size();
	if (totalTrianglesN < 1)
		return 0;
	//replace triangle original indices by new numbers saved in original vertices altN
	for (int tN = 0; tN < totalTrianglesN; tN++) {
		Triangle01* pTR = pUseTriangles->at(tN);
		for (int i = 0; i < 3; i++) {
			Vertex01* pVX0 = pAllVertices->at(pTR->idx[i]);
			pTR->idx[i] = pVX0->altN;
		}
	}
	return 1;
}

int ModelBuilder1base::moveGroupDg(ModelBuilder1base* pMB, float aX, float aY, float aZ, float kX, float kY, float kZ) {
	//moves and rotates vertex group
	//rotation angles are set in degrees
	mat4x4 transformMatrix = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
	mat4x4_translate(transformMatrix, kX, kY, kZ);
	//rotation order: Z-X-Y
	if (aY != 0) mat4x4_rotate_Y(transformMatrix, transformMatrix, degrees2radians * aY);
	if (aX != 0) mat4x4_rotate_X(transformMatrix, transformMatrix, degrees2radians * aX);
	if (aZ != 0) mat4x4_rotate_Z(transformMatrix, transformMatrix, degrees2radians * aZ);

	int vertsN = pMB->vertices.size();
	for (int i = pMB->pCurrentGroup->fromVertexN; i < vertsN; i++) {
		Vertex01* pVX = pMB->vertices.at(i);
		mat4x4_mul_vec4plus(pVX->aPos, transformMatrix, pVX->aPos, 1);
		mat4x4_mul_vec4plus(pVX->aNormal, transformMatrix, pVX->aNormal, 0);
	}
	return 1;
}

int ModelBuilder1base::calculateTangentSpace(std::vector<Vertex01*>* pUseVertices, std::vector<Triangle01*>* pUseTriangles) {
	int totalVertsN = pUseVertices->size();
	if (totalVertsN < 1)
		return 0;
	int totalTrianglesN = pUseTriangles->size();
	//assuming that GL_TRIANGLES
	//clear flags
	for (int vN = 0; vN < totalVertsN; vN++) {
		Vertex01* pV = pUseVertices->at(vN);
		pV->flag = 0;
	}
	for (int vN = 0; vN < totalVertsN; vN++) {
		Vertex01* pVX = pUseVertices->at(vN);
		if (pVX->flag != 0)
			continue;
		Triangle01* pT = NULL;
		for (int tN = 0; tN < totalTrianglesN; tN++) {
			pT = pUseTriangles->at(tN);
			bool haveTriangle = false;
			for (int i = 0; i < 3; i++)
				if (pT->idx[i] == vN) {
					haveTriangle = true;
					break;
				}
			if (haveTriangle)
				break;
		}
		Vertex01* pV[3];
		for (int i = 0; i < 3; i++)
			pV[i] = pUseVertices->at(pT->idx[i]);

		float dPos1[3];
		float dPos2[3];
		float dUV1[2];
		float dUV2[2];
		for (int i = 0; i < 3; i++) {
			dPos1[i] = pV[1]->aPos[i] - pV[0]->aPos[i];
			dPos2[i] = pV[2]->aPos[i] - pV[0]->aPos[i];
		}
		for (int i = 0; i < 2; i++) {
			dUV1[i] = pV[1]->aTuv2[i] - pV[0]->aTuv2[i];
			dUV2[i] = pV[2]->aTuv2[i] - pV[0]->aTuv2[i];
		}
		float tangent[4];
		float binormal[4];
		float divider = dUV1[0] * dUV2[1] - dUV1[1] * dUV2[0];
		if (divider == 0) {
			//calculate tangent space from normal 
			float normPitch = v3pitchRd(pVX->aNormal);
			float normYaw = v3yawRd(pVX->aNormal);
			mat4x4 mx;
			mat4x4_identity(mx);
			if (normYaw != 0)
				mat4x4_rotate_Y(mx, mx, normYaw);
			if (normPitch != 0)
				mat4x4_rotate_X(mx, mx, normPitch);
			float vecRight[4] = { 1,0,0,0 };
			mat4x4_mul_vec4plus(tangent, mx, vecRight, 0);
			float vecUp[4] = { 0,1,0,0 };
			mat4x4_mul_vec4plus(binormal, mx, vecUp, 0);
		}
		else {
			float r = 1.0f / divider;
			for (int i = 0; i < 3; i++) {
				tangent[i] = (dPos1[i] * dUV2[1] - dPos2[i] * dUV1[1]) * r;
				binormal[i] = -(dPos2[i] * dUV1[0] - dPos1[i] * dUV2[0]) * r;
			}
			vec3_norm(tangent, tangent);
			vec3_norm(binormal, binormal);
		}
		//add to all 3 vertices
		for (int n = 0; n < 3; n++) {
			if (pV[n]->flag > 0)
				continue;
			v3copy(pV[n]->aTangent, tangent);
			v3copy(pV[n]->aBinormal, binormal);
			pV[n]->flag = 1;
		}
	}
	//normalize tangent and binormal around normal
	for (int vN = 0; vN < totalVertsN; vN++) {
		Vertex01* pV = pUseVertices->at(vN);
		float v3out[3];
		//tangent
		vec3_mul_cross(v3out, pV->aNormal, pV->aBinormal);
		if (v3dotProduct(pV->aTangent, v3out) < 0)
			v3inverse(v3out);
		v3copy(pV->aTangent, v3out);
		//binormal
		vec3_mul_cross(v3out, pV->aNormal, pV->aTangent);
		if (v3dotProduct(pV->aBinormal, v3out) < 0)
			v3inverse(v3out);
		v3copy(pV->aBinormal, v3out);
	}
	return 1;
}
void ModelBuilder1base::lockGroup(ModelBuilder1base* pMB) {
	Group01* pPrevGroup = pMB->pCurrentGroup;
	if (pMB->pCurrentGroup != NULL)
		pMB->groupsStack.push_back(pMB->pCurrentGroup);
	pMB->pCurrentGroup = new Group01();
	pMB->pCurrentGroup->fromVertexN = pMB->vertices.size();
	pMB->pCurrentGroup->fromTriangleN = pMB->triangles.size();
	//marks
	if(pPrevGroup != NULL)
		if (strcmp(pPrevGroup->marks, "") != 0)
			myStrcpy_s(pMB->pCurrentGroup->marks, 128, pPrevGroup->marks);
}
void ModelBuilder1base::releaseGroup(ModelBuilder1base* pMB) {
	if (pMB->pLastClosedGroup != NULL)
		delete pMB->pLastClosedGroup;
	pMB->pLastClosedGroup = pMB->pCurrentGroup;

	if (pMB->groupsStack.size() > 0) {
		pMB->pCurrentGroup = pMB->groupsStack.back();
		pMB->groupsStack.pop_back();
	}
	else
		pMB->pCurrentGroup = NULL;
}
int ModelBuilder1base::finalizeLine(std::vector<Vertex01*>* pVerts, int lineStartsAt, int lineEndsAt) {
	if (lineEndsAt <= 0)
		lineEndsAt = pVerts->size() - 1;
	Vertex01* pV0 = pVerts->at(lineStartsAt);
	Vertex01* pV2 = pVerts->at(lineEndsAt);
	bool closedLine = false;
	if (v3match(pV0->aPos, pV2->aPos))
		closedLine = true;
	for (int vN = lineStartsAt; vN <= lineEndsAt; vN++) {
		Vertex01* pV = pVerts->at(vN);
		//prev point
		if (vN == lineStartsAt) {
			//first point
			if (closedLine)
				pV0 = pVerts->at(lineEndsAt);
			else
				pV0 = NULL;
		}
		else
			pV0 = pVerts->at(vN - 1);
		//next point
		if (vN == lineEndsAt) {
			//last point
			if (closedLine)
				pV2 = pVerts->at(lineStartsAt);
			else
				pV2 = NULL;
		}
		else
			pV2 = pVerts->at(vN + 1);
		//distances to neighbor points
		float distFromPrev = 0;
		float dirFromPrev[3] = { 0,0,0 };
		if (pV0 != NULL) {
			distFromPrev = v3lengthFromTo(pV0->aPos, pV->aPos);
			v3dirFromTo(dirFromPrev, pV0->aPos, pV->aPos);
		}
		float distToNext = 0;
		float dirToNext[3] = { 0,0,0 };
		if (pV2 != NULL) {
			distToNext = v3lengthFromTo(pV->aPos, pV2->aPos);
			v3dirFromTo(dirToNext, pV->aPos, pV2->aPos);
		}
		float distTotal = distFromPrev + distToNext;
		float kPrev = distFromPrev / distTotal;
		float kNext = distToNext / distTotal;
		if (kPrev > kNext * 3)
			v3copy(pV->aNormal, dirFromPrev);
		else if (kNext > kPrev * 3)
			v3copy(pV->aNormal, dirToNext);
		else
			for (int i = 0; i < 3; i++)
				pV->aNormal[i] = kPrev * dirFromPrev[i] + kNext * dirToNext[i];
		vec3_norm(pV->aNormal, pV->aNormal);
	}
	return 1;
}
int ModelBuilder1base::optimizeMesh(std::vector<Vertex01*>* pVertices, std::vector<Triangle01*>* pTriangles) {
	int trianglesN0 = pTriangles->size();
	if (trianglesN0 == 0)
		return 0;
	int vertsN0 = pVertices->size();
	//clear verts first for comparison
	for (int vN = 0; vN < vertsN0; vN++) {
		Vertex01* pV = pVertices->at(vN);
		myStrcpy_s(pV->marks, 128, "");
		pV->altN = -1;
		pV->flag = 0;
		pV->aTangent[0] = 0;
		//round up
		for (int i = 0; i < 3; i++) {
			pV->aPos[i] = (float)round(pV->aPos[i] * 100.0f) / 100.0f;
			pV->aNormal[i] = (float)round(pV->aNormal[i] * 100.0f) / 100.0f;
		}
 	}
	//find the same verts
	int matchesN = 0;
	for (int vN = 0; vN < vertsN0-1; vN++) {
		Vertex01* pV = pVertices->at(vN);
		if (pV->flag < 0)
			continue;
		for (int vN2 = vN+1; vN2 < vertsN0; vN2++) {
			Vertex01* pV2 = pVertices->at(vN2);
			if (pV2->flag < 0)
				continue;
			if (memcmp(pV, pV2, sizeof(Vertex01)) != 0)
				continue;
			//if here - verts are equal
			pV2->flag = -1;
			matchesN++;
			//change refs in useTriangles from vN2 to vN
			for (int tN = 0; tN < trianglesN0; tN++) {
				Triangle01* pT = pTriangles->at(tN);
				for (int i = 0; i < 3; i++)
					if (pT->idx[i] == vN2)
						pT->idx[i] = vN;
			}
		}
	}
	if (matchesN == 0)
		return 0;
	//unflag all verts
	for (int vN = 0; vN < vertsN0; vN++) {
		Vertex01* pV = pVertices->at(vN);
		pV->flag = -1;
	}
	//flag verts in use
	for (int tN = 0; tN < trianglesN0; tN++) {
		Triangle01* pT = pTriangles->at(tN);
		for (int i = 0; i < 3; i++) {
			int vN = pT->idx[i];
			Vertex01* pV = pVertices->at(vN);
			pV->flag = 0;
		}
	}
	//save original useVertices copy
	std::vector<Vertex01*> oldVertices;
	for (int vN = 0; vN < vertsN0; vN++) {
		Vertex01* pV = pVertices->at(vN);
		oldVertices.push_back(pV);
	}
	pVertices->clear();
	//copy back only verts in use
	for (int vN = 0; vN < vertsN0; vN++) {
		Vertex01* pV = oldVertices.at(vN);
		if (pV->flag < 0)
			continue;
		pV->altN = pVertices->size();
		pVertices->push_back(pV);
	}
	//re-factor triangles
	rearrangeArraysForDrawJob(&oldVertices, pVertices, pTriangles);
	oldVertices.clear();
	return pVertices->size();
}
int ModelBuilder1base::finalizeShorts(std::vector<Vertex01*>* pVerts) {
	int vertsN = pVerts->size();
	for (int vN = 0; vN < vertsN; vN+=2) {
		Vertex01* pV0 = pVerts->at(vN);
		Vertex01* pV1 = pVerts->at(vN+1);
		for (int i = 0; i < 3; i++)
			pV0->aNormal[i] = pV1->aPos[i] - pV0->aPos[i];
		vec3_norm(pV0->aNormal, pV0->aNormal);
		v3copy(pV1->aNormal, pV0->aNormal);
	}
	return 1;
}
int ModelBuilder1base::debug_showNormals(ModelBuilder1base* pMB) {
	return 0;

	mylog("=========================\n");
	std::vector<Vertex01*>* pVerts = &pMB->vertices;
	int totalN = pVerts->size();
	for (int vN = 0; vN < totalN; vN++) {
		Vertex01* pV = pVerts->at(vN);
		if (pV->showNormal < 1)
			continue;
		mylog("vN=%d", vN);
		mylog_v3(" ", pV->aPos);
		mylog_v3(" nrm:", pV->aNormal);
		mylog(" mn=%d area=%.2f\n", pV->triangleMatchingNormals, pV->triangleArea);
	}
	return 1;
}
