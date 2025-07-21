#include "blur_renderer.h"

BlurRenderer::BlurRenderer(int width, int height)
        : eglHelper_(width, height),
          quadVBO_(0), shaderProgram_(0),
          attrPos_(-1), attrTexCoord_(-1),
          uniformTexture_(-1), uniformRadius_(-1), uniformTextureSize_(-1),
          framebuffer_(0), fboTexture_(0), initialized_(false) {}

void BlurRenderer::initialize() {
    if (initialized_) return;

    eglHelper_.makeCurrent();

    compileShaders();
    setupFullscreenQuad();
    setupFramebuffer();

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

    return textureId;
}

void BlurRenderer::render(GLuint textureId, int width, int height, float radius) {
    initialize();
    eglHelper_.makeCurrent();

    glUseProgram(shaderProgram_);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
    glViewport(0, 0, width, height);

    glUniform1f(uniformRadius_, radius);
    glUniform2f(uniformTextureSize_, static_cast<float>(width), static_cast<float>(height));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glUniform1i(uniformTexture_, 0);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO_);

    glEnableVertexAttribArray(attrPos_);
    glVertexAttribPointer(attrPos_, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(attrTexCoord_);
    glVertexAttribPointer(attrTexCoord_, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableVertexAttribArray(attrPos_);
    glDisableVertexAttribArray(attrTexCoord_);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void BlurRenderer::readFBO(unsigned char* pixels, int width, int height) {
    eglHelper_.makeCurrent();
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
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

    glGenBuffers(1, &quadVBO_);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void BlurRenderer::setupFramebuffer() {
    glGenFramebuffers(1, &framebuffer_);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);

    glGenTextures(1, &fboTexture_);
    glBindTexture(GL_TEXTURE_2D, fboTexture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4096, 4096, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture_, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint BlurRenderer::compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
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

    const char* fragmentShaderSrc = R"(
        precision mediump float;
        varying vec2 vTexCoord;
        uniform sampler2D uTexture;
        uniform float uRadius;
        uniform vec2 uTextureSize;

        void main() {
            vec2 texelSize = 1.0 / uTextureSize;
            vec4 color = vec4(0.0);
            float totalWeight = 0.0;
            float pixelRadius = uRadius;
            if (pixelRadius <= 0.5) {
                gl_FragColor = texture2D(uTexture, vTexCoord);
                return;
            }
            float sigma = pixelRadius / 2.0;
            int iRadius = int(ceil(pixelRadius));

            for (int x = -iRadius; x <= iRadius; ++x) {
                for (int y = -iRadius; y <= iRadius; ++y) {
                    float dist = sqrt(float(x * x + y * y));
                    if (dist <= pixelRadius) {
                        vec2 offset = vec2(float(x), float(y)) * texelSize;
                        vec2 sampleCoord = clamp(vTexCoord + offset, vec2(0.0), vec2(1.0));
                        float weight = exp(-(dist * dist) / (2.0 * sigma * sigma));
                        color += texture2D(uTexture, sampleCoord) * weight;
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

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}
