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

float calculateShadow(vec3 worldPos) {
    // 1. 将世界坐标转换到光源裁剪空间 (Light Clip Space)
    vec4 shadowCoord = ubo.lightMat * vec4(worldPos, 1.0);
    
    // 2. 执行透视除法 (虽然正交投影下 w 通常是 1，但这是标准写法)
    vec3 projCoords = shadowCoord.xyz / shadowCoord.w;
    
    // 3. 变换坐标范围：从 [-1, 1] 映射到 [0, 1] 才能采样贴图
    // 注意：Vulkan 的 Y 轴方向已经在你的矩阵翻转里处理过了
    projCoords.xy = projCoords.xy * 0.5 + 0.5;
    
    // 如果坐标在 [0, 1] 范围外，说明在光源视锥体外，通常不设阴影
    if (projCoords.z > 1.0 || projCoords.z < 0.0) return 1.0;

    // 4. 采样阴影贴图中的深度值（记录的是离光源最近的距离）
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    
    // 5. 当前像素到光源的实际深度
    float currentDepth = projCoords.z;

    // 6. 🌟 阴影偏移 (Bias)：防止阴影粉刺 (Shadow Acne)
    // 根据表面法线和光线角度调整偏移量，让阴影更干净
    float bias = max(0.005 * (1.0 - dot(normalize(inNormal), normalize(ubo.lightDir.xyz))), 0.0005);
    
    // 7. 比较深度：如果当前点比最近点还要深，说明被挡住了
    // 因为你之前调的是 Near=100, Far=0.1 这种“反向深度”，
    // 如果你在 RenderDoc 看到房子是黑（小），地是灰（大），那依然满足 current > closest 就是阴影。
    float shadow = currentDepth > closestDepth + bias ? 0.0 : 1.0;

    return shadow;
}


void main()
{	

// 1. 采样基础贴图
    vec4 baseColor = texture(texSampler, inTexCoord);
    
    // 2. 环境光（NPR 通常给一个比较亮且均匀的底色）
    vec3 ambient = 0.3 * baseColor.rgb;

    // 3. 计算光照强度 (Lambertain)
    vec3 norm = normalize(inNormal);
    vec3 lightDir = normalize(ubo.lightDir.xyz); 
    float NdotL = dot(norm, lightDir);

    float shadowMask = smoothstep(0.49, 0.51, NdotL);
    float diffIntensity = mix(0.3, 1.0, shadowMask); // 漫反射强度

    // 4. 🌟 引入阴影贴图计算
    float shadow = calculateShadow(inPos);

    vec3 diffuse = diffIntensity * baseColor.rgb * ubo.lightColor.xyz * shadow;

    vec3 result = diffuse; 
    
    result += ambient;

    outColor = vec4(result, baseColor.a);

}
