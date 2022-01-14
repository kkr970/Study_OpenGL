#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D planeTexture;
uniform sampler2D cubeTexture;
uniform sampler2D grassTexture;
uniform sampler2D windowTexture;
uniform int textureType;

void main()
{    
    switch(textureType)
    {
        case 0:
            FragColor = texture(planeTexture, TexCoords);
            break;
        case 1:
            FragColor = texture(cubeTexture, TexCoords);
            break;
        case 2:
            vec4 texColor = texture(grassTexture, TexCoords);
            if(texColor.a < 0.1)
                discard;
            FragColor = texColor;
            break;
        case 3:
            FragColor = texture(windowTexture, TexCoords);
            break;
        default:
            FragColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
            break;
    }
    
}