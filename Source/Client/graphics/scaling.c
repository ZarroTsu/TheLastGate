//
// Created by james on 11/28/2025.
//

#include "scaling.h"
#include <stdio.h>

#include "sdl.h"
#include "shaders/shaders.h"
#include "../main.h"
#include "glad/glad.h"
#include "shaders/scaling_shader_files.h"

static GLuint fbo = 0;
static GLuint fboTexture = 0;
static GLuint fboRBO = 0;
static GLuint screenVAO = 0;
static GLuint screenVBO = 0;
static GLuint screenShader = 0;

static int BASE_W;
static int BASE_H;

// ------------------------------------------------------------
// Initialize FBO + quad
// ------------------------------------------------------------
void init_fbo_scaling(int baseWidth, int baseHeight) {
    BASE_W = baseWidth;
    BASE_H = baseHeight;

    // 1) Create FBO texture
    create_gl_texture(BASE_W, BASE_H, &fboTexture, NULL);

    // 2) Renderbuffer for depth/stencil
    glGenRenderbuffers(1, &fboRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, fboRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, BASE_W, BASE_H);

    // 3) FBO attach
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, fboTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, fboRBO);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 4) Screen quad setup
    float vertices[] = {
        // positions   // UV
        -1.f, -1.f, 0.f, 0.f,
        1.f, -1.f, 1.f, 0.f,
        1.f, 1.f, 1.f, 1.f,
        -1.f, 1.f, 0.f, 1.f
    };

    unsigned int indices[] = {0, 1, 2, 2, 3, 0};

    glGenVertexArrays(1, &screenVAO);
    glBindVertexArray(screenVAO);

    glGenBuffers(1, &screenVBO);
    glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void *) (2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // 5) Screen shader
    screenShader = load_shader_program(scaling_vertex, scaling_fragment);
}

// ------------------------------------------------------------
// Window resize? Nothing to do — FBO stays fixed.
// Only glViewport changes when drawing the final quad.
// ------------------------------------------------------------
void resize_fbo_scaling(int windowWidth, int windowHeight) {
    glViewport(0, 0, windowWidth, windowHeight);
}

// ------------------------------------------------------------
// Start rendering to fixed-resolution FBO
// ------------------------------------------------------------
void sdl_start_scaling(void) {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, BASE_W, BASE_H);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// ------------------------------------------------------------
// Stop rendering → draw FBO to the window
// ------------------------------------------------------------
void sdl_stop_scaling(void) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Set viewport to actual window size
    glViewport(0, 0, app_state.window_size[0], app_state.window_size[1]);

    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);

    glUseProgram(screenShader);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fboTexture);
    glUniform1i(glGetUniformLocation(screenShader, "uTex"), 0);

    glBindVertexArray(screenVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}
