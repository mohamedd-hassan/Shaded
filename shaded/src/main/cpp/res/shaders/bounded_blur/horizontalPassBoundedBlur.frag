precision mediump float;
varying vec2 vTexCoord;
uniform sampler2D uTexture;
uniform float uRadius;
uniform vec2 uTextureSize;

void main() {
    vec2 texelSize = 1.0 / uTextureSize;
    vec4 color = vec4(0.0);

    if (uRadius <= 0.5) {
        gl_FragColor = texture2D(uTexture, vTexCoord);
        return;
    }

    float sigma = uRadius / 2.0;
    float twoSigmaSq = 2.0 * sigma * sigma;
    float totalWeight = 0.0;
    int iRadius = int(ceil(uRadius));

    // Sample along horizontal axis only
    for (int x = -iRadius; x <= iRadius; ++x) {
        float distance = abs(float(x));
        if (distance <= uRadius) {
            vec2 sampleCoord = vTexCoord + vec2(float(x) * texelSize.x, 0.0);
            sampleCoord = clamp(sampleCoord, vec2(0.0), vec2(1.0));

            float weight = exp(-(distance * distance) / twoSigmaSq);
            color += texture2D(uTexture, sampleCoord) * weight;
            totalWeight += weight;
        }
    }

    gl_FragColor = color / totalWeight;
}