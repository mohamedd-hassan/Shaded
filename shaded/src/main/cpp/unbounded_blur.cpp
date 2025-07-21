#include "unbounded_blur.h"
#include <cmath>
#include <algorithm>

UnboundedBlurRenderer::UnboundedBlurRenderer(int maxWidth, int maxHeight)
        : eglHelper_(maxWidth, maxHeight),
          quadVBO_(0), shaderProgram_(0),
          attrPos_(-1), attrTexCoord_(-1),
          uniformTexture_(-1), uniformRadius_(-1), uniformTextureSize_(-1),
          uniformInputSize_(-1), uniformOutputSize_(-1),
          framebuffer_(0), fboTexture_(0),
          currentFBOWidth_(0), currentFBOHeight_(0),
          initialized_(false) {}

void UnboundedBlurRenderer::initialize() {
    if (initialized_) return;

    eglHelper_.makeCurrent();

    compileShaders();
    setupFullscreenQuad();
    setupFramebuffer();

    initialized_ = true;
}

GLuint UnboundedBlurRenderer::uploadBitmapAsTexture(unsigned char *pixels, int width, int height) {
    initialize();
    eglHelper_.makeCurrent();

    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return textureId;
}

int UnboundedBlurRenderer::calculateOutputSize(int inputSize, float radius) {
    // For unbounded blur, we need to expand the output by the blur radius on all sides
    // This ensures the blur effect extends beyond the original image boundaries
    int expansion = static_cast<int>(std::ceil(radius * 2.0f));
    return inputSize + expansion;
}

void UnboundedBlurRenderer::render(GLuint textureId, int inputWidth, int inputHeight,
                                   float radius, int &outputWidth, int &outputHeight) {
    initialize();
    eglHelper_.makeCurrent();

    // Calculate output dimensions
    outputWidth = calculateOutputSize(inputWidth, radius);
    outputHeight = calculateOutputSize(inputHeight, radius);

    // Resize framebuffer if needed
    if (outputWidth != currentFBOWidth_ || outputHeight != currentFBOHeight_) {
        resizeFramebuffer(outputWidth, outputHeight);
    }

    glUseProgram(shaderProgram_);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
    glViewport(0, 0, outputWidth, outputHeight);

    // Clear with transparent background
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set uniforms
    glUniform1f(uniformRadius_, radius);
    glUniform2f(uniformTextureSize_, static_cast<float>(inputWidth),
                static_cast<float>(inputHeight));
    glUniform2f(uniformInputSize_, static_cast<float>(inputWidth), static_cast<float>(inputHeight));
    glUniform2f(uniformOutputSize_, static_cast<float>(outputWidth),
                static_cast<float>(outputHeight));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glUniform1i(uniformTexture_, 0);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO_);

    glEnableVertexAttribArray(attrPos_);
    glVertexAttribPointer(attrPos_, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0);

    glEnableVertexAttribArray(attrTexCoord_);
    glVertexAttribPointer(attrTexCoord_, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void *) (2 * sizeof(float)));

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableVertexAttribArray(attrPos_);
    glDisableVertexAttribArray(attrTexCoord_);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void UnboundedBlurRenderer::readFBO(unsigned char *pixels, int width, int height) {
    eglHelper_.makeCurrent();
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void UnboundedBlurRenderer::setupFullscreenQuad() {
    const float quadVertices[] = {
            -1.0f, 1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
    };

    glGenBuffers(1, &quadVBO_);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void UnboundedBlurRenderer::setupFramebuffer() {
    glGenFramebuffers(1, &framebuffer_);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);

    glGenTextures(1, &fboTexture_);
    glBindTexture(GL_TEXTURE_2D, fboTexture_);

    // Start with a default size, will be resized as needed
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture_, 0);

    currentFBOWidth_ = 1024;
    currentFBOHeight_ = 1024;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void UnboundedBlurRenderer::resizeFramebuffer(int width, int height) {
    glBindTexture(GL_TEXTURE_2D, fboTexture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    currentFBOWidth_ = width;
    currentFBOHeight_ = height;
}

GLuint UnboundedBlurRenderer::compileShader(GLenum type, const char *source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    return shader;
}

void UnboundedBlurRenderer::compileShaders() {
    const char *vertexShaderSrc = R"(
        attribute vec2 aPosition;
        attribute vec2 aTexCoord;
        varying vec2 vTexCoord;
        void main() {
            vTexCoord = aTexCoord;
            gl_Position = vec4(aPosition, 0.0, 1.0);
        }
    )";

    const char *fragmentShaderSrc = R"(
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
    float totalWeight = 0.0;
    float pixelRadius = uRadius;

    if (pixelRadius <= 0.5) {
        if (originalCoord.x >= 0.0 && originalCoord.x <= 1.0 &&
            originalCoord.y >= 0.0 && originalCoord.y <= 1.0) {
            gl_FragColor = texture2D(uTexture, originalCoord);
        } else {
            gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
        }
        return;
    }

    float sigma = pixelRadius / 2.0;
    int iRadius = int(ceil(pixelRadius));

    for (int x = -iRadius; x <= iRadius; ++x) {
        for (int y = -iRadius; y <= iRadius; ++y) {
            float dist = sqrt(float(x * x + y * y));
            if (dist <= pixelRadius) {
                vec2 offset = vec2(float(x), float(y)) * texelSize;
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

                float weight = exp(-(dist * dist) / (2.0 * sigma * sigma));
                color += sample * weight;
                totalWeight += weight;
            }
        }
    }

    if (totalWeight > 0.0) {
        color /= totalWeight;
    }

    gl_FragColor = color;
}
    )";

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSrc);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);

    shaderProgram_ = glCreateProgram();
    glAttachShader(shaderProgram_, vertexShader);
    glAttachShader(shaderProgram_, fragmentShader);
    glLinkProgram(shaderProgram_);

    attrPos_ = glGetAttribLocation(shaderProgram_, "aPosition");
    attrTexCoord_ = glGetAttribLocation(shaderProgram_, "aTexCoord");

    uniformTexture_ = glGetUniformLocation(shaderProgram_, "uTexture");
    uniformRadius_ = glGetUniformLocation(shaderProgram_, "uRadius");
    uniformTextureSize_ = glGetUniformLocation(shaderProgram_, "uTextureSize");
    uniformInputSize_ = glGetUniformLocation(shaderProgram_, "uInputSize");
    uniformOutputSize_ = glGetUniformLocation(shaderProgram_, "uOutputSize");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}