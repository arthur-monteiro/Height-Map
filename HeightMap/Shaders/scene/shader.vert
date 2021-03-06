#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObjectMVP
{
    mat4 projection;
    mat4 model;
	mat4 view;
} uboMVP;

layout(location = 0) in vec3 inPosition;

out gl_PerVertex
{
    vec4 gl_Position;
};

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

void main() 
{
	vec4 viewPos = uboMVP.view * uboMVP.model * vec4(inPosition, 1.0);
    gl_Position = uboMVP.projection * viewPos;
} 
