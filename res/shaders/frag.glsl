#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec3 FaceColor;

uniform sampler2D texture1;
uniform vec3 lightPos; // Pozicija sunca
uniform vec3 viewPos;  // Pozicija kamere

void main() {
    vec3 lightColor = vec3(1.0, 0.95, 0.9);


    float ambientStrength = 0.4;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(Normal);

    vec3 lightDir = normalize(lightPos - vec3(0.0, 0.0, 0.0));
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;


    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 64.0);
    vec3 specular = specularStrength * spec * lightColor;


    vec4 texColor = texture(texture1, TexCoords);
    vec3 objectColor = texColor.rgb * FaceColor;

    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}