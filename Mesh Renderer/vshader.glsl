#version 150

in vec4 vPosition;
in vec3 vNormal;

out vec4 color;

uniform mat4 projection;
uniform vec4 ambientProduct;
uniform vec4 diffuseProduct;
uniform vec4 specularProduct;
uniform vec4 lightPosition;
uniform vec4 location;

uniform float shininess;
uniform float theta;
uniform float scale;

void main() {

	// Transform the vertex normal into eye coordinates
	vec3 n = normalize(vec4(vNormal, 0.0)).xyz;

	// Normalized vector in the direction of light source
	vec3 l = normalize(lightPosition.xyz - vPosition.xyz);

	// Normalized vector in the direction of the viewer (viewer is at the origin)
	vec3 v = normalize(-vPosition.xyz);

	// Normalized halfway vector
	vec3 h = normalize(l + v);

	// Computation of the diffuse term
	float diffuseCoefficient = max(dot(l, n), 0.0);
	vec4 diffuse = diffuseCoefficient * diffuseProduct;

	// Computation of the specular term
	float specularCoefficient = pow(max(dot(n, h), 0.0), shininess);
	vec4 specular = specularCoefficient * specularProduct;

	// If the light source is behind the surface, there cannot be a specular term
	if (dot(l, n) < 0.0) {
		specular = vec4(0.0, 0.0, 0.0, 1.0);
	}

	// Output color is the sum of all three components
	color = ambientProduct + diffuse + specular;
	color.a = 1.0;

	// Compute sine and cosine of angle theta
	float angle = radians(-theta);
	float cosValue = cos(angle);
	float sinValue = sin(angle);

	// Matrix that defines rotation about y-axis
	mat4 rotation = mat4(cosValue, 0.0, -sinValue, 0.0,
							0.0, 1.0, 0.0, 0.0,
							sinValue, 0.0, cosValue, 0.0,
							0.0, 0.0, 0.0, 1.0);

	// Matrix that defines scale
	mat4 scale = mat4(scale, 0.0, 0.0, 0.0,
						0.0, scale, 0.0, 0.0,
						0.0, 0.0, scale, 0.0,
						0.0, 0.0, 0.0, 1.0);

	// Matrix that defines translation
	mat4 translation = mat4(1.0, 0.0, 0.0, 0.0,
							0.0, 1.0, 0.0, 0.0,
							0.0, 0.0, 1.0, 0.0,
							location.x, location.y, location.z, 1.0);

	// Computation of vertex position
	gl_Position = projection * translation * scale * rotation * vPosition;
}
