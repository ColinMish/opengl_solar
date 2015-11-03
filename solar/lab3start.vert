// Minimal vertex shader

#version 400

// These are the vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec3 normal;

// Uniform variables are passed in from the application
uniform mat4 model, view, projection;
uniform uint colourmode;

// Output the vertex colour - to be rasterized into pixel fragments
out vec4 fcolour;

void main()
{
	vec3 light_dir = vec3(0, 0, 4.0);			// Define a light direction

	vec4 diffuse_colour;
	vec4 position_h = vec4(position, 1.0);
	
	if (colourmode == 1)
		diffuse_colour = colour;
	else
		diffuse_colour = vec4(0.0, 1.0, 0, 1.0);

	vec4 ambient = diffuse_colour * 0.2;			// Define ambient as a darker cversin of the vertex colour

	// calculate diffuse lighting
	mat4 mv_matrix = view * model;					// Be careful with the order of matrix multiplication!
	mat3 n_matrix = transpose(inverse(mat3(mv_matrix)));  // It's more efficient to calcuate this in your application
	vec3 N = normalize(n_matrix * normal);			// Be carefull with the order of multiplication!
	vec3 L = normalize(light_dir);					// Ensure light_dir is unit length
	vec4 diffuse = max(dot(L, N), 0) * diffuse_colour;

	// Calculate specular lighting
	vec4 specular_colour = vec4(1.0, 1.0, 0.6, 0);	// Bright yellow light
	float shininess = 8;							// smaller values give sharper specular repsonses, larger more spread out
	vec4 P = mv_matrix * position_h;				// Calculate vertex position in eye space
	vec3 V = normalize(-P.xyz);
	vec3 R= reflect(-L, N);							// Calculate the reflected beam, N defines the plane (see diagram on labsheet)
	vec4 specular = pow(max(dot(R, V), 0), shininess) * specular_colour;	// Calculate specular component

	// Define the vertex colour by summing the sperate lighting components
	fcolour = ambient + diffuse + specular;

	// Define the vertex position in homogeneous coordindates
	gl_Position = (projection * view * model) * position_h;
}

