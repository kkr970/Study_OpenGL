#version 460 core
struct Material{
   sampler2D diffuse;
   sampler2D specular;
   sampler2D emission;
   float shininess;
};
struct Light{
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

uniform Material material;
uniform Light light;
uniform vec3 viewPos;

void main()
{
   vec3 lightDir = normalize(light.position - FragPos);

   //spotlight
   float theta = dot(lightDir, normalize(-light.direction));

   if(theta > light.outerCutOff)
   {
      // ambient
      vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
      
      // diffuse 
      vec3 norm = normalize(Normal);
      float diff = max(dot(norm, lightDir), 0.0);
      vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
      
      // specular
      vec3 viewDir = normalize(viewPos - FragPos);
      vec3 reflectDir = reflect(-lightDir, norm);  
      float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
      vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));  
      
      // emission
      vec3 show = step(vec3(1.0f), vec3(1.0) -  vec3(texture(material.specular, TexCoords)));
      vec3 emission = vec3(texture(material.emission, TexCoords)) * show;

      // spotlight
      float epsilon = (light.cutOff - light.outerCutOff);
      float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
      diffuse *= intensity;
      specular *= intensity;

      // attenuation
      float distance = length(light.position - FragPos);
      float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
      ambient *= attenuation; 
      diffuse *= attenuation;
      specular *= attenuation;   
         
      vec3 result = ambient + diffuse + specular + emission;
      FragColor = vec4(result, 1.0);
   }
   else
   {
      FragColor = vec4(light.ambient * vec3(texture(material.diffuse, TexCoords)), 1.0);
   }
}