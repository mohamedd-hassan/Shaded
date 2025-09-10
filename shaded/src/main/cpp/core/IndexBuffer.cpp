#include "IndexBuffer.h"
#include "Renderer.h"

IndexBuffer::IndexBuffer(const void *data, unsigned int count)
    : m_Count(count)
{
    ASSERT(sizeof(unsigned int ) == sizeof (GLuint));

    GlCall(glGenBuffers(1, &m_RendererId));
    GlCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererId));
    GlCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW));
}

IndexBuffer::~IndexBuffer() {
    GlCall(glDeleteBuffers(1, &m_RendererId))
}

void IndexBuffer::Bind() const {
    GlCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererId));
}

void IndexBuffer::Unbind() const {
    GlCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}