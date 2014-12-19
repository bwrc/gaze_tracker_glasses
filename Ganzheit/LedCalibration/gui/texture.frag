//varying vec2 texture_coordinate; uniform sampler2D my_color_texture;
//void main()
//{
//        // Sampling The Texture And Passing It To The Frame Buffer
//        gl_FragColor = texture2D(my_color_texture, texture_coordinate);
//}

//uniform sampler2D texture;
//varying mediump vec4 texc;
//void main( void )
//{
//    gl_FragColor = texture2D( texture, texc.st );
//}

uniform sampler2D texture;
void main (void)
{
        gl_FragColor = texture2D( texture, gl_TexCoord[0].st);
}
