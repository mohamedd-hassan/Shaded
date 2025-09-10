precision mediump float;
varying vec2 vTexCoord;
uniform sampler2D uTexture;
uniform float uRadius;
uniform vec2 uTextureSize;
uniform vec2 uInputSize;
uniform vec2 uOutputSize;

void main() {
    // Map from expanded output coordinates to original texture coordinates
    // The original image should be centered in the expanded output
    vec2 expansion = vec2(uRadius) / uOutputSize;
    vec2 originalCoord = (vTexCoord - expansion) / (1.0 - 2.0 * expansion);

    vec2 texelSize = 1.0 / uInputSize;
    vec4 color = vec4(0.0);

    if (uRadius <= 0.5) {
        if (originalCoord.x >= 0.0 && originalCoord.x <= 1.0 &&
        originalCoord.y >= 0.0 && originalCoord.y <= 1.0) {
            gl_FragColor = texture2D(uTexture, originalCoord);
        } else {
            gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
        }
        return;
    }

    float sigma = uRadius / 2.0;
    float twoSigmaSq = 2.0 * sigma * sigma;
    float totalWeight = 0.0;
    int iRadius = int(ceil(uRadius));

    // Sample along horizontal axis only
    for (int x = -iRadius; x <= iRadius; ++x) {
        float distance = abs(float(x));
        if(distance <= uRadius){
            vec2 offset = vec2(float(x) * texelSize.x, 0.0);
            vec2 sampleCoord = originalCoord + offset;

            vec4 sample;
        if (sampleCoord.x >= 0.0 && sampleCoord.x <= 1.0 &&
            sampleCoord.y >= 0.0 && sampleCoord.y <= 1.0) {
            // Sample from the original texture
            sample = texture2D(uTexture, sampleCoord);
            } else {
                // Sample transparent black for pixels outside original bounds
                sample = vec4(0.0, 0.0, 0.0, 0.0);
            }

            float weight = exp(-(distance * distance) / twoSigmaSq);
            color += sample * weight;
            totalWeight += weight;
        }
    }

    gl_FragColor = color / totalWeight;
}