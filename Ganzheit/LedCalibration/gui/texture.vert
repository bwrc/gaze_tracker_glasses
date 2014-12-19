//varying vec2 texture_coordinate;
//void main()
//{
//        // Transforming The Vertex
//        gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
//        // Passing The Texture Coordinate Of Texture Unit 0 To The Fragment Shader
//        texture_coordinate = vec2(gl_MultiTexCoord0);
//}

//attribute highp vec4 vertex;
//attribute mediump vec4 texCoord;
//varying mediump vec4 texc;
//uniform mediump mat4 matrix;
//void main( void )
//{
//    gl_Position = matrix * vertex;
//    texc = texCoord;
//}

void main()
{
        gl_TexCoord[0] = gl_MultiTexCoord0;
        gl_Position = ftransform();
}
