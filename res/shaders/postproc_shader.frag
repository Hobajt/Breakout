#version 450 core
out vec4 FragColor;

#define MAX_TEXTURES 8

in vec4 color;
in vec2 texCoords;
in vec2 texTiling;
in flat float textureID;
in flat float alphaTexture;

uniform sampler2D textures[MAX_TEXTURES];

uniform float offset = 1.0 / 300.0;
uniform vec2 shakeVec = vec2(0);

// const float offset = 1.0 / 300.0;
vec2 offsets[9] = vec2[](
    vec2(-offset,  offset), // top-left
    vec2( 0.0f,    offset), // top-center
    vec2( offset,  offset), // top-right
    vec2(-offset,  0.0f),   // center-left
    vec2( 0.0f,    0.0f),   // center-center
    vec2( offset,  0.0f),   // center-right
    vec2(-offset, -offset), // bottom-left
    vec2( 0.0f,   -offset), // bottom-center
    vec2( offset, -offset)  // bottom-right    
);

float kernel_blur[9] = float[](
    1.0 / 16, 2.0 / 16, 1.0 / 16,
    2.0 / 16, 4.0 / 16, 2.0 / 16,
    1.0 / 16, 2.0 / 16, 1.0 / 16  
);

uniform int effect = 0;

void main() {
    vec2 tc = vec2(texCoords.x, 1 - texCoords.y);

    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++) {
        sampleTex[i] = vec3(texture(textures[1], tc + shakeVec + offsets[i]));
    }

    vec4 color;
    vec3 clr = vec3(0.0);
    switch(effect) {
        default:
        case 0:     //none
            color = texture(textures[1], tc);
            break;
        case 1:     //blur + shake
            for(int i = 0; i < 9; i++)
                clr += sampleTex[i] * kernel_blur[i];
            color = vec4(clr, 1.0);
            break;
        case 2:     //drunk
            for(int i = 0; i < 9; i++)
                clr += sampleTex[i] * kernel_blur[i];
            color = vec4(clr, 1.0);
            break;
        case 3:     //chaos - vertical offset
            tc.y += 0.3;
            color = texture(textures[1], tc);
            break;
        case 4:     //confuse - flip vertically and invert colors
            color = vec4(1 - texture(textures[1], texCoords).rgb, 1.0);
            break;
    }

    FragColor = color;
}