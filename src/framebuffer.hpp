#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H
#include <glad/glad.h>

struct FrameBuffer {
    unsigned int fbo;
    unsigned int rbo;
    unsigned int colorTexture;
    unsigned int brightTexture;
};

void CheckFramebufferStatus(int line)
{
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    printf("ERROR::FRAMEBUFFER:: Framebuffer is not complete! (%s:%d)\n", __FILE__, line);
}

void GenerateFBO(FrameBuffer* fb)
{
    glGenFramebuffers(1, &fb->fbo);
}

Texture prepare16fScreenTextureBoundToFBO(FrameBuffer* fb, Shader* screen_shader, const char * uniform_text_name, unsigned int* texture_loc, int width, unsigned int height)
{   
    glBindFramebuffer(GL_FRAMEBUFFER, fb->fbo);

    Texture screen_texture;
    screen_texture.type = TEXTURE_DIFFUSE;
    
    glGenTextures(1, &screen_texture.ID);
    glBindTexture(GL_TEXTURE_2D, screen_texture.ID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screen_texture.ID, 0);

    CheckFramebufferStatus(__LINE__);

    *texture_loc = attachTexturetoLoc(screen_shader, uniform_text_name);
    printf("screen_shader.num_of_text_locs %u\n",screen_shader->num_of_text_locs);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return screen_texture;

}

Texture prepareDepthTextureBoundToFBO(FrameBuffer* depthMapFBO, Shader* scene_shader,  const char* uniform_text_name, unsigned int* texture_loc, unsigned int shadow_width, unsigned int shadow_height)
{

    // create depth texture
    Texture depthTexture;
    depthTexture.type = TEXTURE_DIFFUSE;
    glGenTextures(1, &depthTexture.ID);
    glBindTexture(GL_TEXTURE_2D, depthTexture.ID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadow_width, shadow_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor); 

    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO->fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture.ID, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    CheckFramebufferStatus(__LINE__);
    
    // *texture_loc = attachTexturetoLoc(scene_shader, uniform_text_name);
    printf("scene(model) shader %u\n", scene_shader->num_of_text_locs);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    return depthTexture;

}
void prepare16fMSAAFrameBuffer(FrameBuffer* fb, unsigned int width, unsigned int height)
{
    GenerateFBO(fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb->fbo);

    glGenTextures(1, &fb->colorTexture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fb->colorTexture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA16F, width, height, GL_TRUE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, fb->colorTexture, 0);
    
    glGenRenderbuffers(1, &fb->rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, fb->rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fb->rbo);
    CheckFramebufferStatus(__LINE__);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void prepare16fMSAABloomFrameBuffer(FrameBuffer* fb, unsigned int width, unsigned int height)
{
    GenerateFBO(fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb->fbo);

    // First color attachment (multisampled)
    glGenTextures(1, &fb->colorTexture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fb->colorTexture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA16F, width, height, GL_TRUE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, fb->colorTexture, 0);
    
    // Depth/stencil attachment (multisampled)
    glGenRenderbuffers(1, &fb->rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, fb->rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fb->rbo);
    
    // Second color attachment (also multisampled)
    glGenTextures(1, &fb->brightTexture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fb->brightTexture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA16F, width, height, GL_TRUE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, fb->brightTexture, 0);
    
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
        printf("Framebuffer incomplete: %d\n", status);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void prepare16fFrameBuffer(FrameBuffer* fb, unsigned int width, unsigned int height)
{
    GenerateFBO(fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb->fbo);

    // Create color texture
    glGenTextures(1, &fb->colorTexture);
    glBindTexture(GL_TEXTURE_2D, fb->colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb->colorTexture, 0);

    // Create depth (or depth-stencil) renderbuffer
    glGenRenderbuffers(1, &fb->rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, fb->rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fb->rbo);

    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("Framebuffer is not complete!\n");
    }

    // Cleanup
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

Texture prepareCubeDepthTextureBoundToFBO(FrameBuffer* depthMapFBO, Shader* scene_shader,  const char* uniform_text_name, unsigned int* texture_loc, unsigned int shadow_width, unsigned int shadow_height)
{
    Texture depthCubemap;
    glGenTextures(1, &depthCubemap.ID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap.ID);
    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, shadow_width, shadow_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    // attach depth texture as FBO's depth buffer

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO->fbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap.ID, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return depthCubemap;
}

void prepareBlurringFrameBuffers(FrameBuffer* blurring_FBO_1, FrameBuffer* blurring_FBO_2, unsigned int width, unsigned int height)
{
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(1, &blurring_FBO_1->fbo);
    glGenFramebuffers(1, &blurring_FBO_2->fbo);
    glGenTextures(2, pingpongColorbuffers);
    
    glBindFramebuffer(GL_FRAMEBUFFER, blurring_FBO_1->fbo);
    glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[0], 0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, blurring_FBO_2->fbo);
    glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[1], 0);
    
    blurring_FBO_1->colorTexture = pingpongColorbuffers[0];
    blurring_FBO_2->colorTexture = pingpongColorbuffers[1];

    CheckFramebufferStatus(__LINE__);
}


#endif
