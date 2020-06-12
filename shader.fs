#version 430 core

in vec3 norm;
in float dist;
in float col;
out vec4 fragColor;

const vec3 lightDir = vec3(1.0f, -1.0f, -1.0f);
const vec3 bgColor = vec3(0.7, 0.4, 0.3);
const vec3 green = vec3(0.1, 0.8, 0.24);
const vec3 gray = vec3(0.4, 0.45, 0.5);

void main() {

	float brightness = max(0.0, dot(normalize(-lightDir), norm));
	brightness = 0.3 + brightness * 0.7;
	fragColor.xyz = gray * col + green * (1.0 - col);
	fragColor.xyz *= brightness;
	if (dist > 0.8) {
		float f = (1.0 - dist) / 0.2;
		fragColor.xyz = fragColor.xyz * f + bgColor * (1.0 - f);
	}
	if (dist > 1.0) {
		fragColor.xyz = bgColor;
	}
	fragColor.w = 1.0;
	
}