#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in float inTime;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inPos;


layout(binding = 0) uniform UniformBufferObject
{
	mat4 view;
	mat4 proj;
	vec4 lightDir;
    vec4 lightColor;
    mat4 lightMat;
	float intime;
} ubo;

layout(binding = 1)uniform sampler2D texSampler;


void main()
{	
   
	//vec4 baseColor = texture(texSampler,inTexCoord);
	

    //vec3 ambient = 0.3 * baseColor.rgb;


    //vec3 norm = normalize(inNormal);

    //vec3 lightDir = normalize(ubo.lightDir.xyz); 
    

    //float diff = max(dot(norm, lightDir), 0.0);
    //vec3 diffuse = diff * baseColor.rgb; 
    //diffuse *= ubo.lightColor.xyz;

    // 4. 混合出最终带有光影的颜色
    //vec3 result = ambient + diffuse;
    //outColor = vec4(result, baseColor.a);

// 1. 采样基础贴图
    vec4 baseColor = texture(texSampler, inTexCoord);
    
    // 2. 环境光（NPR 通常给一个比较亮且均匀的底色）
    vec3 ambient = 0.3 * baseColor.rgb;

    // 3. 计算光照强度 (Lambertain)
    vec3 norm = normalize(inNormal);
    vec3 lightDir = normalize(ubo.lightDir.xyz); 
    float NdotL = dot(norm, lightDir);

    float s1 = step(0.0, NdotL); // 大于 0 变为 1.0
    float s2 = step(0.5, NdotL); // 大于 0.5 变为 1.0
    float shadowMask = smoothstep(0.49, 0.51, NdotL);
    float diff = mix(0.3, 1.0, shadowMask); // 在 0.3 和 1.0 之间平滑过渡

    vec3 diffuse = diff * baseColor.rgb * ubo.lightColor.xyz;

    vec3 result = diffuse; 
    
    result += ambient;

    outColor = vec4(result, baseColor.a);
    

}
