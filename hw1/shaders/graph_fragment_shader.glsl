#version 330 core

in float value;

layout (location = 0) out vec4 out_color;

void main()
{
	float threshold = 1;
	float t = max(-threshold, min(threshold, value));
	t = (t + threshold) / (2 * threshold);

	vec3 down = vec3(0.1, 0.15, 0.3);
	vec3 up = vec3(0.7, 0.75, 1);

	vec3 color = (1 - t) * down + t * up;

	out_color = vec4(color, 1);
}
