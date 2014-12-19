/*
 * http://oivdoc86.vsg3d.com/content/193-writing-pixel-lighting-shader
 * Original code:
 *
 * varying vec4 eposition;
 * varying vec3 normal;
 * varying vec3 diffuseColor;
 * varying vec3 specularColor;
 * varying vec3 emissiveColor;
 * varying vec3 ambientColor;
 * varying float shininess;
 * 
 * void main()
 * {
 *     // Position in clip space
 *     gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
 * 
 *     // Position in eye space
 *     eposition = gl_ModelViewMatrix * gl_Vertex;
 * 
 *     // Normal in eye space
 *     normal = mat3(gl_ModelViewMatrix) * gl_Normal;
 * 
 *     // Retrieves diffuse, specular emissive, and ambient color from the OpenGL state.
 *     diffuseColor = vec3(gl_FrontMaterial.diffuse);
 *     specularColor = vec3(gl_FrontMaterial.specular);
 *     emissiveColor = vec3(gl_FrontMaterial.emission);
 *     ambientColor = vec3(gl_FrontMaterial.ambient);
 *     shininess = gl_FrontMaterial.shininess;
 * }
 *
 */

/*!!GLSL */



const int N_LIGHTS = 5;


varying vec4 eposition;
varying vec3 normal;
varying vec4 diffuseColor[N_LIGHTS];
varying vec4 specularColor;
varying vec4 emissiveColor;
varying vec4 ambientColor[N_LIGHTS];
varying vec4 ambientGlobal;


void main() {


    // Position in eye space
    eposition = gl_ModelViewMatrix * gl_Vertex;

    // Normal in eye space
    normal = normalize(gl_NormalMatrix * gl_Normal);

    // Retrieves diffuse, specular emissive, and ambient color from the OpenGL state.

    for(int i = 0; i < N_LIGHTS; ++i) {
        diffuseColor[i] = gl_FrontMaterial.diffuse * gl_LightSource[i].diffuse;
        ambientColor[i] = gl_FrontMaterial.ambient * gl_LightSource[i].ambient;
    }

    specularColor = gl_FrontMaterial.specular;
    emissiveColor = gl_FrontMaterial.emission;
    ambientGlobal = gl_LightModel.ambient * gl_FrontMaterial.ambient;


    // Position in clip space
    gl_Position = ftransform();

}

