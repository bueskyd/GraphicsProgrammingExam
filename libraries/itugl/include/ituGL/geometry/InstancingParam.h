#pragma once
#include <glad/glad.h>

class InstancingParam
{
public:
    InstancingParam() = default;
    InstancingParam(bool instanced, GLuint instanceCount);

    [[nodiscard]] bool Instanced() const;
    [[nodiscard]] GLuint GetInstanceCount() const;
    void SetInstanceCount(GLuint instanceCount);
private:
    bool m_instanced;
    GLuint m_instanceCount;
};
