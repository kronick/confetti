uniform sampler2D tex0;
uniform vec3 thresholdMin;
uniform vec3 thresholdMax;
uniform vec2 tcOffset[9];

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


void main() {
    //vec3 hls = RGBtoHLS(texture2D(tex0, gl_TexCoord[0].st).rgb);
    //hls = (all(lessThan(hls,thresholdMax)) && all(greaterThan(hls, thresholdMin))) ? hls : vec3(0,0,0);
    
    vec3 sample[9];
    vec3 maxValue = vec3(0.0);
    
    for (int i = 0; i < 9; i++)
    {
        // Sample a grid around and including our texel
        sample[i] = RGBtoHLS(texture2D(tex0, gl_TexCoord[0].st + tcOffset[i]).rgb);
        sample[i] = (all(lessThan(sample[i],thresholdMax)) && all(greaterThan(sample[i], thresholdMin))) ? sample[i] : vec3(0,0,0);
        
        // Keep the maximum value
        maxValue = max(sample[i], maxValue);
    }
    
//    gl_FragColor.rgb = RGBtoHLS(texture2D(tex0, gl_TexCoord[0].st).rgb);
    
    gl_FragColor.rgb = maxValue;
    gl_FragColor.a = 1.0;
    
}