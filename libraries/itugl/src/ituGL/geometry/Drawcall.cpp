#include <ituGL/geometry/Drawcall.h>

#include <ituGL/geometry/VertexArrayObject.h>
#include <ituGL/geometry/ElementBufferObject.h>
#include <cassert>

Drawcall::Drawcall()
    : m_primitive(Primitive::Invalid), m_first(0), m_count(0), m_eboType(Data::Type::None)
{
}

Drawcall::Drawcall(Primitive primitive, GLsizei count, GLint first)
    : Drawcall(primitive, count, Data::Type::None, first)
{
}

Drawcall::Drawcall(Primitive primitive, GLsizei count, Data::Type eboType, GLint first)
    : Drawcall(primitive, count, eboType, InstancingParam{}, first)
{
}

Drawcall::Drawcall(Primitive primitive, GLsizei count, InstancingParam instanced, GLint first)
    : Drawcall(primitive, count, Data::Type::None, instanced, first)
{

}

Drawcall::Drawcall(Primitive primitive, GLsizei count, Data::Type eboType, InstancingParam instanced, GLint first)
    : m_primitive(primitive), m_first(first), m_count(count), m_eboType(eboType), m_instancing(instanced)
{
    assert(primitive != Primitive::Invalid);
    assert(first >= 0);
    assert(count > 0);
}

// Execute the drawcall
void Drawcall::Draw() const
{
    assert(IsValid());
    assert(VertexArrayObject::IsAnyBound());

    GLenum primitive = static_cast<GLenum>(m_primitive);
    if (m_eboType == Data::Type::None)
    {
        // If no EBO is present, use either glDrawArrays or glDrawArraysInstanced
        if (m_instancing.Instanced())
            glDrawArraysInstanced(primitive, m_first, m_count, m_instancing.GetInstanceCount());
        else
            glDrawArrays(primitive, m_first, m_count);
    }
    else
    {
        // If there is an EBO, use either glDrawElements or glDrawElementsInstanced
        assert(ElementBufferObject::IsSupportedType(m_eboType));
        const char* basePointer = nullptr; // Actual element pointer is in VAO
        if (m_instancing.Instanced())
            glDrawElementsInstanced(primitive, m_count, static_cast<GLenum>(m_eboType), basePointer + m_first, m_instancing.GetInstanceCount());
        else
            glDrawElements(primitive, m_count, static_cast<GLenum>(m_eboType), basePointer + m_first);
    }
}

void Drawcall::SetInstanceCount(GLuint instanceCount)
{
    m_instancing.SetInstanceCount(instanceCount);
}
