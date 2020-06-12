#version 430 core

layout (std430, binding = 1) buffer b{
	float heights[];
};
out vec3 norm;
out float dist;
out float col;

uniform vec4 startFinish;
uniform uvec2 samples;
uniform uvec2 meshSize;

uniform mat4 proj;
uniform mat4 view;
uniform vec3 pos;

const uvec2 squareCoords[6] = uvec2[](
	uvec2(0u, 0u),
	uvec2(1u, 0u),
	uvec2(1u, 1u),
	uvec2(1u, 1u),
	uvec2(0u, 1u),
	uvec2(0u, 0u)
);

const float vStretch = 1.0;

float perlinHeight(uvec2 coords) {	
	return heights[samples.x * coords.y + coords.x] * vStretch;
}

float perlinHeight2(uvec2 coords) {

	coords.y *= 4u;
	float height = perlinHeight(coords);
	coords.y += 1u;
	height += perlinHeight(coords) / 2.0;
	coords.y += 1u;
	return height + perlinHeight(coords) / 4.0;

}

float getColor(uvec2 coords) {
	return perlinHeight(uvec2(coords.x, coords.y * 4u + 3u)) / 1.4142 + 1.0;
}

vec2 getWorldCoords(vec2 v) {
	return startFinish.xy + (v / vec2(meshSize - 1u)) * (startFinish.zw - startFinish.xy);
}

void main() {
	
	uint meshIdx = gl_VertexID / 6u;
	uint squareIdx = gl_VertexID % 6u;
	uvec2 meshBaseCoords = uvec2(
		meshIdx % (meshSize.x - 1u),
		meshIdx / (meshSize.x - 1u)
	);
	uvec2 meshCoords = meshBaseCoords + squareCoords[squareIdx];
	vec2 worldCoords = getWorldCoords(vec2(meshCoords));

	vec3 coords = vec3(worldCoords.x, perlinHeight2(meshCoords), worldCoords.y);
	gl_Position = proj * view * vec4(coords, 1.0);
	dist = length(coords - pos) / 12.5;
	col = getColor(meshCoords);

	if (meshCoords.x == meshSize.x - 1u || meshCoords.x == 0u || meshCoords.y == meshSize.y - 1u || meshCoords.y == 0u) {
		norm = vec3(0.0, 1.0, 0.0);
		return;
	}

	vec2 v1 = getWorldCoords(vec2(meshCoords + uvec2(-1u, -1u)));
	vec2 v2 = getWorldCoords(vec2(meshCoords + uvec2(1u, -1u)));
	vec2 v3 = getWorldCoords(vec2(meshCoords + uvec2(1u, 1u)));
	vec3 vv1 = vec3(
		v1.x,
		perlinHeight2(meshCoords + uvec2(-1u, -1u)),
		v1.y
	);
	vec3 vv2 = vec3(
		v2.x,
		perlinHeight2(meshCoords + uvec2(1u, -1u)),
		v2.y
	);
	vec3 vv3 = vec3(
		v3.x,
		perlinHeight2(meshCoords + uvec2(1u, 1u)),
		v3.y
	);
	norm = normalize(cross(vv1 - vv2, vv3 - vv2));

}