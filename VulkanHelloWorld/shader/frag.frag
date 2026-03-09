#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in float inTime;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inPos;

layout(binding = 1)uniform sampler2D texSampler;

void main()
{	
   
	vec4 baseColor = texture(texSampler,inTexCoord);
	
	// 2. 环境光 (Ambient)：让背光的地方不要死黑一片
    vec3 ambient = 0.3 * baseColor.rgb;

	// 3. 漫反射 (Diffuse)：核心光照逻辑
    vec3 norm = normalize(inNormal);
    // 假设太阳在斜上方 (1.0, 2.0, 1.0)
    vec3 lightDir = normalize(vec3(1.0, 2.0, 1.0)); 
    
    // 算一下法线和太阳光的夹角，越垂直越亮
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * baseColor.rgb; // 光是白色的

    // 4. 混合出最终带有光影的颜色
    vec3 result = ambient + diffuse;
    outColor = vec4(result, baseColor.a);

	

}
