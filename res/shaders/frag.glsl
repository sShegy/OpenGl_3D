#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec3 ObjectColor;

uniform sampler2D texture1;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    // 1. Ambient (Osnovno svetlo)
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0);

    // 2. Diffuse (Svetlo koje pada pod uglom)
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);

    // 3. Specular (Odsjaj plastike)
    float specularStrength = 0.8;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64); // 64 je shininess
    vec3 specular = specularStrength * spec * vec3(1.0, 1.0, 1.0);

    // Miksamo teksturu (blago vidljiva) sa bojom kocke
    // Ako ne zelis teksturu, obrisi "* vec3(texColor)"
    vec4 texColor = texture(texture1, TexCoords);

    // Konacna boja
    vec3 result = (ambient + diffuse + specular) * ObjectColor * vec3(texColor);
    FragColor = vec4(result, 1.0);
}