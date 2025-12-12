#include "sdl.h"
#include "glad/glad.h"
#include <stdio.h>
#include <stdlib.h>

#include "shaders/magic_shader.h"


void sdl_alphaeffect_magic(int nr, int str, int xpos, int ypos, int xoff, int yoff) {
    // Flush batch before rendering text (text uses legacy uniform path)
    sdl_batch_flush();

    // Convert world coordinates to screen coordinates (isometric projection)
    int rx = (xpos / 2) + (ypos / 2) - (2 * 16) + 32 + X_OFFSET -
             ((RENDER_DISTANCE - 34) / 2 * 32);

    if (xpos < 0 && (xpos & 1)) rx--;
    if (ypos < 0 && (ypos & 1)) rx--;

    int ry = (xpos / 4) - (ypos / 4) + Y_OFFSET - 2 * 32;

    if (xpos < 0 && (xpos & 3)) ry--;
    if (ypos < 0 && (ypos & 3)) ry++;

    rx += xoff;
    ry += yoff;

    GLint model = use_magic_shader((RGBColor){
                                       (nr & 1) != 0 ? 1.0f : 0.0f,
                                       (nr & 2) != 0 ? 1.0f : 0.0f,
                                       (nr & 4) != 0 ? 1.0f : 0.0f
                                   }, str);

    float model_matrix[16];
    create_model_matrix(model_matrix, (float) rx, (float) ry, (float) 64, (float) 64);

    glUniformMatrix4fv(model, 1, GL_FALSE, model_matrix);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glBindVertexArray(renderer.quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
