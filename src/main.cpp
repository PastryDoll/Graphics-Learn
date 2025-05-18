#include "texture.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "light.hpp"
#include "mesh.hpp"
#include "model.hpp"
#include "framebuffer.hpp"
#include "scene.hpp"
#include "render.hpp"
#include "input.hpp"
#include "dev_console.hpp"
#include <stdio.h>
#include <math.h>
#include "../thirdparty/glm/gtc/type_ptr.hpp"

// TODO: Find better way to force NVIDIA GPU
// Substack: "you should use WGL_NV_gpu_affinity"
#ifdef _WIN32 
extern "C" {
    _declspec(dllexport) int NvOptimusEnablement = 1;
    _declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900
#define WINDOW_TITLE "Hello World"
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
unsigned int planeVAO;
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

#define CAMERA_BINDING_POINT 0

// lighting
glm::vec3 lightColor(10.8f, 10.8f, 10.8f);

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main(void)
{
    GLFWwindow* window;

    if (!glfwInit())
        return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (!window)
    {   
		printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
		printf("Failed to initialize GLAD\n");
        return -1;
    }

    // Setup Input Manager
    InputManager inputManager;
    initInputManager(&inputManager);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    // Openg GL Config
    glViewport(0, 0, WINDOW_WIDTH , WINDOW_HEIGHT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
    CheckFramebufferStatus(__LINE__);
    
    // Query using OpenGL
    {
        GLint glRedBits, glGreenBits, glBlueBits, glAlphaBits;
        GLint glDepthBits, glStencilBits, glSamples;
        // For color buffer (default framebuffer)
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK_LEFT, 
                                                GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE, &glRedBits);
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK_LEFT, 
                                                GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE, &glGreenBits);
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK_LEFT, 
                                                GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE, &glBlueBits);
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK_LEFT, 
                                                GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE, &glAlphaBits);
        
        // For depth and stencil
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH, 
                                                GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &glDepthBits);
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_STENCIL, 
                                                GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &glStencilBits);
        // For multisampling
        glGetIntegerv(GL_SAMPLES, &glSamples);
        // Handle any OpenGL errors from the query
        GLenum error = glGetError();
        if (error == GL_NO_ERROR) {
            printf("\nDefault Framebuffer Format (OpenGL):\n");
            printf("---------------------------\n");
            printf("Red bits:     %d\n", glRedBits);
            printf("Green bits:   %d\n", glGreenBits);
            printf("Blue bits:    %d\n", glBlueBits);
            printf("Alpha bits:   %d\n", glAlphaBits);
            printf("Depth bits:   %d\n", glDepthBits);
            printf("Stencil bits: %d\n", glStencilBits);
            printf("MSAA samples: %d\n", glSamples);
        } else {
            printf("\nUnable to query some OpenGL framebuffer parameters. Error code: 0x%x\n", error);
        }
        
        const GLubyte* renderer = glGetString(GL_RENDERER);
        const GLubyte* vendor = glGetString(GL_VENDOR);
        printf("Renderer: %s\n", renderer);
        printf("Vendor: %s\n", vendor);
    }

    // Create Shaders
    Shader model_shader = createShaderFromFile("shaders/vertex.glsl","shaders/fragment.glsl", nullptr);
    Shader light_shader = createShaderFromFile("shaders/vertex.glsl","shaders/light_frag.glsl", nullptr);
    Shader skybox_shader = createShaderFromFile("shaders/cubemap_vertex.glsl","shaders/cubemap_frag.glsl", nullptr);
    Shader window_shader = createShaderFromFile("shaders/vertex.glsl","shaders/window.glsl", nullptr);
    Shader screen_shader = createShaderFromFile("shaders/screen_vertex.glsl","shaders/screen_frag.glsl", nullptr);
    Shader depth_shader = createShaderFromFile("shaders/depthmap_vertex.glsl","shaders/depthmap_frag.glsl", nullptr);
    Shader depthcube_shader = createShaderFromFile("shaders/depthcube_vertex.glsl","shaders/depthcube_frag.glsl", "shaders/depthcube_geo.glsl");
    Shader depth_debug_shader = createShaderFromFile("shaders/debug_depth_vertex.glsl","shaders/debug_depth_frag.glsl", nullptr);
    Shader blur_shader = createShaderFromFile("shaders/blur_vertex.glsl","shaders/blur_frag.glsl", nullptr);
    

    // // configure MSAA framebuffer
    // // --------------------------

    FrameBuffer msaa_hdr_frame_buffer = {0};
    // prepare16fMSAAFrameBuffer(&msaa_hdr_frame_buffer, WINDOW_WIDTH, WINDOW_HEIGHT);
    prepare16fMSAABloomFrameBuffer(&msaa_hdr_frame_buffer, WINDOW_WIDTH, WINDOW_HEIGHT);
    
    FrameBuffer pingpongFBO[2] = {0, 0};
    prepareBlurringFrameBuffers(&pingpongFBO[0], &pingpongFBO[1], WINDOW_WIDTH, WINDOW_HEIGHT);

    FrameBuffer intermediaryBlurFBO = {0};
    GenerateFBO(&intermediaryBlurFBO);
    
    unsigned int blur_texture_loc;
    Texture blur_texture = prepare16fScreenTextureBoundToFBO(&intermediaryBlurFBO, &blur_shader, "image", &blur_texture_loc, WINDOW_WIDTH, WINDOW_HEIGHT);

    FrameBuffer intermediaryFBO = {0};
    GenerateFBO(&intermediaryFBO);

    unsigned int screen_shader_texture_loc;
    Texture screen_texture = prepare16fScreenTextureBoundToFBO(&intermediaryFBO, &screen_shader, "texture1", &screen_shader_texture_loc, WINDOW_WIDTH, WINDOW_HEIGHT);

    // // configure depth map FBO
    // // -----------------------

    unsigned int SHADOW_WIDTH = 1024; 
    unsigned int SHADOW_HEIGHT = 1024;

    FrameBuffer depthMapFBO = {0};
    GenerateFBO(&depthMapFBO);
    unsigned int shadowMap_texture_loc;
    Texture depthMaptexture = prepareDepthTextureBoundToFBO(&depthMapFBO, &model_shader, "shadowMap", &shadowMap_texture_loc, SHADOW_WIDTH, SHADOW_HEIGHT);

    FrameBuffer depthCubeMapFBO = {0};
    GenerateFBO(&depthCubeMapFBO);
    Texture depthCubeTexture = prepareCubeDepthTextureBoundToFBO(&depthCubeMapFBO, &model_shader, "shadowMap", &shadowMap_texture_loc, SHADOW_WIDTH, SHADOW_HEIGHT);
    
    // Create Textures
    Texture crate = createTextureFromFile("container2.png", "assets/textures",TEXTURE_DIFFUSE,true);
    Texture crate_specular = createTextureFromFile("container2_specular.png", "assets/textures", TEXTURE_SPECULAR, true);
    Texture grass[] = {createTextureFromFile("grass.png", "assets/textures",TEXTURE_DIFFUSE, true)};
    Texture wood_floor[] = {createTextureFromFile("wood.png", "assets/textures",TEXTURE_DIFFUSE, true), createSingleColorTexture(TEXTURE_SPECULAR, {200,200,200}),FLAT_NORMAL_TEXTURE};
    Texture window_red[] = {createTextureFromFile("blending_transparent_window.png", "assets/textures",TEXTURE_DIFFUSE, true)};
    Texture brick_wall[] = {
        createTextureFromFile("brickwall.jpg", "assets/textures",TEXTURE_DIFFUSE, true),
        createTextureFromFile("brickwall_normal.jpg", "assets/textures",TEXTURE_NORMAL, true),
        createSingleColorTexture(TEXTURE_SPECULAR, {20,20,20}),
        
    };
    Texture cubeTextures[] = {
        {crate},
        {crate_specular},
        {FLAT_NORMAL_TEXTURE}
    };

    // Declare an array of 6 file paths for the cubemap textures
    const char* faces[6] = {
        "assets/skyboxes/skybox/right.jpg",
        "assets/skyboxes/skybox/left.jpg",
        "assets/skyboxes/skybox/top.jpg",
        "assets/skyboxes/skybox/bottom.jpg",
        "assets/skyboxes/skybox/front.jpg",
        "assets/skyboxes/skybox/back.jpg"
    };

    unsigned int cubemapTexture = loadCubemap(faces);
    useShader(skybox_shader);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        setInt(skybox_shader, "skybox", 0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    useShader({0});

    Model* model_bag = ModelInit("assets/models/backpack/backpack.obj");
    
    unsigned int quadIndices[] = {
        0, 1, 2,
        2, 3, 0
    };
    
    Mesh cubeMesh( 
        cubeVertices,
        ARRAY_SIZE(cubeVertices),
        indices,
        ARRAY_SIZE(indices),
        cubeTextures,
        ARRAY_SIZE(cubeTextures)
    );

    Mesh skyboxMesh( 
        cubeVertices,
        ARRAY_SIZE(cubeVertices),
        indicesSkyBox,
        ARRAY_SIZE(indicesSkyBox),
        NULL,
        0
    );

    Mesh quadGrass(
        quadVertices,
        ARRAY_SIZE(quadVertices),
        quadIndices,
        ARRAY_SIZE(quadIndices),
        grass,
        ARRAY_SIZE(grass)
    );

    Mesh quadWindow(
        quadVertices,
        ARRAY_SIZE(quadVertices),
        quadIndices,
        ARRAY_SIZE(quadIndices),
        window_red,
        ARRAY_SIZE(window_red)
    );
    Mesh quadFloor(
        quadVertices,
        ARRAY_SIZE(quadVertices),
        quadIndices,
        ARRAY_SIZE(quadIndices),
        wood_floor,
        ARRAY_SIZE(wood_floor)
    );
    Mesh quadBrickWall(
        quadVertices,
        ARRAY_SIZE(quadVertices),
        quadIndices,
        ARRAY_SIZE(quadIndices),
        brick_wall,
        ARRAY_SIZE(brick_wall)
    );
    Mesh quadScreen(
        quadVertices,
        ARRAY_SIZE(quadVertices),
        quadIndices,
        ARRAY_SIZE(quadIndices),
        &screen_texture,
        1
    );

    // Transformations
    glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f), 
        glm::vec3( 2.0f,  5.0f, -15.0f), 
        glm::vec3(-1.5f, -2.2f, -2.5f),  
        glm::vec3(-3.8f, -2.0f, -12.3f),  
        glm::vec3( 2.4f, -0.4f, -3.5f),  
        glm::vec3(-1.7f,  3.0f, -7.5f),  
        glm::vec3( 1.3f, -2.0f, -2.5f),  
        glm::vec3( 1.5f,  2.0f, -2.5f), 
        glm::vec3( 1.5f,  0.2f, -1.5f), 
        glm::vec3(-1.3f,  1.0f, -1.5f)  
    };

    glm::vec3 pointLightPositions[] = {
        glm::vec3( -1.3f,  2.0f,  -3.0f),
        // glm::vec3( 2.3f, -3.3f, -4.0f),
        // glm::vec3(-4.0f,  2.0f, -12.0f),
        // glm::vec3( 0.0f,  0.0f, -3.0f),
        // lightPos,
    };

    unsigned int uboMatrices;
    glGenBuffers(1, &uboMatrices);
  
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_DYNAMIC_DRAW );
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
  
    glBindBufferRange(GL_UNIFORM_BUFFER, CAMERA_BINDING_POINT, uboMatrices, 0, 2 * sizeof(glm::mat4));

    // Static Shaders Uniforms
    Light dirLight = {
        .type = LIGHT_TYPE_DIRECTIONAL,
        .direction = glm::vec3(-0.2f, -1.0f, -0.3f),
        .ambient = glm::vec3(0.01f),
        .diffuse = glm::vec3(0.5f),
        .specular = glm::vec3(0.0f)
    };
    
    useShader(light_shader);
    setVec3(light_shader, "lightColor", glm::value_ptr(lightColor));
    useShader({0});

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        updateInputManager(window, &inputManager);

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window, &camera, &renderState, inputManager, deltaTime);

        if (renderState.devMode)
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            imgui_console(&renderState);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(1.0f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        

        int screen_width, screen_height;
        glfwGetFramebufferSize(window, &screen_width, &screen_height); // TODO: maybe we can do this only when changes happen on the callback

        static int frameCount = 0;
        static float fpsTimer = 0.0f;

        frameCount++;
        fpsTimer += deltaTime;

        if (fpsTimer >= 1.0f) { // Update every second
            float fps = frameCount / fpsTimer;
            printf("FPS: %.2f\n", fps);
            frameCount = 0;
            fpsTimer = 0.0f;
        }


        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)screen_width / (float)screen_height, 0.1f, 100.0f);
        glm::mat4 view = GetViewMatrix(camera);
        glm::mat4 matrices[2] = { projection, view };
        glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(matrices), glm::value_ptr(matrices[0]));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);  

        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        float near_plane = 0.1f, far_plane = 30.0f;
        lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
        lightView = glm::lookAt(pointLightPositions[0], glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;

        glm::mat4 shadowPerpectiveProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
        glm::mat4 shadowTransforms[6];

        shadowTransforms[0] = shadowPerpectiveProj * glm::lookAt(pointLightPositions[0], pointLightPositions[0] + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f));
        shadowTransforms[1] = shadowPerpectiveProj * glm::lookAt(pointLightPositions[0], pointLightPositions[0] + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f));
        shadowTransforms[2] = shadowPerpectiveProj * glm::lookAt(pointLightPositions[0], pointLightPositions[0] + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f));
        shadowTransforms[3] = shadowPerpectiveProj * glm::lookAt(pointLightPositions[0], pointLightPositions[0] + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f));
        shadowTransforms[4] = shadowPerpectiveProj * glm::lookAt(pointLightPositions[0], pointLightPositions[0] + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f));
        shadowTransforms[5] = shadowPerpectiveProj * glm::lookAt(pointLightPositions[0], pointLightPositions[0] + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f));


        Light spot = {
            .type = LIGHT_TYPE_SPOT,
            .position = camera.Position,
            .direction = camera.Front,
            .ambient = glm::vec3(0.0f),
            .diffuse = glm::vec3(0.0f),
            .specular = glm::vec3(0.0f),
            .constant = 0.0f,
            .linear = 0.0f,
            .quadratic = 1.0f,
            .cutOff = glm::cos(glm::radians(12.5f)),
            .outerCutOff = glm::cos(glm::radians(15.0f))
        };

        // static const glm::vec3 OrinalVec = pointLightPositions[0];
        // const glm::mat4 rot = glm::rotate(glm::mat4(1.0f), currentFrame, glm::vec3(0.0f,1.0f,0.0f));
        // pointLightPositions[0] = rot * glm::vec4(OrinalVec, 1.0f);
        
        // Rendedr Omni shadow ////////////////////////////////////////////////////
        useShader(depthcube_shader);
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthCubeMapFBO.fbo);
            glEnable(GL_DEPTH_TEST);
            glClear(GL_DEPTH_BUFFER_BIT);

            for (unsigned int i = 0; i < 6; ++i)
            {
                setMat4(depthcube_shader, ("shadowMatrices[" + std::to_string(i) + "]").c_str(), glm::value_ptr(shadowTransforms[i]));
            }
            setFloat(depthcube_shader, "far_plane", far_plane);
            setVec3(depthcube_shader, "lightPos", glm::value_ptr(pointLightPositions[0]));
            renderScene(currentFrame, model_bag, depthcube_shader, &cubeMesh, &quadGrass, &quadFloor, &quadBrickWall, cubePositions, nullptr, nullptr, SHADOW_PASS);

        // Render Scene /////////////////////////////////////////////////////////
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, msaa_hdr_frame_buffer.fbo);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);  
        glCullFace(GL_BACK);  
        glFrontFace(GL_CCW);
        
        useShader(model_shader);
        setupLightsForShader(model_shader, dirLight, spot, lightColor, pointLightPositions, ARRAY_SIZE(pointLightPositions));
        setVec3(model_shader, "viewPos", glm::value_ptr(camera.Position));
        setVec3(model_shader, "lightPos", glm::value_ptr(pointLightPositions[0]));
        setInt(model_shader, "shadows", renderState.useShadows); // enable/disable shadows by pressing 'SPACE'
        setFloat(model_shader, "far_plane", far_plane);
        setBool(model_shader, "useNormalMap", false);

        renderScene(currentFrame, model_bag, model_shader, &cubeMesh, &quadGrass, &quadFloor, &quadBrickWall ,cubePositions, nullptr, &depthCubeTexture, DRAW_PASS);

        // Point Light Source
        
        {
            useShader(light_shader);
                for (unsigned int i = 0; i < ARRAY_SIZE(pointLightPositions); i++)
                {
                    glm::mat4 model = glm::mat4(1.0f);
                    model = glm::translate(model, pointLightPositions[i]);
                    model = glm::scale(model, glm::vec3(0.1f)); // Make it a smaller cube
                    setMat4(light_shader, "model",  glm::value_ptr(model));
                    drawMesh(&cubeMesh, &light_shader,nullptr, nullptr);
    
                }
        }
        
        {
            useShader(skybox_shader);
            glDepthFunc(GL_LEQUAL);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
            drawMesh(&skyboxMesh, &skybox_shader, nullptr, nullptr);
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            glDepthFunc(GL_LESS);
        }
        
        // {
        //     useShader(window_shader);
        //     glDepthMask(GL_FALSE);
        //     glm::mat4 model = glm::mat4(1.0f);
        //     model = glm::translate(model, glm::vec3(-1.0,2.5,-4.0));
        //     model = glm::rotate(model, glm::radians(75.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        //     setMat4(window_shader, "model",  glm::value_ptr(model));
        //     drawMesh(&quadWindow, &window_shader, nullptr, nullptr);
        //     glDepthMask(GL_TRUE);
        // }

        // 2. blur bright fragments with two-pass Gaussian Blur 
        // --------------------------------------------------

        glBindFramebuffer(GL_READ_FRAMEBUFFER, msaa_hdr_frame_buffer.fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediaryBlurFBO.fbo);
        glReadBuffer(GL_COLOR_ATTACHMENT1);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        bool horizontal = true, first_iteration = true;
        unsigned int amount = 10;
        useShader(blur_shader);
        glActiveTexture(GL_TEXTURE0 + blur_texture_loc);
        for (unsigned int i = 0; i < amount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal].fbo);
            setInt(blur_shader, "horizontal", horizontal);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? blur_texture.ID : pingpongFBO[!horizontal].colorTexture);  // bind texture of other framebuffer (or scene if first iteration)
            renderQuad();
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }
        glActiveTexture(GL_TEXTURE0);


        // 2. now blit multisampled buffer(s) to normal colorbuffer of intermediate FBO. Image is stored in screenTexture
        glBindFramebuffer(GL_READ_FRAMEBUFFER,  msaa_hdr_frame_buffer.fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediaryFBO.fbo);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        // // // now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
        glClearColor(0.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
        glClear(GL_COLOR_BUFFER_BIT);
        if (renderState.sRGB){glEnable(GL_FRAMEBUFFER_SRGB);}
        else{glDisable(GL_FRAMEBUFFER_SRGB);}

        useShader(screen_shader);
        setInt(screen_shader, "bloomBlur", screen_shader_texture_loc + 1 );
        glActiveTexture(GL_TEXTURE0 + screen_shader_texture_loc);
        glBindTexture(GL_TEXTURE_2D, screen_texture.ID);	// use the color attachment texture as the texture of the quad plane
        glActiveTexture(GL_TEXTURE0 + screen_shader_texture_loc + 1 );
        glBindTexture(GL_TEXTURE_2D, pingpongFBO[!horizontal].colorTexture);	// use the color attachment texture as the texture of the quad plane
        setInt(screen_shader, "hdr", renderState.hdr);
        setInt(screen_shader, "bloom", renderState.bloom);
        setFloat(screen_shader, "exposure", renderState.exposure);
        
        renderQuad();
        glDisable(GL_FRAMEBUFFER_SRGB);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,0);
        useShader({0});

        if (renderState.devMode)
        {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
        glfwSwapBuffers(window);
    }
    
    //deleteShader(model_shader);

    glfwTerminate();
    return 0;
}