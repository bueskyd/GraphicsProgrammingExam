#include "ituGL/geometry/InstancingParam.h"

InstancingParam::InstancingParam(bool instanced, GLuint instanceCount)
    : m_instanced(instanced), m_instanceCount(instanceCount)
{
}

bool InstancingParam::Instanced() const
{
    return m_instanced;
}

GLuint InstancingParam::GetInstanceCount() const
{
    return m_instanceCount;
}

void InstancingParam::SetInstanceCount(GLuint instanceCount)
{
    m_instanceCount = instanceCount;
}
