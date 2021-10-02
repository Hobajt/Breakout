#version 450 core
out vec4 FragColor;

#define MAX_TEXTURES 8

in vec4 color;
in vec2 texCoords;
in vec2 texTiling;
in flat float textureID;

uniform sampler2D textures[MAX_TEXTURES];

void main() {
    int tID = int(textureID);
    vec4 tColor = vec4(1.0);
    switch(tID) {
        case 0: tColor = texture(textures[0], texCoords * texTiling); break;
        case 1: tColor = texture(textures[1], texCoords * texTiling); break;
        case 2: tColor = texture(textures[2], texCoords * texTiling); break;
        case 3: tColor = texture(textures[3], texCoords * texTiling); break;
        case 4: tColor = texture(textures[4], texCoords * texTiling); break;
        case 5: tColor = texture(textures[5], texCoords * texTiling); break;
        case 6: tColor = texture(textures[6], texCoords * texTiling); break;
        case 7: tColor = texture(textures[7], texCoords * texTiling); break;
    }

    FragColor = color * tColor;
    // FragColor = vec4(vec3(tID == 0), 1.0);
}