#version 450 core

layout(location = 0) in vec3  aPosition;
layout(location = 1) in vec4  aColor;
layout(location = 2) in vec2  aTexCoords;
layout(location = 3) in vec2  aTexTiling;
layout(location = 4) in float aTextureID;
layout(location = 5) in float aAlphaTexture;

out vec4 color;
out vec2 texCoords;
out vec2 texTiling;
out flat float textureID;
out flat float alphaTexture;

void main() {
    gl_Position = vec4(aPosition, 1.0);
    color       = aColor;
    texCoords   = aTexCoords;
    texTiling   = aTexTiling;
    textureID   = aTextureID;
    alphaTexture = aAlphaTexture;
}