#include "DrawJob.h"
#include "platform.h"
#include "utils.h"
#include "Shader.h"
#include "Texture.h"
#include "Shadows.h"

//static arrays (vectors) of all loaded DrawJobs, VBO ids
std::vector<DrawJob*> DrawJob::drawJobs;
std::vector<unsigned int> DrawJob::buffersIds;

DrawJob::DrawJob() {
	drawJobs.push_back(this);
}
DrawJob::~DrawJob() {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	if (glVAOid > 0) {
		glDeleteVertexArrays(1, &glVAOid);
		glVAOid = 0;
	}
}
int DrawJob::newBufferId() {
	unsigned int bufferId;
	glGenBuffers(1, &bufferId);
	buffersIds.push_back(bufferId);
	return (int)bufferId;
}
unsigned int activeVBOid;
int DrawJob::buildVAOforShader(DrawJob* pDJ, int shaderN) {
	if (shaderN < 0)
		return 0;
	unsigned int* pVAOid = &pDJ->glVAOid;
	//delete VAO if exists already
	if (*pVAOid > 0) {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteVertexArrays(1, pVAOid);
	}
	glGenVertexArrays(1, pVAOid);
	glBindVertexArray(*pVAOid);

	//open shader descriptor to access variables locations
	Shader* pShader = Shader::shaders.at(shaderN);

	activeVBOid = 0;
	attachAttribute(pShader->l_aPos, 3, &pDJ->aPos);
	attachAttribute(pShader->l_aNormal, 3, &pDJ->aNormal);
	attachAttribute(pShader->l_aTuv, 2, &pDJ->aTuv);
	attachAttribute(pShader->l_aTuv2, 2, &pDJ->aTuv2); //for normal map
	attachAttribute(pShader->l_aTangent, 3, &pDJ->aTangent); //for normal map
	attachAttribute(pShader->l_aBinormal, 3, &pDJ->aBinormal); //for normal map

	if (pDJ->glEBOid > 0)
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pDJ->glEBOid);

	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return 1;
}

int DrawJob::attachAttribute(int varLocationInShader, int attributeSizeInFloats, AttribRef* pAR) {
	if (varLocationInShader < 0)
		return 0; //not used in this shader
	if (pAR->glVBOid == 0) {
		mylog("ERROR in DrawJob::attachAttribute, nk such attribute/VBO\n");
		return -1;
	}
	glEnableVertexAttribArray(varLocationInShader);
	if (activeVBOid != pAR->glVBOid) {
		activeVBOid = pAR->glVBOid;
		//attach input stream data
		glBindBuffer(GL_ARRAY_BUFFER, activeVBOid);
	}
	glVertexAttribPointer(varLocationInShader, attributeSizeInFloats, GL_FLOAT, GL_FALSE, pAR->stride, (void*)(long)pAR->offset);
	return 1;
}
int DrawJob::setAttribRef(AttribRef* pAR, unsigned int glVBOid, int offset, int stride) {
	pAR->glVBOid = glVBOid;
	pAR->offset = offset;
	pAR->stride = stride;
	return 1;
}
int DrawJob::executeDrawJob(DrawJob* pDJ, float* uMVP, float* uMV3x3, float* uMM, float* uMVP4dm, float* uVectorToLight, 
		float* uCameraPosition, float sizeUnitPixelsSize, Material* pMt) {
	if (pMt == NULL)
		pMt = &(pDJ->mt);

	int shaderN = pMt->shaderN;
	if (shaderN < 0)
		return 0;

	if (lineWidthIsImportant(pMt->primitiveType)) {
		float lw = sizeUnitPixelsSize * pMt->lineWidth;
		if (lw < 0.8)
			return 0;
		glLineWidth(lw);
	}
	glBindVertexArray(pDJ->glVAOid);

	Shader* pShader = Shader::shaders.at(shaderN);
	glUseProgram(pShader->GLid);
	//input uniforms
	glUniformMatrix4fv(pShader->l_uMVP, 1, GL_FALSE, (const GLfloat*)uMVP);
	if (pShader->l_uMV3x3 >= 0) {
		glUniformMatrix3fv(pShader->l_uMV3x3, 1, GL_FALSE, (const GLfloat*)uMV3x3);
		if (pShader->l_uDepthBias >= 0) //for depthMap
			glUniformMatrix4fv(pShader->l_uDepthBias, 1, GL_FALSE, (const GLfloat*)Shadows::uDepthBias);
	}
	if (pShader->l_uMM >= 0)
		glUniformMatrix4fv(pShader->l_uMM, 1, GL_FALSE, (const GLfloat*)uMM);
	if (pShader->l_uVectorToLight >= 0)
		glUniform3fv(pShader->l_uVectorToLight, 1, (const GLfloat*)uVectorToLight);
	if (pShader->l_uVectorToLightDiffuse >= 0) {
		if(pMt->gem == 0)
			glUniform3fv(pShader->l_uVectorToLightDiffuse, 1, (const GLfloat*)uVectorToLight);
		else {
			float mirrorLight[3];
			for (int i = 0; i < 3; i++) {
				if (i == 2)
					mirrorLight[i] = uVectorToLight[i];// *0.7;
				else
					mirrorLight[i] = -uVectorToLight[i];
			}
			//v3norm(mirrorLight);
			glUniform3fv(pShader->l_uVectorToLightDiffuse, 1, (const GLfloat*)mirrorLight);
		}
	}
	if (pShader->l_uCameraPosition >= 0)
		glUniform3fv(pShader->l_uCameraPosition, 1, (const GLfloat*)uCameraPosition);

	//attach textures
	if (pShader->l_uTex0 >= 0) {
		int textureId = Texture::getGLid(pMt->uTex0);
		
		//if (pMt->uTex0 == Shadows::depthMapTexN)
		//	textureId = Texture::textures.at(pMt->uTex0)->depthBufferId;
		

		//pass textureId to shader program
		glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
		glBindTexture(GL_TEXTURE_2D, textureId);
		// Tell the texture uniform sampler to use this texture in the shader by binding to texture unit 0.    
		glUniform1i(pShader->l_uTex0, 0);
	}
	if (pShader->l_uTex1mask >= 0) {
		int textureId = Texture::getGLid(pMt->uTex1mask);
		//pass textureId to shader program
		glActiveTexture(GL_TEXTURE1); // activate the texture unit first before binding texture
		glBindTexture(GL_TEXTURE_2D, textureId);
		// Tell the texture uniform sampler to use this texture in the shader by binding to texture unit 1.    
		glUniform1i(pShader->l_uTex1mask, 1);
	}
	if (pShader->l_uTex2nm >= 0) {
		int textureId = Texture::getGLid(pMt->uTex2nm);
		//pass textureId to shader program
		glActiveTexture(GL_TEXTURE2); // activate the texture unit first before binding texture
		glBindTexture(GL_TEXTURE_2D, textureId);
		// Tell the texture uniform sampler to use this texture in the shader by binding to texture unit 2.    
		glUniform1i(pShader->l_uTex2nm, 2);
	}
	if (pShader->l_uTex0translateChannelN >= 0) {
		glUniform1i(pShader->l_uTex0translateChannelN, pMt->uTex0translateChannelN);
		if (pMt->uTex0translateChannelN >= 4) {
			//translate to 2-tone
			if (pShader->l_uColor1 >= 0) {
				glUniform4fv(pShader->l_uColor1, 1, pMt->uColor1.forGL());
				glUniform4fv(pShader->l_uColor2, 1, pMt->uColor2.forGL());
			}
		}
		else 
		if (pShader->l_uTex3 >= 0 && pMt->uTex3 >= 0) {
			//translate to texture
			int textureId = Texture::getGLid(pMt->uTex3);
			//pass textureId to shader program
			glActiveTexture(GL_TEXTURE3); // activate the texture unit first before binding texture
			glBindTexture(GL_TEXTURE_2D, textureId);
			// Tell the texture uniform sampler to use this texture in the shader by binding to texture unit 3.    
			glUniform1i(pShader->l_uTex3, 3);
		}
	}
	//shadow uniforms
	if (pShader->l_uTex4dm >= 0) {
		//translate to texture
		int textureId = Texture::getGLid(Shadows::depthMapTexN);
		//int textureId = Texture::textures.at(Shadows::depthMapTexN)->depthBufferId;
		//pass textureId to shader program
		glActiveTexture(GL_TEXTURE4); // activate the texture unit first before binding texture
		glBindTexture(GL_TEXTURE_2D, textureId);
		// Tell the texture uniform sampler to use this texture in the shader by binding to texture unit 4.    
		glUniform1i(pShader->l_uTex4dm, 4);

		glUniform1f(pShader->l_uShadow, Shadows::shadowLight);
		glUniformMatrix4fv(pShader->l_uMVP4dm, 1, GL_FALSE, (const GLfloat*)uMVP4dm);
	}
	//material uniforms
	if (pShader->l_uTex1alphaChannelN >= 0)
		glUniform1i(pShader->l_uTex1alphaChannelN, pMt->uTex1alphaChannelN);
	if (pShader->l_uTex1alphaNegative >= 0)
		glUniform1i(pShader->l_uTex1alphaNegative, pMt->uTex1alphaNegative);
	if (pShader->l_uColor >= 0)
		glUniform4fv(pShader->l_uColor, 1, pMt->uColor.forGL());
	if (pShader->l_uAlphaFactor >= 0)
		glUniform1f(pShader->l_uAlphaFactor, pMt->uAlphaFactor);
	if (pShader->l_uAlphaBlending >= 0)
		glUniform1i(pShader->l_uAlphaBlending, pMt->uAlphaBlending);
	if (pShader->l_uAmbient >= 0)
		glUniform1f(pShader->l_uAmbient, pMt->uAmbient);
	if (pShader->l_uSpecularIntencity >= 0)
		glUniform1f(pShader->l_uSpecularIntencity, pMt->uSpecularIntencity);
	if (pShader->l_uSpecularMinDot >= 0)
		glUniform1f(pShader->l_uSpecularMinDot, pMt->uSpecularMinDot);
	if (pShader->l_uSpecularPowerOf >= 0)
		glUniform1f(pShader->l_uSpecularPowerOf, pMt->uSpecularPowerOf);
	if (pShader->l_uBleach >= 0)
		glUniform1f(pShader->l_uBleach, pMt->uBleach);
	if (pShader->l_uDiscardNormalsOut >= 0)
		glUniform1i(pShader->l_uDiscardNormalsOut, pMt->uDiscardNormalsOut);

	//adjust render settings
	if (pMt->zBuffer > 0) {
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
	}
	else
		glDisable(GL_DEPTH_TEST);

	if (pMt->zBufferUpdate > 0)
		glDepthMask(GL_TRUE);
	else
		glDepthMask(GL_FALSE);

	if (pShader->l_uAlphaBlending >= 0 && pMt->uAlphaBlending > 0) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else
		glDisable(GL_BLEND);


	//execute
	if (pDJ->glEBOid == 0) {
		glDrawArrays(pMt->primitiveType, 0, pDJ->pointsN);
	}
	else { //use EBO
		glDrawElements(pMt->primitiveType, pDJ->pointsN, GL_UNSIGNED_SHORT, 0);
	}
	glBindVertexArray(0);
	return 1;
}
int DrawJob::cleanUp() {
	int itemsN = drawJobs.size();
	//delete all drawJobs
	for (int i = 0; i < itemsN; i++) {
		DrawJob* pDJ = drawJobs.at(i);
		delete pDJ;
	}
	drawJobs.clear();
	//delete Buffers
	itemsN = buffersIds.size();
	//delete all buffers
	for (int i = 0; i < itemsN; i++) {
		unsigned int id = buffersIds.at(i);
		glDeleteBuffers(1, &id);
	}
	buffersIds.clear();

	return 1;
}



int DrawJob::setDesirableOffsetsForSingleVBO(DrawJob* pDJ, int* pStride, int shaderN, int VBOid) {
	//sets desirable offsets and stride according to given shader needs
	//assuming that we have 1 single VBO
	Shader* pSh = Shader::shaders.at(shaderN);
	int stride = 0;
	pDJ->aPos.offset = 0; //attribute o_aPos, always 0
	stride += sizeof(float) * 3; //aPos size - 3 floats (x,y,z)
	if (pSh->l_aNormal >= 0) { //attribute normal
		pDJ->aNormal.offset = stride;
		stride += sizeof(float) * 3;
	}
	if (pSh->l_aTuv >= 0) { //attribute TUV (texture coordinates)
		pDJ->aTuv.offset = stride; //attribute TUV (texture coordinates)
		stride += sizeof(float) * 2;
	}
	if (pSh->l_aTuv2 >= 0) { //for normal map
		pDJ->aTuv2.offset = stride;
		stride += sizeof(float) * 2;
	}
	if (pSh->l_aTangent >= 0) { //for normal map
		pDJ->aTangent.offset = stride;
		stride += sizeof(float) * 3;
	}
	if (pSh->l_aBinormal >= 0) { //for normal map
		pDJ->aBinormal.offset = stride;
		stride += sizeof(float) * 3;
	}
	*pStride = stride;
	//add stride and VBOid to all attributes
	AttribRef* pAR = NULL;
	pAR = &pDJ->aPos; pAR->glVBOid = VBOid; pAR->stride = stride;
	pAR = &pDJ->aNormal; pAR->glVBOid = VBOid; pAR->stride = stride;
	pAR = &pDJ->aTuv; pAR->glVBOid = VBOid; pAR->stride = stride;
	pAR = &pDJ->aTuv2; pAR->glVBOid = VBOid; pAR->stride = stride;
	pAR = &pDJ->aTangent; pAR->glVBOid = VBOid; pAR->stride = stride;
	pAR = &pDJ->aBinormal; pAR->glVBOid = VBOid; pAR->stride = stride;

	return 1;
}
bool DrawJob::lineWidthIsImportant(int primitiveType) {
	if (primitiveType == GL_TRIANGLES) return false;
	if (primitiveType == GL_TRIANGLE_STRIP) return false;
	if (primitiveType == GL_TRIANGLE_FAN) return false;
	return true;
}
