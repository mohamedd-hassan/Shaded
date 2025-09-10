#include "blur_renderer.h"
#include "core/VertexBuffer.h"
#include "core/IndexBuffer.h"


BlurRenderer::BlurRenderer(int width, int height)
        : eglHelper_(width, height),
          quadVBO_(0), horizontalShaderProgram_(0), verticalShaderProgram_(0),
          attrPos_(-1), attrTexCoord_(-1),
          uniformTexture_(-1), uniformRadius_(-1), uniformTextureSize_(-1), uniformDirection_(-1),
          framebuffer1_(0), framebuffer2_(0), fboTexture1_(0), fboTexture2_(0), initialized_(false) {}

void BlurRenderer::initialize() {
    if (initialized_) return;

    eglHelper_.makeCurrent();

    compileShaders();
    setupFullscreenQuad();
    setupFramebuffers();

    initialized_ = true;
}

GLuint BlurRenderer::uploadBitmapAsTexture(unsigned char* pixels, int width, int height) {
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

void BlurRenderer::render(GLuint textureId, int width, int height, float radius) {
    initialize();
    eglHelper_.makeCurrent();

    if (radius <= 0.5f) {

        renderDirect(textureId, width, height);
        return;
    }

    resizeFramebuffers(width, height);

    renderHorizontalPass(textureId, width, height, radius);

    renderVerticalPass(width, height, radius);
}

void BlurRenderer::renderHorizontalPass(GLuint textureId, int width, int height, float radius) {
    glUseProgram(horizontalShaderProgram_);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer1_);
    glViewport(0, 0, width, height);

    glUniform1f(glGetUniformLocation(horizontalShaderProgram_, "uRadius"), radius);
    glUniform2f(glGetUniformLocation(horizontalShaderProgram_, "uTextureSize"),
                static_cast<float>(width), static_cast<float>(height));


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glUniform1i(glGetUniformLocation(horizontalShaderProgram_, "uTexture"), 0);

    drawQuad(horizontalShaderProgram_);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void BlurRenderer::renderVerticalPass(int width, int height, float radius) {
    glUseProgram(verticalShaderProgram_);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer2_);
    glViewport(0, 0, width, height);

    // Set uniforms
    glUniform1f(glGetUniformLocation(verticalShaderProgram_, "uRadius"), radius);
    glUniform2f(glGetUniformLocation(verticalShaderProgram_, "uTextureSize"),
                static_cast<float>(width), static_cast<float>(height));

    // Bind intermediate texture from horizontal pass
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fboTexture1_);
    glUniform1i(glGetUniformLocation(verticalShaderProgram_, "uTexture"), 0);

    drawQuad(verticalShaderProgram_);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void BlurRenderer::renderDirect(GLuint textureId, int width, int height) {
    // Simple pass-through for no blur
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer2_);
    glViewport(0, 0, width, height);

    // Clear and copy texture directly
    glClear(GL_COLOR_BUFFER_BIT);
    // You could implement a simple copy shader here, or just use the vertical shader with radius 0
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void BlurRenderer::drawQuad(GLuint shaderProgram) {
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

void BlurRenderer::readFBO(unsigned char* pixels, int width, int height) {
    eglHelper_.makeCurrent();
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer2_); // Read from final output
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void BlurRenderer::setupFullscreenQuad() {
    const float quadVertices[] = {
            -1.0f,  1.0f,   0.0f, 1.0f,
            -1.0f, -1.0f,   0.0f, 0.0f,
            1.0f,  1.0f,   1.0f, 1.0f,
            1.0f, -1.0f,   1.0f, 0.0f,
    };

    VertexBuffer vb(quadVertices, sizeof(quadVertices));
    vb.Unbind();
}

void BlurRenderer::setupFramebuffers() {
    // Create two framebuffers for ping-pong rendering
    glGenFramebuffers(1, &framebuffer1_);
    glGenFramebuffers(1, &framebuffer2_);

    glGenTextures(1, &fboTexture1_);
    glGenTextures(1, &fboTexture2_);

    currentWidth_ = 0;
    currentHeight_ = 0;
}

void BlurRenderer::resizeFramebuffers(int width, int height) {
    if (currentWidth_ == width && currentHeight_ == height) {
        return; // No resize needed
    }

    currentWidth_ = width;
    currentHeight_ = height;

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

GLuint BlurRenderer::compileShader(GLenum type, const char* source) {
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

void BlurRenderer::compileShaders() {
    const char* vertexShaderSrc = R"(
        attribute vec2 aPosition;
        attribute vec2 aTexCoord;
        varying vec2 vTexCoord;
        void main() {
            vTexCoord = aTexCoord;
            gl_Position = vec4(aPosition, 0.0, 1.0);
        }
    )";

    // Optimized horizontal blur fragment shader
    const char* horizontalFragmentShaderSrc = R"(
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
    )";

    // Optimized vertical blur fragment shader
    const char* verticalFragmentShaderSrc = R"(
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

            // Sample along vertical axis only
            for (int y = -iRadius; y <= iRadius; ++y) {
                float distance = abs(float(y));
                if (distance <= uRadius) {
                    vec2 sampleCoord = vTexCoord + vec2(0.0, float(y) * texelSize.y);
                    sampleCoord = clamp(sampleCoord, vec2(0.0), vec2(1.0));

                    float weight = exp(-(distance * distance) / twoSigmaSq);
                    color += texture2D(uTexture, sampleCoord) * weight;
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