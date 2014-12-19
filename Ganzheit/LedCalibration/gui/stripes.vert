varying vec3 Normal;
 
void main(void)
{
        Normal = normalize(gl_NormalMatrix * gl_Normal);
        gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex + 100.0*sin(gl_Normal[0]);
}

