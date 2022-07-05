#include "ModelBuilder.h"
#include "platform.h"
#include "utils.h"
#include "DrawJob.h"
#include "Shader.h"
#include "GroupTransform.h"

extern float degrees2radians;

ModelBuilder::~ModelBuilder() {
}
int ModelBuilder::buildFace(ModelBuilder* pMB, std::string applyTo, VirtualShape* pVS, TexCoords* pTC, TexCoords* pTC2nm) {
	if (strstr(pVS->shapeType, "box") == pVS->shapeType)
		buildBoxFace(pMB, applyTo, pVS, pTC, pTC2nm);
	else if (strstr(pVS->shapeType, "cylinder") == pVS->shapeType)
		buildRoundFace(pMB, applyTo, pVS, pTC, pTC2nm);
	else if (strstr(pVS->shapeType, "cap") == pVS->shapeType)
		buildRoundFace(pMB, applyTo, pVS, pTC, pTC2nm);
	else if (strstr(pVS->shapeType, "cone") == pVS->shapeType)
		buildRoundFace(pMB, applyTo, pVS, pTC, pTC2nm);
	else
		return -1;
	return 1;
}
int ModelBuilder::buildBoxFace(ModelBuilder* pMB, std::string applyTo, VirtualShape* pVS, TexCoords* pTC, TexCoords* pTC2nm) {
	//this code is for simple box
	VirtualShape vs; //face VS, 
	mat4x4 transformMatrix = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
	vs.sectionsR = pVS->sectionsR;
	//rotate desirable side to face us. 
	if (applyTo.find("front") == 0) {
		//Side <front> is facing us as is.
		vs.whl[0] = pVS->whl[0];
		vs.whl[1] = pVS->whl[1];
		vs.sections[0] = pVS->sections[0];
		vs.sections[1] = pVS->sections[1];
		//extensions
		vs.extF = pVS->extF;
		vs.extL = pVS->extL;
		vs.extR = pVS->extR;
		vs.extU = pVS->extU;
		vs.extD = pVS->extD;
		//define how to move/place generated face back to the VirtualShape
		//just shift closer to us by length/2
		mat4x4_translate(transformMatrix, 0, 0, pVS->whl[2] / 2);
	}
	else if (applyTo.find("back") == 0) {
		vs.whl[0] = pVS->whl[0];
		vs.whl[1] = pVS->whl[1];
		vs.sections[0] = pVS->sections[0];
		vs.sections[1] = pVS->sections[1];
		//extensions
		vs.extF = pVS->extB;
		vs.extL = pVS->extR;
		vs.extR = pVS->extL;
		vs.extU = pVS->extU;
		vs.extD = pVS->extD;
		//rotate 180 degrees around Y and shift farther from us by half-length
		mat4x4_translate(transformMatrix, 0, 0, -pVS->whl[2] / 2);
		mat4x4_rotate_Y(transformMatrix, transformMatrix, degrees2radians * 180);
	}
	else if (applyTo.find("left") == 0) {
		vs.whl[0] = pVS->whl[2]; //width = original length
		vs.whl[1] = pVS->whl[1];
		vs.sections[0] = pVS->sections[2];
		vs.sections[1] = pVS->sections[1];
		//extensions
		vs.extF = pVS->extL;
		vs.extL = pVS->extB;
		vs.extR = pVS->extF;
		vs.extU = pVS->extU;
		vs.extD = pVS->extD;
		//rotate -90 degrees around Y (CW) and shift half-width to the left
		mat4x4_translate(transformMatrix, -pVS->whl[0] / 2, 0, 0);
		mat4x4_rotate_Y(transformMatrix, transformMatrix, -degrees2radians * 90);
	}
	else if (applyTo.find("right") == 0) {
		vs.whl[0] = pVS->whl[2]; //width = original length
		vs.whl[1] = pVS->whl[1];
		vs.sections[0] = pVS->sections[2];
		vs.sections[1] = pVS->sections[1];
		//extensions
		vs.extF = pVS->extR;
		vs.extL = pVS->extF;
		vs.extR = pVS->extB;
		vs.extU = pVS->extU;
		vs.extD = pVS->extD;
		//rotate +90 degrees around Y (CCW) and shift half-width to the right
		mat4x4_translate(transformMatrix, pVS->whl[0] / 2, 0, 0);
		mat4x4_rotate_Y(transformMatrix, transformMatrix, degrees2radians * 90);
	}
	else if (applyTo.find("top") == 0) {
		vs.whl[0] = pVS->whl[0];
		vs.whl[1] = pVS->whl[2]; //height = original length
		vs.sections[0] = pVS->sections[0];
		vs.sections[1] = pVS->sections[2];
		//extensions
		vs.extF = pVS->extU;
		vs.extL = pVS->extR;
		vs.extR = pVS->extL;
		vs.extU = pVS->extF;
		vs.extD = pVS->extB;
		//rotate -90 degrees around X (CW) and 180 around Y, and shift half-height up
		mat4x4_translate(transformMatrix, 0, pVS->whl[1] / 2, 0);
		mat4x4_rotate_Y(transformMatrix, transformMatrix, degrees2radians * 180);
		mat4x4_rotate_X(transformMatrix, transformMatrix, -degrees2radians * 90);
	}
	else if (applyTo.find("bottom") == 0) {
		vs.whl[0] = pVS->whl[0];
		vs.whl[1] = pVS->whl[2]; //height = original length
		vs.sections[0] = pVS->sections[0];
		vs.sections[1] = pVS->sections[2];
		//extensions
		vs.extF = pVS->extD;
		vs.extL = pVS->extL;
		vs.extR = pVS->extR;
		vs.extU = pVS->extF;
		vs.extD = pVS->extB;
		//rotate 90 around X (CCW) and shift half-height down
		mat4x4_translate(transformMatrix, 0, -pVS->whl[1] / 2, 0);
		mat4x4_rotate_X(transformMatrix, transformMatrix, degrees2radians * 90);
	}
	lockGroup(pMB);
	//create vertices
	if (strstr(pVS->shapeType, "tank") != nullptr)
		buildBoxFaceTank(pMB, applyTo, &vs);
	else
		buildBoxFacePlain(pMB, applyTo, &vs);

	groupApplyTexture(pMB, "front", pTC, pTC2nm);

	//move face to it's place (apply transform matrix)
	int vertsN = pMB->vertices.size();
	for (int i = pMB->pCurrentGroup->fromVertexN; i < vertsN; i++) {
		Vertex01* pVX = pMB->vertices.at(i);
		mat4x4_mul_vec4plus(pVX->aPos, transformMatrix, pVX->aPos, 1);
		mat4x4_mul_vec4plus(pVX->aNormal, transformMatrix, pVX->aNormal, 0);
	}
	releaseGroup(pMB);
	return 1;
}
int ModelBuilder::buildBoxFacePlain(ModelBuilder* pMB, std::string applyTo, VirtualShape* pVS) {
	if (pVS->whl[0] == 0 || pVS->whl[1] == 0)
		return 0;
	//create vertices
	int sectionsX = pVS->sections[0];
	int sectionsY = pVS->sections[1];
	int pointsX = sectionsX + 1;
	int pointsY = sectionsY + 1;
	float stepX = pVS->whl[0] / sectionsX;
	float stepY = pVS->whl[1] / sectionsY;
	float kY = pVS->whl[1] / 2;
	for (int iy = 0; iy < pointsY; iy++) {
		float kX = -pVS->whl[0] / 2;
		for (int ix = 0; ix < pointsX; ix++) {
			int nSE = addVertex(pMB, kX, kY, pVS->extF, 0, 0, 1); //vertex number on south-east
			if (iy > 0 && ix > 0) {
				//add 2 triangles
				int nSW = nSE - 1; //vertex number south-west
				int nNE = nSE - pointsX; //north-east
				int nNW = nSW - pointsX; //north-west
				add2triangles(pMB, nNW, nNE, nSW, nSE, iy + ix);
			}
			kX += stepX;
		}
		kY -= stepY;
	}
	return 1;
}
int ModelBuilder::buildBoxFaceTank(ModelBuilder* pMB, std::string applyTo, VirtualShape* pVS) {
	//for diamond effect - sectionsRad=1, don't merge normals
	bool drawMiddle = true;
	//edges
	bool drawTop = false;
	bool drawBottom = false;
	bool drawLeft = false;
	bool drawRight = false;
	//corners
	bool drawTopLeft = false;
	bool drawTopRight = false;
	bool drawBottomLeft = false;
	bool drawBottomRight = false;
	if (pVS->extF == 0 || applyTo.find(" all") != std::string::npos) {
		drawTop = true;
		drawBottom = true;
		drawLeft = true;
		drawRight = true;
		drawTopLeft = true;
		drawTopRight = true;
		drawBottomLeft = true;
		drawBottomRight = true;
	}
	else if (applyTo.find(" h") != std::string::npos) {
		drawLeft = true;
		drawRight = true;
	}
	else if (applyTo.find(" v") != std::string::npos) {
		drawTop = true;
		drawBottom = true;
	}
	if (applyTo.find(" no") != std::string::npos) {
		if (applyTo.find(" noM") != std::string::npos) {
			//middle
			if (applyTo.find(" noMrow") != std::string::npos) {
				drawMiddle = false;
				drawLeft = false;
				drawRight = false;
			}
			if (applyTo.find(" noMcol") != std::string::npos) {
				drawMiddle = false;
				drawTop = false;
				drawBottom = false;
			}
			if (applyTo.find(" noMid") != std::string::npos)
				drawMiddle = false;
		}
		if (applyTo.find(" noN") != std::string::npos) {
			//north
			if (applyTo.find(" noNrow") != std::string::npos) {
				drawTop = false;
				drawTopLeft = false;
				drawTopRight = false;
			}
			if (applyTo.find(" noNedge") != std::string::npos)
				drawTop = false;
			if (applyTo.find(" noNW") != std::string::npos)
				drawTopLeft = false;
			if (applyTo.find(" noNE") != std::string::npos)
				drawTopRight = false;
		}
		if (applyTo.find(" noS") != std::string::npos) {
			//south
			if (applyTo.find(" noSrow") != std::string::npos) {
				drawBottom = false;
				drawBottomLeft = false;
				drawBottomRight = false;
			}
			if (applyTo.find(" noSedge") != std::string::npos)
				drawBottom = false;
			if (applyTo.find(" noSW") != std::string::npos)
				drawBottomLeft = false;
			if (applyTo.find(" noSE") != std::string::npos)
				drawBottomRight = false;
		}
		if (applyTo.find(" noW") != std::string::npos) {
			//west
			if (applyTo.find(" noWcol") != std::string::npos) {
				drawLeft = false;
				drawTopLeft = false;
				drawBottomLeft = false;
			}
			if (applyTo.find(" noWedge") != std::string::npos)
				drawLeft = false;
		}
		if (applyTo.find(" noE") != std::string::npos) {
			//east
			if (applyTo.find(" noEcol") != std::string::npos) {
				drawRight = false;
				drawTopRight = false;
				drawBottomRight = false;
			}
			if (applyTo.find(" noEedge") != std::string::npos)
				drawRight = false;
		}
	}
	lockGroup(pMB);
	//middle
	if (pVS->whl[0] > 0 && pVS->whl[1] > 0 && drawMiddle) {
		buildBoxFacePlain(pMB, applyTo, pVS);
	}
	VirtualShape vs;
	//edges
	//vs.type.assign("cylinder");
	vs.sectionsR = pVS->sectionsR;
	if (pVS->whl[0] > 0) {
		vs.sections[2] = pVS->sections[0]; //cylinder Z sections n
		vs.whl[2] = pVS->whl[0]; //cylinder length Z
		vs.whl[0] = pVS->extF * 2; //cylinder diameter X
		if (pVS->extU > 0 && drawTop) {
			vs.whl[1] = pVS->extU * 2; //cylinder diameter Y
			lockGroup(pMB);
			cylinderWrap(pMB, &vs, 0, 90, NULL, NULL);
			//rotate -90 degrees around Y and shift up
			moveGroupDg(pMB, 0, -90, 0, 0, pVS->whl[1] * 0.5f, 0);
			releaseGroup(pMB);
		}
		if (pVS->extD > 0 && drawBottom) {
			vs.whl[1] = pVS->extD * 2; //cylinder diameter Y
			lockGroup(pMB);
			cylinderWrap(pMB, &vs, -90, 0, NULL, NULL);
			//rotate -90 degrees around Y and shift down
			moveGroupDg(pMB, 0, -90, 0, 0, -pVS->whl[1] * 0.5f, 0);
			releaseGroup(pMB);
		}
	}
	if (pVS->whl[1] > 0) {
		vs.sections[2] = pVS->sections[1]; //cylinder Z sections n
		vs.whl[2] = pVS->whl[1]; //cylinder length Z
		vs.whl[1] = pVS->extF * 2; //cylinder diameter Y
		if (pVS->extL > 0 && drawLeft) {
			vs.whl[0] = pVS->extL * 2; //cylinder diameter X
			lockGroup(pMB);
			cylinderWrap(pMB, &vs, 90, 180, NULL, NULL);
			//rotate 90 degrees around Y and shift left
			moveGroupDg(pMB, 90, 0, 0, -pVS->whl[0] * 0.5f, 0, 0);
			releaseGroup(pMB);
		}
		if (pVS->extR > 0 && drawRight) {
			vs.whl[0] = pVS->extR * 2; //cylinder diameter X
			lockGroup(pMB);
			cylinderWrap(pMB, &vs, 0, 90, NULL, NULL);
			//rotate 90 degrees around Y and shift left
			moveGroupDg(pMB, 90, 0, 0, pVS->whl[0] * 0.5f, 0, 0);
			releaseGroup(pMB);
		}
	}
	//corners
	//vs.type.assign("cap");
	vs.sectionsR = pVS->sectionsR;
	vs.sections[2] = pVS->sectionsR;
	vs.whl[2] = pVS->extF;
	if (pVS->extU > 0) {
		//top corners
		vs.whl[1] = pVS->extU * 2;
		if (pVS->extL > 0 && drawTopLeft) {
			vs.whl[0] = pVS->extL * 2;
			lockGroup(pMB);
			capWrap(pMB, &vs, 90, 180, NULL, NULL);
			//rotate 90 degrees around Y and shift left
			moveGroupDg(pMB, 0, 0, 0, -pVS->whl[0] * 0.5f, pVS->whl[1] * 0.5f, 0);
			releaseGroup(pMB);
		}
		if (pVS->extR > 0 && drawTopRight) {
			vs.whl[0] = pVS->extR * 2;
			lockGroup(pMB);
			capWrap(pMB, &vs, 0, 90, NULL, NULL);
			//rotate 90 degrees around Y and shift left
			moveGroupDg(pMB, 0, 0, 0, pVS->whl[0] * 0.5f, pVS->whl[1] * 0.5f, 0);
			releaseGroup(pMB);

		}
	}
	if (pVS->extD > 0) {
		//bottom corners
		vs.whl[1] = pVS->extD * 2;
		if (pVS->extL > 0 && drawBottomLeft) { 
			vs.whl[0] = pVS->extL * 2;
			lockGroup(pMB);
			capWrap(pMB, &vs, -180, -90, NULL, NULL);
			//rotate 90 degrees around Y and shift left
			moveGroupDg(pMB, 0, 0, 0, -pVS->whl[0] * 0.5f, -pVS->whl[1] * 0.5f, 0);
			releaseGroup(pMB);
		}
		if (pVS->extR > 0 && drawBottomRight) {
			vs.whl[0] = pVS->extR * 2;
			lockGroup(pMB);
			capWrap(pMB, &vs, -90, 0, NULL, NULL);
			//rotate 90 degrees around Y and shift left
			moveGroupDg(pMB, 0, 0, 0, pVS->whl[0] * 0.5f, -pVS->whl[1] * 0.5f, 0);
			releaseGroup(pMB);
		}
	}
	if (pVS->extF == 0) {
		int vertsN = pMB->vertices.size();
		for (int i = pMB->pCurrentGroup->fromVertexN; i < vertsN; i++) {
			Vertex01* pVX = pMB->vertices.at(i);
			//normal
			v3set(pVX->aNormal, 0, 0, 1);
		}
	}
	releaseGroup(pMB);
	return 1;
}

int ModelBuilder::cylinderWrap(ModelBuilder* pMB, VirtualShape* pVS, float angleFrom, float angleTo, TexCoords* pTC, TexCoords* pTC2nm) {
	// angleFrom/To - in degrees
	lockGroup(pMB);
	float stepZ = pVS->whl[2] / pVS->sections[2];
	float stepDg = (angleTo - angleFrom) / pVS->sectionsR; //in degrees
	for (int nz = 0; nz <= pVS->sections[2]; nz++) {
		float kz = stepZ * nz - pVS->whl[2] * 0.5f;
		//tuv
		float yRate = 1.0f - (float)nz / (float)pVS->sections[2];
		for (int rpn = 0; rpn <= pVS->sectionsR; rpn++) {
			// rpn - radial point number
			float angleRd = (angleFrom + stepDg * rpn) * degrees2radians;
			float kx = cosf(angleRd);
			float ky = sinf(angleRd);
			int nSE = addVertex(pMB, kx, ky, kz, kx, ky, 0);
			if (nz > 0 && rpn > 0) {
				int nSW = nSE - 1;
				int nNW = nSW - pVS->sectionsR - 1;
				int nNE = nSE - pVS->sectionsR - 1;
				add2triangles(pMB, nNE, nNW, nSE, nSW, nz + rpn);
			}
			//tuv
			float xRate = (float)rpn / (float)pVS->sectionsR;
			Vertex01* pVX = pMB->vertices.at(nSE);
			TexCoords::setTUV(pTC, pVX->aTuv, xRate, yRate);
			TexCoords::setTUV(pTC2nm, pVX->aTuv2, xRate, yRate);
		}
	}
	//scale to desirable diameters
	mat4x4 transformMatrix = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
	mat4x4_scale_aniso(transformMatrix, transformMatrix, pVS->whl[0] * 0.5f, pVS->whl[1] * 0.5f, 1);
	int vertsN = pMB->vertices.size();
	for (int i = pMB->pCurrentGroup->fromVertexN; i < vertsN; i++) {
		Vertex01* pVX = pMB->vertices.at(i);
		mat4x4_mul_vec4plus(pVX->aPos, transformMatrix, pVX->aPos, 1);
	}
	releaseGroup(pMB);
	return 1;
}


int ModelBuilder::capWrap(ModelBuilder* pMB, VirtualShape* pVS, float angleFrom, float angleTo, TexCoords* pTC, TexCoords* pTC2nm) {
	// angleFrom/To - in degrees
	//builds R=1 cap, then - scale
	lockGroup(pMB);
	//textures
	float xRate = 0;
	float yRate = 0;
	//center point
	int n0 = addVertex(pMB, 0, 0, 1, 0, 0, 1);
	//apply textures
	Vertex01* pVX = pMB->vertices.at(n0);
	TexCoords::setTUV(pTC, pVX->aTuv, 0, 1);
	TexCoords::setTUV(pTC2nm, pVX->aTuv2, 0, 1);

	float stepZdg = 90.0f / pVS->sections[2]; //in degrees
	float stepRdg = (angleTo - angleFrom) / pVS->sectionsR; //in degrees
	for (int nz = 1; nz <= pVS->sections[2]; nz++) {
		float angleZrd = stepZdg * nz * degrees2radians;
		float kz = cosf(angleZrd);
		float R = sinf(angleZrd);
		//tuv
		yRate = 1.0f - R;
		for (int rpn = 0; rpn <= pVS->sectionsR; rpn++) {
			// rpn - radial point number
			float angleRd = (angleFrom + stepRdg * rpn) * degrees2radians;
			float kx = cosf(angleRd) * R;
			float ky = sinf(angleRd) * R;
			int nSE = addVertex(pMB, kx, ky, kz, kx, ky, kz);
			//tuv
			xRate = (float)rpn / (float)pVS->sectionsR;
			Vertex01* pVX = pMB->vertices.at(nSE);
			TexCoords::setTUV(pTC, pVX->aTuv, xRate, yRate);
			TexCoords::setTUV(pTC2nm, pVX->aTuv2, xRate, yRate);

			if (rpn > 0) {
				if (nz == 1) {
					int nSW = nSE - 1;
					addTriangle(pMB, n0, nSW, nSE);
				}
				else {
					int nSW = nSE - 1;
					int nNW = nSW - pVS->sectionsR - 1;
					int nNE = nSE - pVS->sectionsR - 1;
					add2triangles(pMB, nNW, nNE, nSW, nSE, nz + rpn);
				}
			}
		}
	}
	//scale to desirable diameters
	mat4x4 transformMatrix = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
	mat4x4_scale_aniso(transformMatrix, transformMatrix, pVS->whl[0] * 0.5f, pVS->whl[1] * 0.5f, pVS->whl[2]);
	int vertsN = pMB->vertices.size();
	for (int i = pMB->pCurrentGroup->fromVertexN; i < vertsN; i++) {
		Vertex01* pVX = pMB->vertices.at(i);
		mat4x4_mul_vec4plus(pVX->aPos, transformMatrix, pVX->aPos, 1);
	}
	if (pVS->whl[2] == 0) { //flattern?
		for (int i = pMB->pCurrentGroup->fromVertexN; i < vertsN; i++) {
			Vertex01* pVX = pMB->vertices.at(i);
			v3set(pVX->aNormal, 0,0,1);
		}
	}
	releaseGroup(pMB);
	return 1;
}
int ModelBuilder::coneWrap(ModelBuilder* pMB, VirtualShape* pVS, float angleFrom, float angleTo, TexCoords* pTC, TexCoords* pTC2nm) {
	// angleFrom/To - in degrees
	//builds R=1 cone, then - scale
	lockGroup(pMB);
	/*
	//textures
	float xRate = 0;
	float yRate = 0;
	//center point
	int n0 = addVertex(pMB, 0, 0, 1, 0, 0, 1);
	//apply textures
	Vertex01* pVX = pMB->vertices.at(n0);
	TexCoords::setTUV(pTC, pVX->aTuv, 0, 1);
	TexCoords::setTUV(pTC2nm, pVX->aTuv2, 0, 1);

	float stepZdg = 90.0f / pVS->sections[2]; //in degrees
	*/
	float stepRdg = (angleTo - angleFrom) / pVS->sectionsR; //in degrees
	for (int nz = 0; nz <= pVS->sections[2]; nz++) {
		float kz = 1.0f - (float)nz / (float)pVS->sections[2];
		float R = (float)nz / (float)pVS->sections[2];
		//tuv
		float yRate = 1.0f - R;
		for (int rpn = 0; rpn <= pVS->sectionsR; rpn++) {
			// rpn - radial point number
			if (nz == 0 && rpn == pVS->sectionsR) 
				continue;
			float angleRd = (angleFrom + stepRdg * rpn) * degrees2radians;
			float kx = cosf(angleRd) * R;
			float ky = sinf(angleRd) * R;
			int nSE = addVertex(pMB, kx*R, ky*R, kz, kx, ky, 0);
			//tuv
			float xRate = (float)rpn / (float)pVS->sectionsR;
			Vertex01* pVX = pMB->vertices.at(nSE);
			TexCoords::setTUV(pTC, pVX->aTuv, xRate, yRate);
			TexCoords::setTUV(pTC2nm, pVX->aTuv2, xRate, yRate);
			if (nz > 0)
			if (rpn > 0) {
				if (nz == 1) {
					int nSW = nSE - 1;
					int n0 = nSE - pVS->sectionsR; //cone center point
					addTriangle(pMB, n0, nSW, nSE);
				}
				else {
					int nSW = nSE - 1;
					int nNW = nSW - pVS->sectionsR - 1;
					int nNE = nSE - pVS->sectionsR - 1;
					add2triangles(pMB, nNW, nNE, nSW, nSE, nz + rpn);
				}
			}
		}
	}
	//scale to desirable diameters
	mat4x4 transformMatrix = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
	mat4x4_scale_aniso(transformMatrix, transformMatrix, pVS->whl[0] * 0.5f, pVS->whl[1] * 0.5f, pVS->whl[2]);
	int vertsN = pMB->vertices.size();
	for (int i = pMB->pCurrentGroup->fromVertexN; i < vertsN; i++) {
		Vertex01* pVX = pMB->vertices.at(i);
		mat4x4_mul_vec4plus(pVX->aPos, transformMatrix, pVX->aPos, 1);
	}
	releaseGroup(pMB);
	return 1;
}
int ModelBuilder::groupApplyTexture(ModelBuilder* pMB, std::string applyTo, TexCoords* pTC, TexCoords* pTC2nm) {
	int vertsN = pMB->vertices.size();
	for (int vN = 0; vN < vertsN; vN++) {
		Vertex01* pVX = pMB->vertices.at(vN);
		if (vN < pMB->pCurrentGroup->fromVertexN)
			pVX->flag = -1;
		else
			pVX->flag = 0;
	}
	applyTexture2flagged(&pMB->vertices, applyTo, pTC, pTC2nm);
	return 1;
}
int ModelBuilder::applyTexture2flagged(std::vector<Vertex01*>* pVerts, std::string applyTo, TexCoords* pTC, TexCoords* pTC2nm) {
	if (pTC == NULL && pTC2nm == NULL)
		return 0;
	float posMin[3];
	float posMax[3];
	float posRange[3];
	for (int i = 0; i < 3; i++) {
		posMin[i] = 1000000;
		posMax[i] = -1000000;
	}
	int vertsN = pVerts->size();
	for (int vN = 0; vN < vertsN; vN++) {
		Vertex01* pVX = pVerts->at(vN);
		if (pVX->flag < 0) //ignore
			continue;
		for (int i = 0; i < 3; i++) {
			if (posMin[i] > pVX->aPos[i])
				posMin[i] = pVX->aPos[i];
			if (posMax[i] < pVX->aPos[i])
				posMax[i] = pVX->aPos[i];
		}
	}
	//here we have coordinates range
	for (int i = 0; i < 3; i++)
		posRange[i] = posMax[i] - posMin[i];
	//for "front"
	int xRateIndex = 0;
	bool xRateInverse = false;
	int yRateIndex = 1;
	bool yRateInverse = true;
	if (applyTo.find("front") == 0)
		; //do nothing
	else if (applyTo.find("back") == 0)
		xRateInverse = true;
	else if (applyTo.find("left") == 0)
		xRateIndex = 2;
	else if (applyTo.find("right") == 0) {
		xRateIndex = 2;
		xRateInverse = true;
	}
	else if (applyTo.find("top") == 0) {
		xRateInverse = true;
		yRateIndex = 2;
	}
	else if (applyTo.find("bottom") == 0)
		yRateIndex = 2;

	float xRate = 0;
	float yRate = 0;
	for (int vN = 0; vN < vertsN; vN++) {
		Vertex01* pVX = pVerts->at(vN);
		if (pVX->flag < 0) //ignore
			continue;
		if (posRange[xRateIndex] == 0)
			xRate = 0;
		else {
			xRate = (pVX->aPos[xRateIndex] - posMin[xRateIndex]) / posRange[xRateIndex];
			if (xRateInverse)
				xRate = 1.0f - xRate;
		}
		if (posRange[yRateIndex] == 0)
			yRate = 0;
		else {
			yRate = (pVX->aPos[yRateIndex] - posMin[yRateIndex]) / posRange[yRateIndex];
			if (yRateInverse)
				yRate = 1.0f - yRate;
		}
		TexCoords::setTUV(pTC, pVX->aTuv, xRate, yRate);
		TexCoords::setTUV(pTC2nm, pVX->aTuv2, xRate, yRate);
	}
	return 1;
}
int ModelBuilder::buildRoundFace(ModelBuilder* pMB, std::string applyTo, VirtualShape* pVS, TexCoords* pTC, TexCoords* pTC2nm) {
	//cap, cylinder, cone
	VirtualShape vs; //face VS, 
	memcpy(&vs, pVS, sizeof(VirtualShape));
	mat4x4 transformMatrix = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
	lockGroup(pMB);
	if (applyTo.find("wrap") == 0) {
		if (strstr(pVS->shapeType, "cap") == pVS->shapeType)
			capWrap(pMB, pVS, pVS->angleFromTo[0], pVS->angleFromTo[1], pTC, pTC2nm);
		else if (strstr(pVS->shapeType, "cylinder") == pVS->shapeType)
			cylinderWrap(pMB, pVS, pVS->angleFromTo[0], pVS->angleFromTo[1], pTC, pTC2nm);
		else //if (strstr(pVS->shapeType, "cone") == pVS->shapeType)
			coneWrap(pMB, pVS, pVS->angleFromTo[0], pVS->angleFromTo[1], pTC, pTC2nm);
	}
	else if (applyTo.find("front") == 0) {
		if (strstr(pVS->shapeType, "cap") == NULL) {
			//not cap - flat circle
			vs.sections[2] = 1;
			vs.whl[2] = 0;
		}
		if (applyTo.find("wrap") != std::string::npos)
			capWrap(pMB, &vs, pVS->angleFromTo[0], pVS->angleFromTo[1], pTC, pTC2nm);
		else {
			capWrap(pMB, &vs, pVS->angleFromTo[0], pVS->angleFromTo[1], NULL, NULL);
			groupApplyTexture(pMB, "front", pTC, pTC2nm);
		}
		if (strstr(pVS->shapeType, "cylinder") == pVS->shapeType)
			moveGroupDg(pMB, 0, 0, 0, 0, 0, pVS->whl[2] * 0.5);
	}
	else{ //if (applyTo.find("back") == 0) {
		//flat circle
		vs.sections[2] = 1;
		vs.whl[2] = 0;
		if (applyTo.find("wrap") > 0)
			capWrap(pMB, &vs, pVS->angleFromTo[1], pVS->angleFromTo[0], pTC, pTC2nm);
		else {
			capWrap(pMB, &vs, pVS->angleFromTo[1], pVS->angleFromTo[0], NULL, NULL);
			groupApplyTexture(pMB, "front", pTC, pTC2nm);
		}
		if (strstr(pVS->shapeType, "cylinder") == pVS->shapeType)
			moveGroupDg(pMB, 0, 0, 0, 0, 0, -pVS->whl[2] * 0.5f);
	}
	releaseGroup(pMB);
	return 1;
}


int ModelBuilder::normalsCalc(ModelBuilder* pMB) {
	int totalTriangles = pMB->triangles.size();
	int totalVerts = pMB->vertices.size();
	//triangles normals
	for (int tN = totalTriangles - 1; tN >= 0; tN--) {
		Triangle01* pT = pMB->triangles.at(tN);
		if (pT->flag < 0)
			continue;
		Vertex01* pV0 = pMB->vertices.at(pT->idx[0]);
		Vertex01* pV1 = pMB->vertices.at(pT->idx[1]);
		Vertex01* pV2 = pMB->vertices.at(pT->idx[2]);
		vec4 vA;
		vec4 vB;
		for (int i = 0; i < 3; i++) {
			vA[i] = pV1->aPos[i] - pV0->aPos[i];
			vB[i] = pV2->aPos[i] - pV0->aPos[i];
		}
		vec4_mul_cross(pT->normal, vA, vB);
		pT->triangleArea = v3length(pT->normal)/2;
		if (pT->triangleArea == 0) {
			pMB->triangles.erase(pMB->triangles.begin() + tN);
			totalTriangles--;
		}
		else
			vec3_norm(pT->normal, pT->normal);
	}
	//verts normals
	for (int vN = 0; vN < totalVerts; vN++) {
		Vertex01* pV = pMB->vertices.at(vN);
		if (pV->flag < 0)
			continue;
		v3set(pV->aNormal, 0, 0, 0);
		for (int tN = totalTriangles - 1; tN >= 0; tN--) {
			Triangle01* pT = pMB->triangles.at(tN);
			if (pT->flag < 0)
				continue;
			for (int iN = 0; iN < 3; iN++)
				if (pT->idx[iN] == vN) {
					for (int i = 0; i < 3; i++)
						pV->aNormal[i] += pT->normal[i];
					break;
				}
		}
		vec3_norm(pV->aNormal, pV->aNormal);
		for(int i=0;i<3;i++)
			pV->aNormal[i] = (float)round(pV->aNormal[i] * 100.0f) / 100.0f;
	}
	//count matching normals
	for (int tN = totalTriangles - 1; tN >= 0; tN--) {
		Triangle01* pT = pMB->triangles.at(tN);
		if (pT->flag < 0)
			continue;
		Vertex01* pV0 = pMB->vertices.at(pT->idx[0]);
		Vertex01* pV1 = pMB->vertices.at(pT->idx[1]);
		Vertex01* pV2 = pMB->vertices.at(pT->idx[2]);
		pT->matchingNormalsN = 0;
		if (v3match(pV0->aNormal, pV1->aNormal)) pT->matchingNormalsN++;
		if (v3match(pV0->aNormal, pV2->aNormal)) pT->matchingNormalsN++;
		if (v3match(pV2->aNormal, pV1->aNormal)) pT->matchingNormalsN++;
	}
	//update verts
	for (int vN = 0; vN < totalVerts; vN++) {
		Vertex01* pV = pMB->vertices.at(vN);
		if (pV->flag < 0)
			continue;
		pV->triangleMatchingNormals = 0;
		pV->triangleArea = 0;
	}
	for (int tN = totalTriangles - 1; tN >= 0; tN--) {
		Triangle01* pT = pMB->triangles.at(tN);
		if (pT->flag < 0)
			continue;
		for (int i = 0; i < 3; i++) {
			Vertex01* pV = pMB->vertices.at(pT->idx[i]);
			if (pV->flag < 0)
				continue;
			pV->triangleMatchingNormals = std::max(pV->triangleMatchingNormals, pT->matchingNormalsN);
			pV->triangleArea = std::max(pV->triangleArea, pT->triangleArea);
		}
	}
	return 1;
}
int ModelBuilder::normalsMerge(ModelBuilder* pMB) {
	int totalVerts = pMB->vertices.size();
	//dump flags
	for (int vN = 0; vN < totalVerts; vN++) {
		Vertex01* pV = pMB->vertices.at(vN);
		if (pV->flag < 0)
			continue;
		pV->flag = 0;
		for(int i=0;i<3;i++)
			pV->aPos[i] = (float)round(pV->aPos[i] * 50.0f) / 50.0f;
	}
	for (int vN = 0; vN < totalVerts - 1; vN++) {
		Vertex01* pV = pMB->vertices.at(vN);
		if (pV->flag != 0)
			continue;
		if (pV->dontMerge)
			continue;

int vN00 = -1;
if (vN == vN00) {
	mylog("---vN=%d", vN);
	mylog_v3(" ", pV->aPos);
	mylog_v3(" nrm:", pV->aNormal);
	mylog(" mn=%d area=%.2f\n", pV->triangleMatchingNormals, pV->triangleArea);
}

		int matchingVertsN = 0;
		for (int vN2 = vN + 1; vN2 < totalVerts; vN2++) {
			Vertex01* pV2 = pMB->vertices.at(vN2);
			if (pV2->flag != 0)
				continue;
			if (pV2->dontMerge)
				continue;
			if (!v3match(pV->aPos, pV2->aPos))
				continue;
			if (v3dotProduct(pV->aNormal, pV2->aNormal) < 0.1)
				continue;
			//if here - merge
			matchingVertsN++;
			pV2->flag = 1; //mark for future correction (as participating in merge)
			int isV2better = -1; //-1 - not, 0 - the same, 1 - better
			if (pV->triangleMatchingNormals < pV2->triangleMatchingNormals)
				isV2better = 1; //better
			else if (pV->triangleMatchingNormals == pV2->triangleMatchingNormals) {
				if (pV->triangleArea < pV2->triangleArea * 0.25)
					isV2better = 1; //better
				else if (pV->triangleArea * 0.25 > pV2->triangleArea * 0.25)
					isV2better = -1; //worse
				else
					isV2better = 0; //the same
			}

if (vN == vN00) {
	mylog("  vN2=%d", vN2);
	mylog_v3(" ", pV2->aPos);
	mylog_v3(" nrm:", pV2->aNormal);
	mylog(" mn=%d area=%.2f isV2better=%d\n", pV2->triangleMatchingNormals, pV2->triangleArea, isV2better);
}

			if(isV2better>=0){
				pV->triangleMatchingNormals = pV2->triangleMatchingNormals;
				pV->triangleArea = std::max(pV->triangleArea, pV2->triangleArea);
				if (isV2better == 1)
					v3copy(pV->aNormal, pV2->aNormal);
				else //share
					for (int i = 0; i < 3; i++)
						pV->aNormal[i] += pV2->aNormal[i];
			}
		}
		vec3_norm(pV->aNormal, pV->aNormal);
		
if (vN == vN00) {
	mylog("===vN=%d result:", vN);
	mylog_v3(" ", pV->aPos);
	mylog_v3(" nrm:", pV->aNormal);
	mylog(" mn=%d area=%.2f\n", pV->triangleMatchingNormals, pV->triangleArea);
}

		//populate normal to matching verts
		if(matchingVertsN>0)
		for (int vN2 = vN + 1; vN2 < totalVerts; vN2++) {
			Vertex01* pV2 = pMB->vertices.at(vN2);
			if (pV2->flag != 1)
				continue;
			pV2->flag = 2;
			v3copy(pV2->aNormal, pV->aNormal);
		} 
	}
	return 1;
}
int ModelBuilder::replaceLineByGroup(ModelBuilder* pMB, std::vector<Vertex01*>* pVx, std::vector<Triangle01*>* pTr, std::vector<Vertex01*>* pGuideLine) {
	//prepare vx0 group - align=back
	GroupTransform gt;
	gt.flagAll(pVx, NULL);
	gt.buildBoundingBoxFlagged(&gt, pVx);
	if (gt.bbMin[2] != 0) {
		gt.shift[2] = -gt.bbMin[2];
		gt.transformFlagged(&gt, pVx);
		gt.shift[2] = 0;
	}
	//prepare vx0 group - mark front and back
	int vxN = pVx->size();
	for (int vN = 0; vN < vxN; vN++) {
		Vertex01* pV = pVx->at(vN);
		for (int i = 0; i < 3; i++)
			pV->aPos[i] = (float)round(pV->aPos[i] * 100.0f) / 100.0f;
		if (pV->aPos[2] == 0)
			myStrcpy_s(pV->marks, 128, "<back>");
		else// z>0
			myStrcpy_s(pV->marks, 128, "<front>");
		pV->aPos[2] = 0;
	}
	//all z coords == 0

	/////////////////////////////////////////
	//prepare guideLine
	int lineTotalN = pGuideLine->size();
	for (int vN = 0; vN < lineTotalN - 1; vN++) {
		Vertex01* pV = pGuideLine->at(vN);
		Vertex01* pV2 = pGuideLine->at(vN + 1);
		v3dirFromTo(pV->aNormal, pV->aPos, pV2->aPos);
		pV->aTangent[2] = v3lengthFromTo(pV->aPos, pV2->aPos); //save length in pV->aTangent[2]
		pV->aTangent[0] = v3pitchDg(pV->aNormal); //save pitch
		pV->aTangent[1] = v3yawDg(pV->aNormal); //save yaw
	}
	//fill last point
	Vertex01* pV2 = pGuideLine->at(lineTotalN - 1); //last vert
	Vertex01* pV0 = pGuideLine->at(lineTotalN - 2); //prev vert
	pV2->aTangent[0] = pV0->aTangent[0]; //save pitch
	pV2->aTangent[1] = pV0->aTangent[1]; //save yaw
	//round angles
	for (int vN = 0; vN < lineTotalN; vN++) {
		Vertex01* pV = pGuideLine->at(vN);
		pV->aTangent[0] = (float)round(pV->aTangent[0] * 100.0f) / 100.0f;
		pV->aTangent[1] = (float)round(pV->aTangent[1] * 100.0f) / 100.0f;
	}
	//normalize pitch/yaw to relative form
	for (int vN = 1; vN < lineTotalN; vN++) {
		Vertex01* pV0 = pGuideLine->at(vN - 1);
		Vertex01* pV = pGuideLine->at(vN);

		if (abs(pV->aTangent[0]) == 90)
			pV->aTangent[1] = pV0->aTangent[1];
		else if (abs(angleDgFromTo(pV0->aTangent[1], pV->aTangent[1])) > 90) {
			pV->aTangent[1] = angleDgNorm180(pV->aTangent[1] + 180);
			pV->aTangent[0] = angleDgNorm180(180.0f - pV->aTangent[0]);
		}
		pV->aTangent[0] = pV0->aTangent[0] + angleDgFromTo(pV0->aTangent[0], pV->aTangent[0]); //pitch
		pV->aTangent[1] = pV0->aTangent[1] + angleDgFromTo(pV0->aTangent[1], pV->aTangent[1]); //yaw
	}
	//connect planes - pitchD,yawD
	for (int vN = 1; vN < lineTotalN - 1; vN++) {
		Vertex01* pV = pGuideLine->at(vN);
		Vertex01* pV0 = pGuideLine->at(vN - 1);
		pV->aBinormal[0] = angleDgFromTo(pV0->aTangent[0], pV->aTangent[0]) / 2; //pitchD
		pV->aBinormal[1] = angleDgFromTo(pV0->aTangent[1], pV->aTangent[1]) / 2; // yawD
	}

	//all z coords == 0
	//MAIN CYCLE
	gt.flagAll(pVx, pTr);
	float workPos[3] = { 0,0,0 };
	float workAngDg[3] = { 0,0,0 };
	float workScale[3] = { 1,1,1 };
	for (int ribN = 0; ribN < lineTotalN - 1; ribN++) {
		Vertex01* pV0 = pGuideLine->at(ribN);
		Vertex01* pV2 = pGuideLine->at(ribN + 1);
		std::vector<Vertex01*> vx2;
		std::vector<Triangle01*> tr2;
		GroupTransform::cloneFlagged(NULL, &vx2, &tr2, pVx, pTr);
		//apply tUVs
		for (int vN = 0; vN < vxN; vN++) {
			Vertex01* pV = vx2.at(vN);
			if (strstr(pV->marks, "front") != NULL) { //front
				pV->aTuv[1] = pV0->aTuv[1];
				pV->aTuv2[1] = pV0->aTuv2[1];
			}
			else { //back
				pV->aTuv[1] = pV2->aTuv[1];
				pV->aTuv2[1] = pV2->aTuv2[1];
			}
		}
		//back plane first
		gt.limit2mark(&gt, "back");
		gt.flagSelection(&gt, &vx2, NULL);
		if (ribN == 0) {
			v3copy(workPos, pV0->aPos);
			v3copy(workAngDg, pV0->aTangent);
			v3setAll(workScale, 1);
		}
		v3copy(gt.shift, workPos);
		v3copy(gt.spinDg, workAngDg);
		v3copy(gt.scale, workScale);
		gt.transformFlagged(&gt, &vx2);
		gt.clear(&gt);

		//now - front plane
		gt.limit2mark(&gt, "front");
		gt.flagSelection(&gt, &vx2, NULL);
		if (ribN == (lineTotalN - 2)) {
			//last rib
			v3copy(workPos, pV2->aPos);
			v3copy(workAngDg, pV2->aTangent);
			v3setAll(workScale, 1);
		}
		else { //normal rib
			v3copy(workPos, pV2->aPos);
			for (int i = 0; i < 2; i++)
				workAngDg[i] = angleDgNorm180(pV0->aTangent[i] + pV2->aBinormal[i]);
			if (pV2->aBinormal[0] != 0)
				workScale[1] = 1.0f / cosf(abs(pV2->aBinormal[0]) * degrees2radians);
			if (pV2->aBinormal[1] != 0)
				workScale[0] = 1.0f / cosf(abs(pV2->aBinormal[1]) * degrees2radians);
		}
		v3copy(gt.shift, workPos);
		v3copy(gt.spinDg, workAngDg);
		v3copy(gt.scale, workScale);
		gt.transformFlagged(&gt, &vx2);
		gt.clear(&gt);

		//save to ModelBuilder
		gt.flagAll(&vx2, &tr2);
		gt.cloneFlagged(pMB, &pMB->vertices, &pMB->triangles, &vx2, &tr2);
		//clear memory
		int totalN = vx2.size();
		for (int vN = totalN - 1; vN >= 0; vN--)
			delete vx2.at(vN);
		vx2.clear();
		totalN = tr2.size();
		for (int tN = totalN - 1; tN >= 0; tN--)
			delete tr2.at(tN);
		tr2.clear();
	}
	return 1;
}
int ModelBuilder::lineDirFromN2N(float* pDir, int vN0, int vN2, std::vector<Vertex01*>* pVx, int lineStartsAt) {
	if (vN2 <= lineStartsAt) { //vN2 is a first point
		v3setAll(pDir, 0);
		return 1;
	}
	int totalN = pVx->size();
	if (vN0 >= totalN - 1) { //vN0 is a last point
		v3setAll(pDir, 0);
		return 1;
	}
	//normal point
	Vertex01* pV0 = pVx->at(vN0);
	Vertex01* pV2 = pVx->at(vN2);
	for (int i = 0; i < 3; i++)
		pDir[i] = pV2->aPos[i] - pV0->aPos[i];
	v3norm(pDir);
	return 1;
}
int ModelBuilder::fillLineCurveTo(int vN2, std::vector<Vertex01*>* pVx, int lineStartsAt) {
	float dMax = 0.8f;
	if (vN2 <= lineStartsAt)
		return 0;
	Vertex01* pV2 = pVx->at(vN2);
	int vN0 = vN2 - 1;
	Vertex01* pV0 = pVx->at(vN0);

	float dist0 = v3lengthFromTo(pV2->aPos, pV0->aPos);
	if (dist0 <= dMax)
		return 0;

	int ribsN = (int)ceil(dist0 / dMax);
	for (int ribN = ribsN - 1; ribN > 0; ribN--) {
		Vertex01* pV = new Vertex01(*pV0);
		pV->endOfSequence = 0;
		pV->rad = 0;
		pVx->insert(pVx->begin() + vN2, pV);

		float progress = (float)ribN / ribsN;

		float weight2 = (float)pow(progress, 2);
		float weight0 = (float)pow(1.0 - progress, 2);
		float k0 = weight0 / (weight0 + weight2);
		float k2 = 1.0f - k0;
		for (int i = 0; i < 3; i++) {
			pV->aPos[i] = (pV0->aPos[i] + pV0->aNormal[i] * dist0 * progress) * k0 +
				(pV2->aPos[i] - pV2->aNormal[i] * dist0 * (1.0f - progress)) * k2;
		}
		int a = 0;
	}
	return ribsN - 1;
}
int ModelBuilder::finalizeLine(ModelBuilder* pMB, int lineStartsAt) {
	//assuming that the line is uninterrupted
	int totalN = pMB->vertices.size();
	for (int vN = pMB->vertices.size() - 1; vN >= lineStartsAt; vN--) {
		Vertex01* pV = pMB->vertices.at(vN);
		if (pV->rad > 0) {
			//have a curve
			//next point
			Vertex01* pV2 = pV;
			if (vN < totalN - 1) {
				pV2 = pMB->vertices.at(vN + 1);
				float dist0 = v3lengthFromTo(pV->aPos, pV2->aPos);
				if (dist0 > pV->rad) {
					//dist > rad - add shifted back point
					pV2 = new Vertex01(*pV2);
					pMB->vertices.insert(pMB->vertices.begin() + vN + 1, pV2);
					float k = pV->rad / dist0;
					for (int i = 0; i < 3; i++) {
						float fromV = pV2->aPos[i] - pV->aPos[i];
						pV2->aPos[i] = pV->aPos[i] + (fromV * k);
					}
					pV2->endOfSequence = 0;
					pV2->rad = 0;
				}
			}
			//prev point
			Vertex01* pV0 = pV;
			if (vN > lineStartsAt) {
				pV0 = pMB->vertices.at(vN - 1);
				float dist0 = v3lengthFromTo(pV->aPos, pV0->aPos);
				if (dist0 > pV->rad) {
					//dist > rad - add shifted back point
					pV0 = new Vertex01(*pV0);
					pMB->vertices.insert(pMB->vertices.begin() + vN, pV0);
					vN++;
					float k = pV->rad / dist0;
					for (int i = 0; i < 3; i++) {
						float fromV = pV0->aPos[i] - pV->aPos[i];
						pV0->aPos[i] = pV->aPos[i] + (fromV * k);
					}
					pV0->endOfSequence = 0;
					pV0->rad = 0;
				}
			}
			if (pV != pV0 && pV != pV2) {
				delete pV;
				pMB->vertices.erase(pMB->vertices.begin() + vN);
			}
			//calculate pV0 dir from prev point
			lineDirFromN2N(pV0->aNormal, vN - 2, vN - 1, &pMB->vertices, lineStartsAt);
			//calculate pV2 dir to next point
			lineDirFromN2N(pV2->aNormal, vN, vN + 1, &pMB->vertices, lineStartsAt);

			fillLineCurveTo(vN, &pMB->vertices, lineStartsAt);
		}
		else if (pV->rad == -2) {
			//just curve
			delete pV;
			pMB->vertices.erase(pMB->vertices.begin() + vN);
			Vertex01* pV0 = pMB->vertices.at(vN - 1);
			Vertex01* pV2 = pMB->vertices.at(vN);
			//calculate pV0 dir from prev point
			lineDirFromN2N(pV0->aNormal, vN - 2, vN - 1, &pMB->vertices, lineStartsAt);
			//calculate pV2 dir to next point
			lineDirFromN2N(pV2->aNormal, vN, vN + 1, &pMB->vertices, lineStartsAt);

			fillLineCurveTo(vN, &pMB->vertices, lineStartsAt);
		}
		else if (pV->rad == -1) {
			//pass-through point
			//next point
			Vertex01* pV2 = pV;
			float dToV2 = 0;
			float dirToV2[3] = { 0,0,0 };
			if (vN < totalN - 1) {
				pV2 = pMB->vertices.at(vN + 1);
				dToV2 = v3lengthFromTo(pV->aPos, pV2->aPos);
				v3dirFromTo(dirToV2, pV->aPos, pV2->aPos);
				lineDirFromN2N(pV2->aNormal, vN + 1, vN + 2, &pMB->vertices, lineStartsAt);
			}
			//prev point
			Vertex01* pV0 = pV;
			float dFromV0 = 0;
			float dirFromV0[3] = { 0,0,0 };
			if (vN > lineStartsAt) {
				pV0 = pMB->vertices.at(vN - 1);
				dFromV0 = v3lengthFromTo(pV0->aPos, pV->aPos);
				v3dirFromTo(dirFromV0, pV0->aPos, pV->aPos);
				lineDirFromN2N(pV0->aNormal, vN - 2, vN - 1, &pMB->vertices, lineStartsAt);
			}
			float k0 = dToV2 / (dFromV0 + dToV2);
			float k2 = 1.0f - k0;
			for (int i = 0; i < 3; i++)
				pV->aNormal[i] = dirFromV0[i] * k0 + dirToV2[i] * k2;
			v3norm(pV->aNormal);

			fillLineCurveTo(vN + 1, &pMB->vertices, lineStartsAt);
			fillLineCurveTo(vN, &pMB->vertices, lineStartsAt);
		}
	}
	return 1;
}


