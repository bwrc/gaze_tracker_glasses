varying vec3 vNormal;
varying vec3 vVertex;
uniform vec3 CameraPosition;
            
void main(void)
{
	vVertex = gl_Vertex.xyz;
	vNormal = gl_Normal;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;// + sin(gl_Vertex.x)*sin(sin(gl_Vertex.x));

	vec3 CameraPosition = vec3(gl_ModelViewMatrixInverse * vec4(0,0,0,1.0));
	CameraPosition = normalize(CameraPosition - gl_Vertex.xyz);
}

