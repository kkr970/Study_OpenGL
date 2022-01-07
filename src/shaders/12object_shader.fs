#version 460 core
struct Material{
   sampler2D diffuse;
   sampler2D specular;
   sampler2D emission;
   float shininess;
};
struct DirLight{
   vec3 direction;

   vec3 ambient;
   vec3 diffuse;
   vec3 specular;
};
struct PointLight{
   vec3 position;
   
   float constant;
   float linear;
   float quadratic;

   vec3 ambient;
   vec3 diffuse;
   vec3 specular;
};
struct SpotLight{
   vec3 position;
   vec3 direction;
   float cutOff;
   float outerCutOff;

   vec3 ambient;
   vec3 diffuse;
   vec3 specular;

   float constant;
   float linear;
   float quadratic;
};

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

out vec4 FragColor;

#define NR_POINT_LIGHT 4

uniform vec3 viewPos;
uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHT];
uniform SpotLight spotLight;
uniform bool isOnSpotLight;

vec3 CalcDirLight(DirLight light, vec3 normal,vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 FragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 FragPos, vec3 viewDir);

void main()
{
   vec3 norm = normalize(Normal);
   vec3 viewDir = normalize(viewPos - FragPos);

   // Directional Light
   vec3 result = CalcDirLight(dirLight, norm, viewDir);
   // Point Light
   for(int i = 0 ; i < NR_POINT_LIGHT ; i++)
      result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
   // Spot Light
   if(isOnSpotLight)
      result += CalcSpotLight(spotLight, norm, FragPos, viewDir);

   // Emission
   vec3 show = step(vec3(1.0f), vec3(1.0f) -  vec3(texture(material.specular, TexCoords)));
   vec3 emission = vec3(texture(material.emission, TexCoords)) * show;
   result += emission;
   FragColor = vec4(result, 1.0f);
}

// Dirlight 계산
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
   vec3 lightDir = normalize(-light.direction);
   // diffuse shading
   float diff = max(dot(normal, lightDir), 0.0);
   // specular shading
   vec3 reflectDir = reflect(-lightDir, normal);
   float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
   // 결과들을 결합
   vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
   vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
   vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));

   return (ambient + diffuse + specular);
}

// PointLight 계산
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
   vec3 lightDir = normalize(light.position - fragPos);
   // diffuse shading
   float diff = max(dot(normal, lightDir), 0.0);
   // specular shading
   vec3 reflectDir = reflect(-lightDir, normal);
   float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
   // attenuation
   float distance    = length(light.position - fragPos);
   float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
   // 결과들을 결합
   vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
   vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
   vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
   ambient *= attenuation;
   diffuse *= attenuation;
   specular *= attenuation;

   return (ambient + diffuse + specular);
} 

// Spot Light 계산
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir){
   vec3 lightDir = normalize(light.position - fragPos);
   // diffuse shading
   float diff = max(dot(normal, lightDir), 0.0);
   // specular shading
   vec3 reflectDir = reflect(-lightDir, normal);
   float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
   // attenuation
   float distance = length(light.position - fragPos);
   float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
   // spotlight intensity
   float theta = dot(lightDir, normalize(-light.direction)); 
   float epsilon = light.cutOff - light.outerCutOff;
   float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
   // combine results
   vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
   vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
   vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
   ambient *= attenuation * intensity;
   diffuse *= attenuation * intensity;
   specular *= attenuation * intensity;

   return (ambient + diffuse + specular);
}
