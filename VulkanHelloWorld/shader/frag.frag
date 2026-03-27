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
layout(binding = 2) uniform sampler2D shadowMap;


float calculatePCFShadow(vec3 worldPos) {

    vec4 shadowCoord = ubo.lightMat * vec4(worldPos, 1.0);
    

    vec3 projCoords = shadowCoord.xyz / shadowCoord.w;
    
    // 从 [-1, 1] 映射到 [0, 1]
    projCoords.xy = projCoords.xy * 0.5 + 0.5;
    
    // 如果坐标在 [0, 1] 范围外，说明在光源视锥体外，通常不设阴影
    if (projCoords.z > 1.0 || projCoords.z < 0.0 || 
        projCoords.x > 1.0 || projCoords.x < 0.0 || 
        projCoords.y > 1.0 || projCoords.y < 0.0) {
        return 1.0; 
    }

    float currentDepth = projCoords.z;

    // 计算自适应偏移量，防止阴影失真(Shadow Acne)
    float bias = max(0.005 * (1.0 - dot(normalize(inNormal), normalize(ubo.lightDir.xyz))), 0.0005);
    
    // --- PCF 核心逻辑 ---
    float shadow = 0.0;
    // 获取阴影贴图的纹素大小 (1.0 / 贴图宽高)
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));

    // 3x3 采样循环
    for(int x = -2; x <= 2; ++x)
    {
        for(int y = -2; y <= 2; ++y)
        {
            // 采样周围的深度值
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            // 如果当前深度大于采样深度+偏移量，说明在阴影中(0.0)，否则被照亮(1.0)
            shadow += currentDepth > pcfDepth + bias ? 0.0 : 1.0;        
        }    
    }
    shadow /= 25.0;
    
    return shadow;
}


void main()
{	

    vec4 baseColor = texture(texSampler, inTexCoord);
    

    vec3 ambient = 0.3 * baseColor.rgb;


    vec3 norm = normalize(inNormal);
    vec3 lightDir = normalize(ubo.lightDir.xyz); 
    float NdotL = dot(norm, lightDir);

    float shadowMask = smoothstep(0.49, 0.51, NdotL);
    float diffIntensity = mix(0.3, 1.0, shadowMask); 


    float shadow = calculatePCFShadow(inPos);

    vec3 diffuse = diffIntensity * baseColor.rgb * ubo.lightColor.xyz * shadow;

    vec3 result = diffuse; 
    
    result += ambient;

    outColor = vec4(result, baseColor.a);

}
