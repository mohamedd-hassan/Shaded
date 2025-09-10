precision mediump float;
varying vec2 vTexCoord;
uniform sampler2D uTexture;
uniform float uRadius;
uniform vec2 uTextureSize;
uniform vec2 uInputSize;
uniform vec2 uOutputSize;

void main() {
    // For the vertical pass, we're sampling from the intermediate texture
    // which is already in the expanded coordinate space
    vec2 texelSize = 1.0 / uTextureSize; // Use the intermediate texture size
    vec4 color = vec4(0.0);

    if (uRadius <= 0.5) {
        gl_FragColor = texture2D(uTexture, vTexCoord);
        return;
    }

    float sigma = uRadius / 2.0;
    float twoSigmaSq = 2.0 * sigma * sigma;
    float totalWeight = 0.0;
    int iRadius = int(ceil(uRadius));

    // Sample along vertical axis only
    for (int y = -iRadius; y <= iRadius; ++y) {
        float distance = abs(float(y));
        if (distance <= uRadius) {
            vec2 offset = vec2(0.0, float(y) * texelSize.y);
            vec2 sampleCoord = vTexCoord + offset;

            vec4 sample;
            if (sampleCoord.x >= 0.0 && sampleCoord.x <= 1.0 &&
                sampleCoord.y >= 0.0 && sampleCoord.y <= 1.0) {
                // Sample from the intermediate texture
                sample = texture2D(uTexture, sampleCoord);
            } else {
                // Sample transparent black for pixels outside bounds
                sample = vec4(0.0, 0.0, 0.0, 0.0);
            }

            float weight = exp(-(distance * distance) / twoSigmaSq);
            color += sample * weight;
            totalWeight += weight;
        }
    }

    gl_FragColor = color / totalWeight;
}