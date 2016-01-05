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

#include <leaf3d/L3DRenderer.h>
#include <leaf3d/L3DLight.h>

using namespace l3d;

L3DLight::L3DLight(
    L3DRenderer* renderer,
    const L3DVec3& position,
    const L3DVec4& color,
    const L3DLightAttenuation& attenuation,
    bool on
) : L3DResource(L3D_LIGHT, renderer),
    position(position),
    color(color),
    attenuation(attenuation),
    isOn(on)
{
    if (renderer) renderer->addLight(this);
}

L3DLight::L3DLight(
    L3DRenderer* renderer,
    const L3DVec3& position,
    float kc, float kl, float kq,
    const L3DVec4& color,
    bool on
) : L3DResource(L3D_LIGHT, renderer),
    position(position),
    color(color),
    attenuation(kc, kl, kq),
    isOn(on)
{
    if (renderer) renderer->addLight(this);
}

void L3DLight::translate(const L3DVec3& movement)
{
    this->position += movement;
}
