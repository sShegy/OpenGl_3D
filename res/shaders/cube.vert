#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aColor;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 FaceColor;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    TexCoords = aTexCoords;
    FaceColor = aColor;

    vec3 N = normalize(vec3(model * vec4(aNormal, 0.0)));


    vec3 up = abs(N.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);

    vec3 T = normalize(cross(up, N));
    vec3 B = cross(N, T);

    TBN = mat3(T, B, N);

    gl_Position = projection * view * vec4(FragPos, 1.0);
}