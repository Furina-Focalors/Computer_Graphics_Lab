#version 430

layout (location = 0) in vec3 position;

uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
uniform mat4 view_matrix;

layout (binding=0) uniform sampler2D s;

void main(void){
	gl_Position = proj_matrix * view_matrix * mv_matrix * vec4(position,1.0);
}
