#version 330

#define PI  3.14159265

uniform mat4 invViewMX;    //!< inverse view matrix

uniform vec3 matcolor;
uniform int  haveNormals;
uniform int  haveColors;
uniform int  haveCheckerTex;

uniform vec3 ambient;      //!< ambient color
uniform vec3 diffuse;      //!< diffuse color
uniform vec3 specular;     //!< specular color

uniform float k_amb;       //!< ambient factor
uniform float k_diff;      //!< diffuse factor
uniform float k_spec;      //!< specular factor
uniform float k_exp;       //!< specular exponent

uniform vec3 pickIdCol;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 fragColorPic;

in vec3 texCoord;
in vec3 normal;
in vec3 pos;

// --------------------------------------------------
//   Blinn-Phong shading model
// --------------------------------------------------
vec3 blinnPhong(vec3 n, vec3 l, vec3 v) {
    vec3 col = vec3(0.0);

    // ambient
    col = k_amb * ambient;

    // diffuse
    float NdotL = max(0.0, dot(n, l));
    NdotL = abs(dot(n,l));
    col += k_diff * diffuse * NdotL;

    // specular
    vec3 h = normalize(l+v);
    float NdotH = max(0.0, dot(n, h));
    NdotH = abs(dot(n,h));
   // col += k_spec * specular * ((k_exp+2.0)/(2.0*PI)) * pow(NdotH, k_exp);
    return col;
}

// --------------------------------------------------
//
// --------------------------------------------------
void main() {    
    vec3 camera  = invViewMX[3].xyz / invViewMX[3].w;
    vec3 ray_dir = normalize(camera-pos);

    vec3 color = texCoord;
    color = normal*0.5+vec3(0.5);
    color = vec3(1.0);

    int freq = 4;
    float checkerVal = 0.8 + 0.2*sign(sin(texCoord.x*2*PI*freq)*sin(texCoord.y*2*PI*freq));
    color = haveCheckerTex*vec3(checkerVal) + (1-haveCheckerTex)*vec3(1);
    color = haveColors*matcolor + (1-haveColors)*color;
    color *= blinnPhong(normal, normalize(camera-pos), ray_dir);

    fragColor = vec4(color,1);
    fragColorPic = vec4(pickIdCol,1);
}
