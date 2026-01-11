#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in float inTime;
layout(location = 2) in vec2 inTexCoord;

layout(binding = 1)uniform sampler2D texSampler;

void main()
{	
   
	outColor = texture(texSampler,inTexCoord);

}
