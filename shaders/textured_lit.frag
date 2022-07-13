#version 450

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 FragPos;
layout (location = 3) in vec3 Normal;
layout (location = 4) in vec3 viewPos;

layout (location = 0) out vec4 outFragColor;

layout(set = 0, binding = 1) uniform  SceneData{
    vec4 fogColor;
    vec4 fogDistances;
    vec4 ambientColor;
    vec4 sunlightPos;
    vec4 sunlightColor;
} sceneData;

layout( push_constant ) uniform constants
{
    vec3 camPos;
} PushConstants;


layout(set = 2, binding = 0) uniform sampler2D tex1;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(sceneData.sunlightPos.xyz - FragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * sceneData.sunlightColor.xyz;

    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * sceneData.sunlightColor.xyz;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8);

    vec3 d = {0, 255, 0};
    vec3 specular = specularStrength * spec * sceneData.sunlightColor.xyz;

    vec3 color = texture(tex1,texCoord).xyz;

    vec3 result = (ambient + diffuse + specular) * color;
    outFragColor = vec4(result,1.0f);
}