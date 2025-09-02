#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

// 材质属性
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform bool hasTexture;

// 光照属性
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

void main()
{    
    // 环境光
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;
    
    // 漫反射光
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // 镜面反射光
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    vec3 specular = specularStrength * spec * lightColor;
    
    // 获取纹理颜色
    vec3 textureColor;
    if(hasTexture) {
        textureColor = texture(texture_diffuse1, TexCoords).rgb;
    } else {
        textureColor = vec3(0.8, 0.8, 0.8); // 默认灰色
    }
    
    // 结合光照和纹理
    vec3 result = (ambient + diffuse + specular) * textureColor;
    FragColor = vec4(result, 1.0);
}