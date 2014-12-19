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
 *     const vec3 lightColor = vec3(1, 1, 1);
 *     const vec3 globalAmbient = vec3(0.2, 0.2, 0.2);
 * 
 *     // Position in eye space
 *     vec3 P = vec3(eposition);
 * 
 *     // Normalize normal in eye space
 *     vec3 N = normalize(normal);
 * 
 *     // Compute the emissive term
 *     vec3 emissive = emissiveColor;
 * 
 *     // Compute the ambient term
 *     vec3 ambient = ambientColor * globalAmbient;
 * 
 *     // Compute the diffuse term
 *     // Normalized vector toward the light source
 *     vec3 L = normalize(vec3(gl_LightSource[0].position) - P);
 *     float diffuseLight = max(dot(N, L), 0);
 *     vec3 diffuse = diffuseColor * lightColor * diffuseLight;
 * 
 *     // Compute the specular term
 *     vec3 V = normalize(-P);      // Normalized vector toward the viewpoint
 *     vec3 H = normalize(L + V);   // Normalized vector that is halfway between V and L
 *     float specularLight = pow(max(dot(N, H),0), shininess);
 *     if(diffuseLight <= 0)
 *         specularLight = 0;
 *     vec3 specular = specularColor * lightColor * specularLight;
 * 
 *     // Define the final vertex color
 *     gl_FragColor.xyz = emissive + ambient + diffuse + specular;
 *     gl_FragColor.w = 1.0;
 * }
 *
 */



/*
 * This modified code makes use of alpha, the original does not. Also
 * shininnes does not need to be a varying variable as it is accessible
 * through gl_FrontMaterial.shininess
 */

const int N_LIGHTS = 5;


varying vec4 eposition;
varying vec3 normal;
varying vec4 diffuseColor[N_LIGHTS];
varying vec4 specularColor;
varying vec4 emissiveColor;
varying vec4 ambientColor[N_LIGHTS];
varying vec4 ambientGlobal;


void main() {

    // // Position in eye space
    vec3 P = vec3(eposition);

    // Normalize normal in eye space, not enough that it's normalised in the vertex
    // program due to interpolation
    vec3 N = normalize(normal);

    vec4 cumulColor = vec4(0.0);

    for(int i = 0; i < N_LIGHTS; ++i) {

        // Compute the ambient term
        vec4 ambient = ambientColor[i] * ambientGlobal;

        // Compute the diffuse term
        // Normalized vector toward the light source
        vec3 L = normalize(vec3(gl_LightSource[i].position) - P);
        float diffuseLight = max(dot(N, L), 0.0);

        vec4 diffuse = diffuseColor[i] * diffuseLight;

        // Compute the specular term
        vec3 V = normalize(-P);      // Normalized vector toward the viewpoint
        vec3 H = normalize(L + V);   // Normalized vector that is halfway between V and L
        float specularLight = pow(max(dot(N, H), 0.0), gl_FrontMaterial.shininess);
        if(diffuseLight <= 0.0) {
            specularLight = 0.0;
        }

        vec4 specular = specularColor * specularLight;

        //        cumulColor += ambient + diffuse + specular + emissiveColor;
        cumulColor = cumulColor + ambient + diffuse + specular;

    }

    cumulColor = cumulColor + emissiveColor;

    // Define the final vertex color
    gl_FragColor = cumulColor;

}

