uniform sampler2D tex0;
uniform vec3 thresholdMin;
uniform vec3 thresholdMax;

float Epsilon = 1e-10;
// Color space conversion functions from: http://www.chilliant.com/rgb2hsv.html



vec3 RGBtoHCV(vec3 RGB)
{
    // Based on work by Sam Hocevar and Emil Persson
    vec4 P = (RGB.g < RGB.b) ? vec4(RGB.bg, -1.0, 2.0/3.0) : vec4(RGB.gb, 0.0, -1.0/3.0);
    vec4 Q = (RGB.r < P.x) ? vec4(P.xyw, RGB.r) : vec4(RGB.r, P.yzx);
    float C = Q.x - min(Q.w, Q.y);
    float H = abs((Q.w - Q.y) / (6.0 * C + Epsilon) + Q.z);
    return vec3(H, C, Q.x);
}

vec3 RGBtoHLS(vec3 RGB)
{
    vec3 HCV = RGBtoHCV(RGB);
    float L = HCV.z - HCV.y * 0.5;
    float S = HCV.y / (1.0 - abs(L * 2.0 - 1.0) + Epsilon);
    return vec3(HCV.x, L, S);   // H *= 0.703125
    
}


//vec3 RGBtoHLS( vec3 _input ){
//    float h = 0.0;
//    float s = 0.0;
//    float l = 0.0;
//    float r = _input.r;
//    float g = _input.g;
//    float b = _input.b;
//    float cMin = min( r, min( g, b ) );
//    float cMax = max( r, max( g, b ) );
//    
//    l = ( cMax + cMin ) / 2.0;
//    if ( cMax > cMin ) {
//        float cDelta = cMax - cMin;
//        
//        s = l < .05 ? cDelta / ( cMax + cMin ) : cDelta / ( 2.0 - ( cMax + cMin ) );
//        
//        // hue
//        if ( r == cMax ) {
//            h = ( g - b ) / cDelta;
//        } else if ( g == cMax ) {
//            h = 2.0 + ( b - r ) / cDelta;
//        } else {
//            h = 4.0 + ( r - g ) / cDelta;
//        }
//        
//        if ( h < 0.0) {
//            h += 6.0;
//        }
//        h = h / 6.0;
//    }
//    return vec3( h * 0.7058823529, l, s );
//}

void main() {
    vec3 hls = RGBtoHLS(texture2D(tex0, gl_TexCoord[0].st).rgb);
    hls = (all(lessThan(hls,thresholdMax)) && all(greaterThan(hls, thresholdMin))) ? hls : vec3(0,0,0);
    
    vec4 sample[25];
    vec4 maxValue = vec4(0.0);
    
    for (int i = 0; i < 25; i++)
    {
        // Sample a grid around and including our texel
        sample[i] = texture(quadTexture, vTex + tcOffset[i]);
        
        // Keep the maximum value
        maxValue = max(sample[i], maxValue);
    }
    
    vFragColour = maxValue;
    
//    gl_FragColor.rgb = RGBtoHLS(texture2D(tex0, gl_TexCoord[0].st).rgb);
    
    gl_FragColor.rgb = hls;
    gl_FragColor.a = 1.0;
    
}