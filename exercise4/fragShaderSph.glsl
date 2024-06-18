#version 430

in vec2 tc;
in vec3 varyingNormal;
in vec3 varyingLightDir;
in vec3 varyingVertPos;

in vec3 varyingTangent;
in vec3 originalVertex;
in vec3 varyingHalfVector;

out vec4 color;

//layout (binding=0) uniform sampler2D normMap;

struct PositionalLight
{	vec4 ambient;  
	vec4 diffuse;  
	vec4 specular;  
	vec3 position;
};

struct Material
{	vec4 ambient;  
	vec4 diffuse;  
	vec4 specular;  
	float shininess;
};

uniform vec4 globalAmbient;
uniform PositionalLight light;
uniform Material material;
uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
uniform mat4 view_matrix;
uniform mat4 norm_matrix;

layout (binding=0) uniform sampler2D s;


vec3 calcNewNormal()
{
	vec3 normal = normalize(varyingNormal);
	vec3 tangent = normalize(varyingTangent);
	tangent = normalize(tangent - dot(tangent, normal) * normal);
	vec3 bitangent = cross(tangent, normal);
	mat3 tbn = mat3(tangent, bitangent, normal);
	vec3 retrievedNormal = texture(s,tc).xyz;
	retrievedNormal = retrievedNormal * 2.0 - 1.0;
	vec3 newNormal = tbn * retrievedNormal;
	newNormal = normalize(newNormal);
	return newNormal;
}

void main(void)
{	
	// normalize the light, normal, and view vectors:
	//vec3 L = normalize(varyingLightDir);
	//vec3 N = normalize(varyingNormal);
	//vec3 V = normalize(-varyingVertPos);
	
	// compute light reflection vector, with respect N:
	//vec3 R = normalize(reflect(-L, N));
	
	// get the angle between the light and surface normal:
	//float cosTheta = dot(L,N);
	
	// angle between the view vector and reflected light:
	//float cosPhi = dot(V,R);

	// compute ADS contributions (per pixel):
	//vec3 ambient = ((globalAmbient * material.ambient) + (light.ambient * material.ambient)).xyz;
	//vec3 diffuse = light.diffuse.xyz * material.diffuse.xyz * max(cosTheta,0.0);
	//vec3 specular = light.specular.xyz * material.specular.xyz * pow(max(cosPhi,0.0), material.shininess);
	
	//vec4 lightColor = vec4((ambient + diffuse + specular), 1.0);

	//color = lightColor * texture(s,tc);

	// normalize the light, normal, and view vectors:
	vec3 L = normalize(varyingLightDir);
	vec3 V = normalize(-varyingVertPos);

	vec3 N = calcNewNormal();

	// get the angle between the light and surface normal:
	float cosTheta = dot(L,N);
	
	// compute light reflection vector, with respect N:
	//vec3 R = normalize(reflect(-L, N));
	
	vec3 H = normalize(varyingHalfVector);
	
	// angle between the view vector and reflected light:
	float cosPhi = dot(H,N);

	// compute ADS contributions (per pixel):
	color = globalAmbient * material.ambient
	+ light.ambient * material.ambient
	+ light.diffuse * material.diffuse * max(cosTheta,0.0)
	+ light.specular  * material.specular
		* pow(max(cosPhi,0.0), material.shininess*3.0);
}
