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

#ifndef L3D_L3DMATERIAL_H
#define L3D_L3DMATERIAL_H
#pragma once

#include "leaf3d/L3DResource.h"

namespace l3d
{
    class L3DShaderProgram;

    class L3DMaterial : public L3DResource
    {
    private:
        const char* m_name;
        L3DShaderProgram* m_shaderProgram;

    public:
        L3DMaterial(
            const char* name,
            L3DShaderProgram* shaderProgram
        );
        ~L3DMaterial();

        const char* name() const { return m_name; }
        L3DShaderProgram* shaderProgram() const { return m_shaderProgram; }

        void setUniform(const char* name, int value);
        void setUniform(const char* name, float value);
        void setUniform(const char* name, const L3DMat4& value);
    };
}

#endif // L3D_L3DMATERIAL_H

