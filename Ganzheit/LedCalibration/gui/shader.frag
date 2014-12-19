varying vec3 vNormal;
varying vec3 vVertex;
uniform vec3 CameraPosition;

#define shininess 99.0
void main (void)
{
         
// Material Color:
vec4 color0 = vec4(0.3, 0.3, 0.3, 1.0);
         
// Silhouette Color:
vec4 color1 = vec4(0.0, 0.0, 0.0, 1.0);
         
// Specular Color:
vec4 color2 = vec4(0.8, 0.0, 0.0, 1.0);
vec4 white = vec4(1.0, 1.0, 1.0, 1.0);
            
// Lighting
//ilmeisesti tän pitää olla esineen paikan negaatio, eli
//glTranslate(0,0,-5) => eyePos = (0,0,5)
vec3 eyePos = vec3(0.0,0.0,5.0);
//vec3 eyePos = CameraPosition;
//eyePos[2] = -1*eyePos[2];

vec3 lightPos1 = gl_LightSource[0].position.xyz; //vec3(-2.0,1.0,1.0); //
//vec3 lightPos1 = vec3(0.3,0.2,0);
vec3 lightPos2 = gl_LightSource[1].position.xyz; //vec3(2.0,-1.0,1.0); //

vec3 Normal = normalize(gl_NormalMatrix * vNormal);
vec3 EyeVert = normalize(eyePos - vVertex);

//vec3 LightVert = normalize(lightPos - vVertex);
vec3 LightVert1 = normalize(lightPos1 - vVertex);
vec3 LightVert2 = normalize(lightPos2 - vVertex);

//vec3 EyeLight = normalize(LightVert+EyeVert);
vec3 EyeLight1 = normalize(LightVert1+EyeVert);
vec3 EyeLight2 = normalize(LightVert2+EyeVert);

// Simple Silhouette
float sil = max(dot(Normal,EyeVert), 0.0);
if ((sil < 0.3)) gl_FragColor = color0*0.8;
else 
  {
   gl_FragColor = color0;
   // Specular part
   float spec1 = pow(max(dot(Normal,EyeLight1),0.0), shininess);
   float spec2 = pow(max(dot(Normal,EyeLight2),0.0), shininess);
   if ((spec1 < 0.2) && (spec2 < 0.2)) gl_FragColor *= 0.8;
   else gl_FragColor = white;//color2;
   // Diffuse part
//   float diffuse1 = max(dot(Normal,LightVert1),0.0);
//   float diffuse2 = max(dot(Normal,LightVert2),0.0);
//   if ((diffuse1 < 0.3)&&(diffuse2 < 0.3)) gl_FragColor *=0.8;
   }


/* NOT USED, pitää pistää samaan silmukkaan, muuten piirtää päälle
// Simple Silhouette
float sil = max(dot(Normal,EyeVert), 0.0);
if (sil < 0.3) gl_FragColor = color1;
else 
  {
   gl_FragColor = color0;
   // Specular part
   float spec = pow(max(dot(Normal,EyeLight),0.0), shininess);
   if (spec < 0.2) gl_FragColor *= 0.8;
   else gl_FragColor = white;//color2;
   // Diffuse part
   float diffuse = max(dot(Normal,LightVert),0.0);
   if (diffuse < 0.5) gl_FragColor *=0.8;
   }
*/

}
