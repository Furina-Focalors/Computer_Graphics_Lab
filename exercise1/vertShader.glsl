#version 430

in vec2 vPosition;
in vec4 vColor;
out vec4 fColor;

void main()
{
	fColor = vColor;
	gl_Position = vec4(vPosition, 0.0, 1.0);
}