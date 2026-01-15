#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;
in vec3 FaceColor;
in mat3 TBN;

uniform sampler2D texture1;
uniform sampler2D normalMap;
uniform samplerCube skybox;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{

    vec3 normal = texture(normalMap, TexCoords).rgb;
    normal = normal * 2.0 - 1.0;
    normal = normalize(TBN * normal);

    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    vec3 ambient = 0.3 * vec3(1.0);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0);


    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = vec3(0.5) * spec;


    vec3 R = reflect(-viewDir, normal);
    vec3 reflectionColor = texture(skybox, R).rgb * 0.3;

    vec4 texColor = texture(texture1, TexCoords);
    vec3 objectColor = mix(texColor.rgb, FaceColor, 0.7);

    vec3 result = (ambient + diffuse) * objectColor + specular + reflectionColor;

    FragColor = vec4(result, 1.0);
}