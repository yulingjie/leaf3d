/*
 * This file is part of the leaf3d project.
 *
 * Copyright 2014-2015 Emanuele Bertoldi. All rights reserved.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * You should have received a copy of the modified BSD License along with this
 * program. If not, see <http://www.opensource.org/licenses/bsd-license.php>
 */

#include <stdio.h>
#include <leaf3d/L3DBuffer.h>
#include <leaf3d/L3DTexture.h>
#include <leaf3d/L3DShader.h>
#include <leaf3d/L3DShaderProgram.h>
#include <leaf3d/L3DMaterial.h>
#include <leaf3d/L3DCamera.h>
#include <leaf3d/L3DLight.h>
#include <leaf3d/L3DMesh.h>
#include <leaf3d/L3DRenderQueue.h>
#include <leaf3d/L3DRenderer.h>

using namespace l3d;

static GLenum _toOpenGL(const BufferType& orig)
{
    switch (orig)
    {
    case L3D_BUFFER_VERTEX:
        return GL_ARRAY_BUFFER;
    case L3D_BUFFER_INDEX:
        return GL_ELEMENT_ARRAY_BUFFER;
    default:
        break;
    }

    return 0;
}

static GLenum _toOpenGL(const DrawType& orig)
{
    switch (orig)
    {
    case L3D_DRAW_STATIC:
        return GL_STATIC_DRAW;
    case L3D_DRAW_DYNAMIC:
        return GL_DYNAMIC_DRAW;
    default:
        break;
    }

    return 0;
}

static GLenum _toOpenGL(const TextureType& orig)
{
    switch (orig)
    {
    case L3D_TEXTURE_1D:
        return GL_TEXTURE_1D;
    case L3D_TEXTURE_2D:
        return GL_TEXTURE_2D;
    case L3D_TEXTURE_3D:
        return GL_TEXTURE_3D;
    default:
        break;
    }

    return 0;
}

static GLenum _toOpenGL(const ImageFormat& orig)
{
    switch (orig)
    {
    case L3D_RGB:
        return GL_RGB;
    case L3D_RGBA:
        return GL_RGBA;
    default:
        break;
    }

    return 0;
}

static GLenum _toOpenGL(const DrawPrimitive& orig)
{
    switch (orig)
    {
    case L3D_DRAW_TRIANGLES:
        return GL_TRIANGLES;
    case L3D_DRAW_POINTS:
        return GL_POINTS;
    case L3D_DRAW_LINES:
        return GL_LINES;
    default:
        break;
    }

    return 0;
}

static GLenum _toOpenGL(const BlendFactor& orig)
{
    switch (orig)
    {
    case L3D_ZERO:
        return GL_ZERO;
    case L3D_ONE:
        return GL_ONE;
    case L3D_SRC_COLOR:
        return GL_SRC_COLOR;
    case L3D_ONE_MINUS_SRC_COLOR:
        return GL_ONE_MINUS_SRC_COLOR;
    case L3D_DST_COLOR:
        return GL_DST_COLOR;
    case L3D_ONE_MINUS_DST_COLOR:
        return GL_ONE_MINUS_DST_COLOR;
    case L3D_SRC_ALPHA:
        return GL_SRC_ALPHA;
    case L3D_ONE_MINUS_SRC_ALPHA:
        return GL_ONE_MINUS_SRC_ALPHA;
    case L3D_DST_ALPHA:
        return GL_DST_ALPHA;
    case L3D_ONE_MINUS_DST_ALPHA:
        return GL_ONE_MINUS_DST_ALPHA;
    case L3D_CONSTANT_COLOR:
        return GL_CONSTANT_COLOR;
    case L3D_ONE_MINUS_CONSTANT_COLOR:
        return GL_ONE_MINUS_CONSTANT_COLOR;
    case L3D_CONSTANT_ALPHA:
        return GL_CONSTANT_ALPHA;
    case L3D_ONE_MINUS_CONSTANT_ALPHA:
        return GL_ONE_MINUS_CONSTANT_ALPHA;
    default:
        break;
    }

    return 0;
}

static GLenum _toOpenGL(const ShaderType& orig)
{
    switch (orig)
    {
    case L3D_SHADER_VERTEX:
        return GL_VERTEX_SHADER;
    case L3D_SHADER_FRAGMENT:
        return GL_FRAGMENT_SHADER;
    case L3D_SHADER_GEOMETRY:
        return GL_GEOMETRY_SHADER;
    }

    return 0;
}

static void _enableVertexAttribute(
    GLuint attrib,
    GLint size,
    GLenum type,
    GLsizei stride,
    void* startPtr = 0,
    GLboolean normalized = GL_FALSE
)
{
    glEnableVertexAttribArray(attrib);
    glVertexAttribPointer(attrib, size, type, normalized, stride, startPtr);
}

L3DRenderer::L3DRenderer()
{
}

L3DRenderer::~L3DRenderer()
{
    this->terminate();
}

int L3DRenderer::init()
{
    // Init GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    return L3D_TRUE;
}

int L3DRenderer::terminate()
{
    for (L3DBufferPool::reverse_iterator it = m_buffers.rbegin(); it != m_buffers.rend(); ++it)
        delete it->second;
    m_buffers.clear();

    for (L3DTexturePool::reverse_iterator it = m_textures.rbegin(); it != m_textures.rend(); ++it)
        delete it->second;
    m_textures.clear();

    for (L3DShaderPool::reverse_iterator it = m_shaders.rbegin(); it != m_shaders.rend(); ++it)
        delete it->second;
    m_shaders.clear();

    for (L3DShaderProgramPool::reverse_iterator it = m_shaderPrograms.rbegin(); it != m_shaderPrograms.rend(); ++it)
        delete it->second;
    m_shaderPrograms.clear();

    for (L3DMaterialPool::reverse_iterator it = m_materials.rbegin(); it != m_materials.rend(); ++it)
        delete it->second;
    m_materials.clear();

    for (L3DCameraPool::reverse_iterator it = m_cameras.rbegin(); it != m_cameras.rend(); ++it)
        delete it->second;
    m_cameras.clear();

    for (L3DLightPool::reverse_iterator it = m_lights.rbegin(); it != m_lights.rend(); ++it)
        delete it->second;
    m_lights.clear();

    for (L3DMeshPool::reverse_iterator it = m_meshes.rbegin(); it != m_meshes.rend(); ++it)
        delete it->second;
    m_meshes.clear();

    for (L3DRenderQueuePool::reverse_iterator it = m_renderQueues.rbegin(); it != m_renderQueues.rend(); ++it)
        delete it->second;
    m_renderQueues.clear();

    return L3D_TRUE;
}

void L3DRenderer::renderFrame(L3DCamera* camera, L3DRenderQueue* renderQueue)
{
    if (!camera || !renderQueue)
        return;

    const L3DRenderCommandList& commands = renderQueue->commands();

    for (L3DRenderCommandList::const_iterator it = commands.begin(); it != commands.end(); ++it)
    {
        const L3DRenderCommand* command = *it;

        switch (command->type())
        {
        case L3D_CLEAR_BUFFERS:
        {
            const L3DClearBuffersCommand* cmd = static_cast<const L3DClearBuffersCommand*>(command);
            if (cmd)
                this->clearBuffers(cmd->colorBuffer, cmd->depthBuffer, cmd->stencilBuffer, cmd->clearColor);
        }
        break;

        case L3D_SET_DEPTH_TEST:
        {
            const L3DSetDepthTestCommand* cmd = static_cast<const L3DSetDepthTestCommand*>(command);
            if (cmd)
                this->setDepthTest(cmd->enable);
        }
        break;

        case L3D_SET_STENCIL_TEST:
        {
            const L3DSetStencilTestCommand* cmd = static_cast<const L3DSetStencilTestCommand*>(command);
            if (cmd)
                this->setStencilTest(cmd->enable);
        }
        break;

        case L3D_SET_BLEND:
        {
            const L3DSetBlendCommand* cmd = static_cast<const L3DSetBlendCommand*>(command);
            if (cmd)
                this->setBlend(cmd->enable, cmd->srcFactor, cmd->dstFactor);
        }
        break;

        case L3D_DRAW_MESHES:
            this->drawMeshes(camera);
        break;

        default:
        break;
        }
    }
}

void L3DRenderer::addResource(L3DResource* resource)
{
    if (resource)
    {
        switch(resource->resourceType())
        {
        case L3D_BUFFER:
            this->addBuffer(static_cast<L3DBuffer*>(resource));
            break;
        case L3D_TEXTURE:
            this->addTexture(static_cast<L3DTexture*>(resource));
            break;
        case L3D_SHADER:
            this->addShader(static_cast<L3DShader*>(resource));
            break;
        case L3D_SHADER_PROGRAM:
            this->addShaderProgram(static_cast<L3DShaderProgram*>(resource));
            break;
        case L3D_MATERIAL:
            this->addMaterial(static_cast<L3DMaterial*>(resource));
            break;
        case L3D_CAMERA:
            this->addCamera(static_cast<L3DCamera*>(resource));
            break;
        case L3D_LIGHT:
            this->addLight(static_cast<L3DLight*>(resource));
            break;
        case L3D_MESH:
            this->addMesh(static_cast<L3DMesh*>(resource));
            break;
        case L3D_RENDER_QUEUE:
            this->addRenderQueue(static_cast<L3DRenderQueue*>(resource));
            break;
        default:
            break;
        }
    }
}

void L3DRenderer::addBuffer(L3DBuffer* buffer)
{
    if (buffer && m_buffers.find(buffer->id()) == m_buffers.end())
    {
        GLuint id = 0;
        glGenBuffers(1, &id);

        if (buffer->count())
        {
            GLenum gl_type = _toOpenGL(buffer->type());
            GLenum gl_draw_type = _toOpenGL(buffer->drawType());

            glBindBuffer(gl_type, id);
            glBufferData(gl_type, buffer->size(), buffer->data(), gl_draw_type);
            glBindBuffer(gl_type, 0);
        }

        buffer->setId(id);

        m_buffers[id] = buffer;
    }
}

void L3DRenderer::addTexture(L3DTexture* texture)
{
    if (texture && m_textures.find(texture->id()) == m_textures.end())
    {
        GLuint id = 0;
        glGenTextures(1, &id);

        GLenum gl_format = _toOpenGL(texture->format());
        GLenum gl_type = _toOpenGL(texture->type());

        glBindTexture(gl_type, id);

        switch (texture->type())
        {
        case L3D_TEXTURE_1D:
            glTexImage1D(gl_type, 0, gl_format, texture->width(), 0, gl_format, GL_UNSIGNED_BYTE, texture->data());
            break;
        case L3D_TEXTURE_2D:
            glTexImage2D(gl_type, 0, gl_format, texture->width(), texture->height(), 0, gl_format, GL_UNSIGNED_BYTE, texture->data());
            break;
        case L3D_TEXTURE_3D:
            glTexImage3D(gl_type, 0, gl_format, texture->width(), texture->height(), texture->depth(), 0, gl_format, GL_UNSIGNED_BYTE, texture->data());
            break;
        default:
            // Don't store id. Free resource.
            glBindTexture(gl_type, 0);
            glDeleteTextures(1, &id);
            return;
        }

        glGenerateMipmap(gl_type);
        glTexParameteri(gl_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(gl_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(gl_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(gl_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        glBindTexture(gl_type, 0);

        texture->setId(id);

        m_textures[id] = texture;
    }
}

void L3DRenderer::addShader(L3DShader* shader)
{
    if (shader && m_shaders.find(shader->id()) == m_shaders.end())
    {
        GLuint gl_type = _toOpenGL(shader->type());

        const char* code = shader->code();

        GLuint id = glCreateShader(gl_type);
        glShaderSource(id, 1, &code, L3D_NULLPTR);
        glCompileShader(id);

        shader->setId(id);

        m_shaders[id] = shader;
    }
}

void L3DRenderer::addShaderProgram(L3DShaderProgram* shaderProgram)
{
    if (shaderProgram && m_shaderPrograms.find(shaderProgram->id()) == m_shaderPrograms.end())
    {
        GLuint id = glCreateProgram();

        if (shaderProgram->vertexShader())
            glAttachShader(id, shaderProgram->vertexShader()->id());

        if (shaderProgram->fragmentShader())
            glAttachShader(id, shaderProgram->fragmentShader()->id());

        if (shaderProgram->geometryShader())
            glAttachShader(id, shaderProgram->geometryShader()->id());

        glLinkProgram(id);

        shaderProgram->setId(id);

        m_shaderPrograms[id] = shaderProgram;
    }
}

void L3DRenderer::addMaterial(L3DMaterial* material)
{
    if (material)
    {
        GLuint id = 1;
        if (m_materials.size() > 0)
            id = m_materials.rbegin()->second->id() + 1;

        material->setId(id);

        m_materials[id] = material;
    }
}

void L3DRenderer::addCamera(L3DCamera* camera)
{
    if (camera)
    {
        GLuint id = 1;
        if (m_cameras.size() > 0)
            id = m_cameras.rbegin()->second->id() + 1;

        camera->setId(id);

        m_cameras[id] = camera;
    }
}

void L3DRenderer::addLight(L3DLight* light)
{
    if (light)
    {
        GLuint id = 1;
        if (m_lights.size() > 0)
            id = m_lights.rbegin()->second->id() + 1;

        light->setId(id);

        m_lights[id] = light;
    }
}

void L3DRenderer::addMesh(L3DMesh* mesh)
{
    if (mesh)
    {
        GLuint id;
        glGenVertexArrays(1, &id);
        glBindVertexArray(id);

        if (mesh->vertexBuffer() && mesh->vertexCount())
        {
            this->addBuffer(mesh->vertexBuffer());

            // Bind vertex buffer.
            glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer()->id());

            if (mesh->material() && mesh->material()->shaderProgram())
            {
                L3DMaterial* material = mesh->material();
                L3DShaderProgram* shaderProgram = material->shaderProgram();

                // Enable vertex attributes.
                GLint posAttrib     = glGetAttribLocation(shaderProgram->id(), "position");
                GLint colAttrib     = glGetAttribLocation(shaderProgram->id(), "color");
                GLint tex0Attrib    = glGetAttribLocation(shaderProgram->id(), "texcoord0");
                GLint tex1Attrib    = glGetAttribLocation(shaderProgram->id(), "texcoord1");
                GLint tex2Attrib    = glGetAttribLocation(shaderProgram->id(), "texcoord2");
                GLint tex3Attrib    = glGetAttribLocation(shaderProgram->id(), "texcoord3");
                GLint norAttrib     = glGetAttribLocation(shaderProgram->id(), "normal");
                GLint tanAttrib     = glGetAttribLocation(shaderProgram->id(), "tan");
                GLint btanAttrib    = glGetAttribLocation(shaderProgram->id(), "btan");

                switch(mesh->vertexFormat())
                {
                case L3D_POS2:
                    _enableVertexAttribute(posAttrib, 2, GL_FLOAT, 2*sizeof(GLfloat), 0);
                    break;
                case L3D_POS3:
                    _enableVertexAttribute(posAttrib, 3, GL_FLOAT, 3*sizeof(GLfloat), 0);
                    break;
                case L3D_POS2_UV2:
                    _enableVertexAttribute(posAttrib, 2, GL_FLOAT, 4*sizeof(GLfloat), 0);
                    _enableVertexAttribute(tex0Attrib, 2, GL_FLOAT, 4*sizeof(GLfloat), (void*)(2*sizeof(GLfloat)));
                    break;
                case L3D_POS3_UV2:
                    _enableVertexAttribute(posAttrib, 3, GL_FLOAT, 5*sizeof(GLfloat), 0);
                    _enableVertexAttribute(tex0Attrib, 2, GL_FLOAT, 5*sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));
                    break;
                case L3D_POS2_COL3_UV2:
                    _enableVertexAttribute(posAttrib, 2, GL_FLOAT, 7*sizeof(GLfloat), 0);
                    _enableVertexAttribute(colAttrib, 3, GL_FLOAT, 7*sizeof(GLfloat), (void*)(2*sizeof(GLfloat)));
                    _enableVertexAttribute(tex0Attrib, 2, GL_FLOAT, 7*sizeof(GLfloat), (void*)(5*sizeof(GLfloat)));
                    break;
                case L3D_POS3_NOR3_UV2:
                    _enableVertexAttribute(posAttrib, 3, GL_FLOAT, 8*sizeof(GLfloat), 0);
                    _enableVertexAttribute(norAttrib, 3, GL_FLOAT, 8*sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));
                    _enableVertexAttribute(tex0Attrib, 2, GL_FLOAT, 8*sizeof(GLfloat), (void*)(6*sizeof(GLfloat)));
                    break;
                case L3D_POS3_NOR3_COL3_UV2:
                    _enableVertexAttribute(posAttrib, 3, GL_FLOAT, 11*sizeof(GLfloat), 0);
                    _enableVertexAttribute(norAttrib, 3, GL_FLOAT, 11*sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));
                    _enableVertexAttribute(colAttrib, 3, GL_FLOAT, 11*sizeof(GLfloat), (void*)(6*sizeof(GLfloat)));
                    _enableVertexAttribute(tex0Attrib, 2, GL_FLOAT, 11*sizeof(GLfloat), (void*)(9*sizeof(GLfloat)));
                    break;
                case L3D_POS3_NOR3_TAN3_COL3_UV2:
                    _enableVertexAttribute(posAttrib, 3, GL_FLOAT, 14*sizeof(GLfloat), 0);
                    _enableVertexAttribute(norAttrib, 3, GL_FLOAT, 14*sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));
                    _enableVertexAttribute(tanAttrib, 3, GL_FLOAT, 14*sizeof(GLfloat), (void*)(6*sizeof(GLfloat)));
                    _enableVertexAttribute(colAttrib, 3, GL_FLOAT, 14*sizeof(GLfloat), (void*)(9*sizeof(GLfloat)));
                    _enableVertexAttribute(tex0Attrib, 2, GL_FLOAT, 14*sizeof(GLfloat), (void*)(12*sizeof(GLfloat)));
                    break;
                case L3D_POS3_NOR3_TAN3_BTAN3_COL3_UV2:
                    _enableVertexAttribute(posAttrib, 3, GL_FLOAT, 17*sizeof(GLfloat), 0);
                    _enableVertexAttribute(norAttrib, 3, GL_FLOAT, 17*sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));
                    _enableVertexAttribute(tanAttrib, 3, GL_FLOAT, 17*sizeof(GLfloat), (void*)(6*sizeof(GLfloat)));
                    _enableVertexAttribute(btanAttrib, 3, GL_FLOAT, 17*sizeof(GLfloat), (void*)(9*sizeof(GLfloat)));
                    _enableVertexAttribute(colAttrib, 3, GL_FLOAT, 17*sizeof(GLfloat), (void*)(12*sizeof(GLfloat)));
                    _enableVertexAttribute(tex0Attrib, 2, GL_FLOAT, 17*sizeof(GLfloat), (void*)(15*sizeof(GLfloat)));
                    break;
                case L3D_POS3_NOR3_TAN3_BTAN3_COL3_UV2_UV2:
                    _enableVertexAttribute(posAttrib, 3, GL_FLOAT, 19*sizeof(GLfloat), 0);
                    _enableVertexAttribute(norAttrib, 3, GL_FLOAT, 19*sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));
                    _enableVertexAttribute(tanAttrib, 3, GL_FLOAT, 19*sizeof(GLfloat), (void*)(6*sizeof(GLfloat)));
                    _enableVertexAttribute(btanAttrib, 3, GL_FLOAT, 19*sizeof(GLfloat), (void*)(9*sizeof(GLfloat)));
                    _enableVertexAttribute(colAttrib, 3, GL_FLOAT, 19*sizeof(GLfloat), (void*)(12*sizeof(GLfloat)));
                    _enableVertexAttribute(tex0Attrib, 2, GL_FLOAT, 19*sizeof(GLfloat), (void*)(15*sizeof(GLfloat)));
                    _enableVertexAttribute(tex1Attrib, 2, GL_FLOAT, 19*sizeof(GLfloat), (void*)(17*sizeof(GLfloat)));
                    break;
                case L3D_POS3_NOR3_TAN3_BTAN3_COL3_UV2_UV2_UV2:
                    _enableVertexAttribute(posAttrib, 3, GL_FLOAT, 21*sizeof(GLfloat), 0);
                    _enableVertexAttribute(norAttrib, 3, GL_FLOAT, 21*sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));
                    _enableVertexAttribute(tanAttrib, 3, GL_FLOAT, 21*sizeof(GLfloat), (void*)(6*sizeof(GLfloat)));
                    _enableVertexAttribute(btanAttrib, 3, GL_FLOAT, 21*sizeof(GLfloat), (void*)(9*sizeof(GLfloat)));
                    _enableVertexAttribute(colAttrib, 3, GL_FLOAT, 21*sizeof(GLfloat), (void*)(12*sizeof(GLfloat)));
                    _enableVertexAttribute(tex0Attrib, 2, GL_FLOAT, 21*sizeof(GLfloat), (void*)(15*sizeof(GLfloat)));
                    _enableVertexAttribute(tex1Attrib, 2, GL_FLOAT, 21*sizeof(GLfloat), (void*)(17*sizeof(GLfloat)));
                    _enableVertexAttribute(tex2Attrib, 2, GL_FLOAT, 21*sizeof(GLfloat), (void*)(19*sizeof(GLfloat)));
                    break;
                case L3D_POS3_NOR3_TAN3_BTAN3_COL3_UV2_UV2_UV2_UV2:
                    _enableVertexAttribute(posAttrib, 3, GL_FLOAT, 23*sizeof(GLfloat), 0);
                    _enableVertexAttribute(norAttrib, 3, GL_FLOAT, 23*sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));
                    _enableVertexAttribute(tanAttrib, 3, GL_FLOAT, 23*sizeof(GLfloat), (void*)(6*sizeof(GLfloat)));
                    _enableVertexAttribute(btanAttrib, 3, GL_FLOAT, 23*sizeof(GLfloat), (void*)(9*sizeof(GLfloat)));
                    _enableVertexAttribute(colAttrib, 3, GL_FLOAT, 23*sizeof(GLfloat), (void*)(12*sizeof(GLfloat)));
                    _enableVertexAttribute(tex0Attrib, 2, GL_FLOAT, 23*sizeof(GLfloat), (void*)(15*sizeof(GLfloat)));
                    _enableVertexAttribute(tex1Attrib, 2, GL_FLOAT, 23*sizeof(GLfloat), (void*)(17*sizeof(GLfloat)));
                    _enableVertexAttribute(tex2Attrib, 2, GL_FLOAT, 23*sizeof(GLfloat), (void*)(19*sizeof(GLfloat)));
                    _enableVertexAttribute(tex3Attrib, 2, GL_FLOAT, 23*sizeof(GLfloat), (void*)(21*sizeof(GLfloat)));
                    break;
                default:
                    glDeleteVertexArrays(1, &id);
                    return;
                }
            }
        }

        if (mesh->indexBuffer() && mesh->indexCount())
        {
            this->addBuffer(mesh->indexBuffer());

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer()->id());
        }

        glBindVertexArray(0);

        mesh->setId(id);

        m_meshes[id] = mesh;
    }
}

void L3DRenderer::addRenderQueue(L3DRenderQueue* renderQueue)
{
    if (renderQueue)
    {
        GLuint id = 1;
        if (m_renderQueues.size() > 0)
            id = m_renderQueues.rbegin()->second->id() + 1;

        renderQueue->setId(id);

        m_renderQueues[id] = renderQueue;
    }
}

void L3DRenderer::removeResource(L3DResource* resource)
{
    if (resource)
    {
        switch(resource->resourceType())
        {
        case L3D_BUFFER:
            this->removeBuffer(static_cast<L3DBuffer*>(resource));
            break;
        case L3D_TEXTURE:
            this->removeTexture(static_cast<L3DTexture*>(resource));
            break;
        case L3D_SHADER:
            this->removeShader(static_cast<L3DShader*>(resource));
            break;
        case L3D_SHADER_PROGRAM:
            this->removeShaderProgram(static_cast<L3DShaderProgram*>(resource));
            break;
        case L3D_MATERIAL:
            this->removeMaterial(static_cast<L3DMaterial*>(resource));
            break;
        case L3D_CAMERA:
            this->removeCamera(static_cast<L3DCamera*>(resource));
            break;
        case L3D_LIGHT:
            this->removeLight(static_cast<L3DLight*>(resource));
            break;
        case L3D_MESH:
            this->removeMesh(static_cast<L3DMesh*>(resource));
            break;
        case L3D_RENDER_QUEUE:
            this->removeRenderQueue(static_cast<L3DRenderQueue*>(resource));
            break;
        default:
            break;
        }
    }
}

void L3DRenderer::removeBuffer(L3DBuffer* buffer)
{
    if (buffer)
    {
        GLuint id = buffer->id();
        m_buffers[id] = L3D_NULLPTR;
        glDeleteBuffers(1, &id);
        buffer->setId(0);
    }
}

void L3DRenderer::removeTexture(L3DTexture* texture)
{
    if (texture)
    {
        GLuint id = texture->id();
        m_textures[id] = L3D_NULLPTR;
        glDeleteTextures(1, &id);
        texture->setId(0);
    }
}

void L3DRenderer::removeShader(L3DShader* shader)
{
    if (shader)
    {
        GLuint id = shader->id();
        m_shaders[id] = L3D_NULLPTR;
        glDeleteShader(id);
        shader->setId(0);
    }
}

void L3DRenderer::removeShaderProgram(L3DShaderProgram* shaderProgram)
{
    if (shaderProgram)
    {
        GLuint id = shaderProgram->id();
        m_shaderPrograms[id] = L3D_NULLPTR;
        glDeleteProgram(id);
        shaderProgram->setId(0);
    }
}

void L3DRenderer::removeMaterial(L3DMaterial* material)
{
    if (material)
    {
        GLuint id = material->id();
        m_materials[id] = L3D_NULLPTR;
        // TODO: clean material resources.
        material->setId(0);
    }
}

void L3DRenderer::removeCamera(L3DCamera* camera)
{
    if (camera)
    {
        GLuint id = camera->id();
        m_cameras[id] = L3D_NULLPTR;
        // TODO: clean camera resources.
        camera->setId(0);
    }
}

void L3DRenderer::removeLight(L3DLight* light)
{
    if (light)
    {
        GLuint id = light->id();
        m_lights[id] = L3D_NULLPTR;
        // TODO: clean light resources.
        light->setId(0);
    }
}

void L3DRenderer::removeMesh(L3DMesh* mesh)
{
    if (mesh)
    {
        GLuint id = mesh->id();
        m_meshes[id] = L3D_NULLPTR;
        glDeleteVertexArrays(1, &id);
        mesh->setId(0);
    }
}

void L3DRenderer::removeRenderQueue(L3DRenderQueue* renderQueue)
{
    if (renderQueue)
    {
        GLuint id = renderQueue->id();
        m_renderQueues[id] = L3D_NULLPTR;
        // TODO: clean render queue resources.
        renderQueue->setId(0);
    }
}

L3DResource* L3DRenderer::getResource(const L3DHandle& handle) const
{
    switch(handle.data.type)
    {
    case L3D_BUFFER:
        return m_buffers.find(handle.data.id)->second;
    case L3D_TEXTURE:
        return m_textures.find(handle.data.id)->second;
    case L3D_SHADER:
        return m_shaders.find(handle.data.id)->second;
    case L3D_SHADER_PROGRAM:
        return m_shaderPrograms.find(handle.data.id)->second;
    case L3D_MATERIAL:
        return m_materials.find(handle.data.id)->second;
    case L3D_CAMERA:
        return m_cameras.find(handle.data.id)->second;
    case L3D_LIGHT:
        return m_lights.find(handle.data.id)->second;
    case L3D_MESH:
        return m_meshes.find(handle.data.id)->second;
    case L3D_RENDER_QUEUE:
        return m_renderQueues.find(handle.data.id)->second;
    default:
        return L3D_NULLPTR;
    }
}

L3DBuffer* L3DRenderer::getBuffer(const L3DHandle& handle) const
{
    if (handle.data.type == L3D_BUFFER)
        return m_buffers.find(handle.data.id)->second;

    return L3D_NULLPTR;
}

L3DTexture* L3DRenderer::getTexture(const L3DHandle& handle) const
{
    if (handle.data.type == L3D_TEXTURE)
        return m_textures.find(handle.data.id)->second;

    return L3D_NULLPTR;
}

L3DShader* L3DRenderer::getShader(const L3DHandle& handle) const
{
    if (handle.data.type == L3D_SHADER)
        return m_shaders.find(handle.data.id)->second;

    return L3D_NULLPTR;
}

L3DShaderProgram* L3DRenderer::getShaderProgram(const L3DHandle& handle) const
{
    if (handle.data.type == L3D_SHADER_PROGRAM)
        return m_shaderPrograms.find(handle.data.id)->second;

    return L3D_NULLPTR;
}

L3DMaterial* L3DRenderer::getMaterial(const L3DHandle& handle) const
{
    if (handle.data.type == L3D_MATERIAL)
        return m_materials.find(handle.data.id)->second;

    return L3D_NULLPTR;
}

L3DCamera* L3DRenderer::getCamera(const L3DHandle& handle) const
{
    if (handle.data.type == L3D_CAMERA)
        return m_cameras.find(handle.data.id)->second;

    return L3D_NULLPTR;
}

L3DLight* L3DRenderer::getLight(const L3DHandle& handle) const
{
    if (handle.data.type == L3D_LIGHT)
        return m_lights.find(handle.data.id)->second;

    return L3D_NULLPTR;
}

L3DMesh* L3DRenderer::getMesh(const L3DHandle& handle) const
{
    if (handle.data.type == L3D_MESH)
        return m_meshes.find(handle.data.id)->second;

    return L3D_NULLPTR;
}

L3DRenderQueue* L3DRenderer::getRenderQueue(const L3DHandle& handle) const
{
    if (handle.data.type == L3D_RENDER_QUEUE)
        return m_renderQueues.find(handle.data.id)->second;

    return L3D_NULLPTR;
}

void L3DRenderer::clearBuffers(
    bool colorBuffer,
    bool depthBuffer,
    bool stencilBuffer,
    const L3DVec4& clearColor
)
{
    unsigned int clearMask = 0;
    if (colorBuffer) clearMask |= GL_COLOR_BUFFER_BIT;
    if (depthBuffer) clearMask |= GL_DEPTH_BUFFER_BIT;
    if (stencilBuffer) clearMask |= GL_STENCIL_BUFFER_BIT;

    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(clearMask);
}

void L3DRenderer::setDepthTest(bool enable)
{
    enable ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
}

void L3DRenderer::setStencilTest(bool enable)
{
    enable ? glEnable(GL_STENCIL_TEST) : glDisable(GL_STENCIL_TEST);
}

void L3DRenderer::setBlend(
    bool enable,
    const BlendFactor& srcFactor,
    const BlendFactor& dstFactor
)
{
    enable ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
    glBlendFunc(_toOpenGL(srcFactor), _toOpenGL(dstFactor));
}

void L3DRenderer::drawMeshes(L3DCamera* camera)
{
    for (L3DMeshPool::iterator it = m_meshes.begin(); it != m_meshes.end(); ++it)
    {
        L3DMesh* mesh = it->second;

        if (mesh && mesh->material() && mesh->material()->shaderProgram())
        {
            L3DMaterial* material = mesh->material();
            L3DShaderProgram* shaderProgram = material->shaderProgram();
            GLenum gl_draw_primitive = _toOpenGL(mesh->drawPrimitive());
            unsigned int index_count = mesh->indexCount();

            // Bind VAO.
            glBindVertexArray(mesh->id());

            // Bind shaders.
            glUseProgram(shaderProgram->id());

            // Bind textures.
            unsigned int i = 0;
            for (L3DTextureRegistry::iterator tex_it = material->textures.begin(); tex_it!=material->textures.end(); ++tex_it)
            {
                L3DTexture* texture = tex_it->second;
                GLenum gl_type = _toOpenGL(texture->type());
                GLint gl_sampler = glGetUniformLocation(shaderProgram->id(), tex_it->first);

                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(gl_type, texture->id());
                glUniform1i(gl_sampler, i);

                ++i;
            }

            // Bind uniforms.
            L3DUniformMap uniforms = shaderProgram->uniforms();
            for (L3DUniformMap::iterator unif_it = uniforms.begin(); unif_it!=uniforms.end(); ++unif_it)
            {
                L3DUniform uniform = unif_it->second;
                GLint gl_location = glGetUniformLocation(shaderProgram->id(), unif_it->first);

                switch (uniform.type)
                {
                case L3D_UNIFORM_FLOAT:
                    glUniform1f(gl_location, uniform.value.valueF);
                    break;
                case L3D_UNIFORM_INT:
                    glUniform1i(gl_location, uniform.value.valueI);
                    break;
                case L3D_UNIFORM_UINT:
                    glUniform1ui(gl_location, uniform.value.valueUI);
                    break;
                case L3D_UNIFORM_MAT4:
                    glUniformMatrix4fv(gl_location, 1, GL_FALSE, uniform.value.valueMat4);
                    break;
                }
            }

            // Bind matrices.
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram->id(), "view"), 1, GL_FALSE, glm::value_ptr(camera->view));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram->id(), "proj"), 1, GL_FALSE, glm::value_ptr(camera->proj));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram->id(), "model"), 1, GL_FALSE, glm::value_ptr(mesh->trans));

            // Render geometry.
            if (index_count > 0)
            {
                // Render vertices using indices.
                glDrawElements(gl_draw_primitive, index_count, GL_UNSIGNED_INT, 0);
            }
            else
            {
                // Render vertices without using indices.
                glDrawArrays(gl_draw_primitive, 0, mesh->vertexCount());
            }
        }
    }

    glBindVertexArray(0);
}
