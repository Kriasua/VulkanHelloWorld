#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in float inTime;

void main()
{	
    
    float g = (cos(inTime * 1.0) + 1.0) / 2.0;
   
    vec3 finalColor = vec3(fragColor.r, g, fragColor.b);
	outColor = vec4(finalColor, 1.0);

}
