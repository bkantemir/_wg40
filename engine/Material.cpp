#include "Material.h"
#include "Shader.h"
#include "platform.h"

Material::~Material() {
}


int Material::pickShaderNumber(Material* pMT) {
    int shadersN = Shader::shaders.size();
    if (strcmp(pMT->shaderType, "flat") == 0)
        pMT->takesShadow = 0;
    for (int i = 0; i < shadersN; i++) {
        Shader* pSH = Shader::shaders.at(i);
        if (strcmp(pMT->shaderType, pSH->shaderType) != 0)
            continue;
        if (pMT->uColor.isZero() != (pSH->l_uColor < 0))
            continue;
        if ((pMT->uTex0 < 0) != (pSH->l_uTex0 < 0))
            continue;
        if ((pMT->uTex1mask < 0) != (pSH->l_uTex1mask < 0))
            continue;
        if ((pMT->uTex2nm < 0) != (pSH->l_uTex2nm < 0))
            continue;
        if ((pMT->takesShadow < 1) != (pSH->l_uTex4dm < 0))
            continue;
        if ((strcmp(pMT->shaderType, "flat") == 0) != (pSH->l_aNormal < 0))
            continue;
        pMT->shaderN = i;
        return i;
    }
    mylog("ERROR in Material::pickShaderNumber:\n");
    mylog("Can't find '%s' shader for uColor=%08x uTex0=%d uTex1mask=%d uTex2nm=%d takesShadow=%d\n",
        pMT->shaderType, pMT->uColor.getUint32(), pMT->uTex0, pMT->uTex1mask, pMT->uTex2nm, pMT->takesShadow);
    return -1;
}
void Material::clear(Material* pMT) {
    Material mtZero;
    memcpy(pMT, &mtZero, sizeof(Material));
}
int Material::assignShader(Material* pMT, std::string shaderType) {
    setShaderType(pMT, shaderType);
    return pickShaderNumber(pMT);
}

