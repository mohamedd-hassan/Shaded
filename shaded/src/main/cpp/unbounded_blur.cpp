#include "unbounded_blur.h"
#include <cmath>
#include <algorithm>

UnboundedBlurRenderer::UnboundedBlurRenderer(int maxWidth, int maxHeight)
        : eglHelper_(maxWidth, maxHeight),
          quadVBO_(0), horizontalShaderProgram_(0), verticalShaderProgram_(0),
          attrPos_(-1), attrTexCoord_(-1),
          uniformTexture_(-1), uniformRadius_(-1), uniformTextureSize_(-1),
          uniformInputSize_(-1), uniformOutputSize_(-1),
          framebuffer1_(0), framebuffer2_(0), fboTexture1_(0), fboTexture2_(0),
          currentFBOWidth_(0), currentFBOHeight_(0),
          initialized_(false) {}

void UnboundedBlurRenderer::initialize() {
    if (initialized_) return;

    eglHelper_.makeCurrent();

    compileShaders();
    setupFullscreenQuad();
    setupFramebuffers();

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

    // Resize framebuffers if needed
    if (outputWidth != currentFBOWidth_ || outputHeight != currentFBOHeight_) {
        resizeFramebuffers(outputWidth, outputHeight);
    }

    if (radius <= 0.5f) {
        // No blur needed, just render directly
        renderDirect(textureId, inputWidth, inputHeight, outputWidth, outputHeight);
        return;
    }

    // First pass: Horizontal blur
    renderHorizontalPass(textureId, inputWidth, inputHeight, outputWidth, outputHeight, radius);

    // Second pass: Vertical blur
    renderVerticalPass(inputWidth, inputHeight, outputWidth, outputHeight, radius);
}

void UnboundedBlurRenderer::renderHorizontalPass(GLuint textureId, int inputWidth, int inputHeight,
                                                 int outputWidth, int outputHeight, float radius) {
    glUseProgram(horizontalShaderProgram_);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer1_);
    glViewport(0, 0, outputWidth, outputHeight);

    // Clear with transparent background
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set uniforms
    glUniform1f(glGetUniformLocation(horizontalShaderProgram_, "uRadius"), radius);
    glUniform2f(glGetUniformLocation(horizontalShaderProgram_, "uTextureSize"),
                static_cast<float>(inputWidth), static_cast<float>(inputHeight));
    glUniform2f(glGetUniformLocation(horizontalShaderProgram_, "uInputSize"),
                static_cast<float>(inputWidth), static_cast<float>(inputHeight));
    glUniform2f(glGetUniformLocation(horizontalShaderProgram_, "uOutputSize"),
                static_cast<float>(outputWidth), static_cast<float>(outputHeight));

    // Bind input texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glUniform1i(glGetUniformLocation(horizontalShaderProgram_, "uTexture"), 0);

    drawQuad(horizontalShaderProgram_);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void UnboundedBlurRenderer::renderVerticalPass(int inputWidth, int inputHeight,
                                               int outputWidth, int outputHeight, float radius) {
    glUseProgram(verticalShaderProgram_);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer2_);
    glViewport(0, 0, outputWidth, outputHeight);

    // Clear with transparent background
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set uniforms
    glUniform1f(glGetUniformLocation(verticalShaderProgram_, "uRadius"), radius);
    glUniform2f(glGetUniformLocation(verticalShaderProgram_, "uTextureSize"),
                static_cast<float>(outputWidth), static_cast<float>(outputHeight)); // Note: using output size for intermediate texture
    glUniform2f(glGetUniformLocation(verticalShaderProgram_, "uInputSize"),
                static_cast<float>(inputWidth), static_cast<float>(inputHeight));
    glUniform2f(glGetUniformLocation(verticalShaderProgram_, "uOutputSize"),
                static_cast<float>(outputWidth), static_cast<float>(outputHeight));

    // Bind intermediate texture from horizontal pass
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fboTexture1_);
    glUniform1i(glGetUniformLocation(verticalShaderProgram_, "uTexture"), 0);

    drawQuad(verticalShaderProgram_);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void UnboundedBlurRenderer::renderDirect(GLuint textureId, int inputWidth, int inputHeight,
                                         int outputWidth, int outputHeight) {
    // For no blur case, we still need to expand the canvas and center the image
    glUseProgram(horizontalShaderProgram_); // Reuse horizontal shader with radius 0
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer2_);
    glViewport(0, 0, outputWidth, outputHeight);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUniform1f(glGetUniformLocation(horizontalShaderProgram_, "uRadius"), 0.0f);
    glUniform2f(glGetUniformLocation(horizontalShaderProgram_, "uTextureSize"),
                static_cast<float>(inputWidth), static_cast<float>(inputHeight));
    glUniform2f(glGetUniformLocation(horizontalShaderProgram_, "uInputSize"),
                static_cast<float>(inputWidth), static_cast<float>(inputHeight));
    glUniform2f(glGetUniformLocation(horizontalShaderProgram_, "uOutputSize"),
                static_cast<float>(outputWidth), static_cast<float>(outputHeight));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glUniform1i(glGetUniformLocation(horizontalShaderProgram_, "uTexture"), 0);

    drawQuad(horizontalShaderProgram_);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void UnboundedBlurRenderer::drawQuad(GLuint shaderProgram) {
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO_);

    GLint posLoc = glGetAttribLocation(shaderProgram, "aPosition");
    GLint texLoc = glGetAttribLocation(shaderProgram, "aTexCoord");

    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(texLoc);
    glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableVertexAttribArray(posLoc);
    glDisableVertexAttribArray(texLoc);
}

void UnboundedBlurRenderer::readFBO(unsigned char *pixels, int width, int height) {
    eglHelper_.makeCurrent();
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer2_); // Read from final output
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

void UnboundedBlurRenderer::setupFramebuffers() {
    // Create two framebuffers for ping-pong rendering
    glGenFramebuffers(1, &framebuffer1_);
    glGenFramebuffers(1, &framebuffer2_);

    glGenTextures(1, &fboTexture1_);
    glGenTextures(1, &fboTexture2_);

    // Start with default sizes, will be resized as needed
    currentFBOWidth_ = 0;
    currentFBOHeight_ = 0;
}

void UnboundedBlurRenderer::resizeFramebuffers(int width, int height) {
    if (currentFBOWidth_ == width && currentFBOHeight_ == height) {
        return; // No resize needed
    }

    currentFBOWidth_ = width;
    currentFBOHeight_ = height;

    // Setup first framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer1_);
    glBindTexture(GL_TEXTURE_2D, fboTexture1_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture1_, 0);

    // Setup second framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer2_);
    glBindTexture(GL_TEXTURE_2D, fboTexture2_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture2_, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint UnboundedBlurRenderer::compileShader(GLenum type, const char *source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    // Check compilation status
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        // Log error here if needed
        glDeleteShader(shader);
        return 0;
    }

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

    // Optimized horizontal blur fragment shader for unbounded blur
    const char *horizontalFragmentShaderSrc = R"(
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
        if (distance <= uRadius) {
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
    )";

    // Optimized vertical blur fragment shader for unbounded blur
    const char *verticalFragmentShaderSrc = R"(
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
    )";

    // Compile vertex shader (shared)
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSrc);

    // Compile horizontal fragment shader
    GLuint horizontalFragmentShader = compileShader(GL_FRAGMENT_SHADER, horizontalFragmentShaderSrc);

    // Compile vertical fragment shader
    GLuint verticalFragmentShader = compileShader(GL_FRAGMENT_SHADER, verticalFragmentShaderSrc);

    // Create horizontal blur program
    horizontalShaderProgram_ = glCreateProgram();
    glAttachShader(horizontalShaderProgram_, vertexShader);
    glAttachShader(horizontalShaderProgram_, horizontalFragmentShader);
    glLinkProgram(horizontalShaderProgram_);

    // Create vertical blur program
    verticalShaderProgram_ = glCreateProgram();
    glAttachShader(verticalShaderProgram_, vertexShader);
    glAttachShader(verticalShaderProgram_, verticalFragmentShader);
    glLinkProgram(verticalShaderProgram_);

    // Clean up shaders
    glDeleteShader(vertexShader);
    glDeleteShader(horizontalFragmentShader);
    glDeleteShader(verticalFragmentShader);
}