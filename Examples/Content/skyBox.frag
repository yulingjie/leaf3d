#version 330 core

/* INPUTS *********************************************************************/

// Data from vertex shader.
// Direction vector representing a 3D texture coordinate.
in vec3 o_textureDir;

/* UNIFORMS *******************************************************************/

// Cubemap texture sampler.
uniform samplerCube u_cubemap;

/* MAIN ***********************************************************************/

void main()
{
    // Final fragment color.
    gl_FragColor = texture(u_cubemap, o_textureDir);
}
