#include "texture.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "light.hpp"
#include "mesh.hpp"
#include "model.hpp"
#include <GLFW/glfw3.h>
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

float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)WINDOW_WIDTH/2.0f;
float lastY = (float)WINDOW_HEIGHT/2.0f;
bool firstMouse = true;
bool sRGB = true;

// lighting
glm::vec3 lightColor(0.6f, 0.6f, 0.6f);

bool hdr = true;
bool hdrKeyPressed = false;
float exposure = 1.0f;

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    static bool lKeyPressedLastFrame = false;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        ProcessKeyboard(camera,FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        ProcessKeyboard(camera,BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        ProcessKeyboard(camera,LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        ProcessKeyboard(camera,RIGHT, deltaTime);
    
    bool lKeyCurrentlyPressed = glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS;
    if (lKeyCurrentlyPressed && !lKeyPressedLastFrame)
    {
        sRGB = !sRGB;
    }
    lKeyPressedLastFrame = lKeyCurrentlyPressed;

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !hdrKeyPressed)
    {
        hdr = !hdr;
        hdrKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
    {
        hdrKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        if (exposure > 0.0f)
            exposure -= 0.001f;
        else
            exposure = 0.0f;
    }
    else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        exposure += 0.001f;
    }
    
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 

    lastX = xpos;
    lastY = ypos;

    ProcessMouseMovement(camera, xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    ProcessMouseScroll(camera, static_cast<float>(yoffset));
}

void setupLightsForShader(const Shader& shader, const Light& dirLight, const Light spotLight, const glm::vec3& lightColor, const glm::vec3* pointLightPositions, int pointLightCount) {
    useShader(shader);
    setLight("dirLight", dirLight, shader);
    setLight("spotLight", spotLight, shader);
    for (int i = 0; i < pointLightCount; i++) {
        Light point = {
            .type = LIGHT_TYPE_POINT,
            .position = pointLightPositions[i],
            .ambient = glm::vec3(0.0f),
            .diffuse = lightColor,
            .specular = glm::vec3(1.0f),
            .constant = 0.0f,
            .linear = 0.0f,
            .quadratic = 1.0f
        };
        std::string name = "pointLights[" + std::to_string(i) + "]";
        setLight(name.c_str(), point, shader);
    }
    useShader({0});
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
    // Openg GL Config
    glViewport(0, 0, WINDOW_WIDTH , WINDOW_HEIGHT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

    GLuint defaultFramebuffer = 0;  // Default framebuffer ID is always 0
    // Bind the default framebuffer explicitly (default framebuffer is always bound, but let's be explicit)
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);

    // Check if binding succeeded
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "Framebuffer is not complete.\n");
        return -1;
    }
    
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
    }
    
  

    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* vendor = glGetString(GL_VENDOR);
    printf("Renderer: %s\n", renderer);
    printf("Vendor: %s\n", vendor);

    // configure MSAA framebuffer
    // --------------------------
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // create a multisampled color attachment texture
    unsigned int textureColorBufferMultiSampled;
    glGenTextures(1, &textureColorBufferMultiSampled);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB16F, WINDOW_WIDTH, WINDOW_HEIGHT, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled, 0);
    // create a (also multisampled) renderbuffer object for depth and stencil attachments
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, WINDOW_WIDTH, WINDOW_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        printf("ERROR::FRAMEBUFFER:: Framebuffer is not complete! (%s:%d)\n", __FILE__, __LINE__);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unsigned int intermediateFBO;
    glGenFramebuffers(1, &intermediateFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);

    Texture screen_texture;
    screen_texture.type = TEXTURE_DIFFUSE;
    glGenTextures(1, &screen_texture.ID);
    glBindTexture(GL_TEXTURE_2D, screen_texture.ID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screen_texture.ID, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        printf("ERROR::FRAMEBUFFER:: Framebuffer is not complete! (%s:%d)\n", __FILE__, __LINE__);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Create Shaders
    Shader model_shader = createShaderFromFile("shaders/vertex.glsl","shaders/fragment.glsl");
    Shader light_shader = createShaderFromFile("shaders/vertex.glsl","shaders/light_frag.glsl");
    Shader skybox_shader = createShaderFromFile("shaders/cubemap_vertex.glsl","shaders/cubemap_frag.glsl");
    Shader window_shader = createShaderFromFile("shaders/vertex.glsl","shaders/window.glsl");
    Shader screen_shader = createShaderFromFile("shaders/screen_vertex.glsl","shaders/screen_frag.glsl");
    // Create Textures
    Texture crate = createTextureFromFile("container2.png", "assets/textures",TEXTURE_DIFFUSE,true);
    Texture crate_specular = createTextureFromFile("container2_specular.png", "assets/textures", TEXTURE_SPECULAR, true);
    Texture grass[] = {createTextureFromFile("grass.png", "assets/textures",TEXTURE_DIFFUSE, true)};
    Texture wood_floor[] = {createTextureFromFile("wood.png", "assets/textures",TEXTURE_DIFFUSE, true), createSingleColorTexture(TEXTURE_SPECULAR, {150,150,150})};
    Texture window_red[] = {createTextureFromFile("blending_transparent_window.png", "assets/textures",TEXTURE_DIFFUSE, true)};
    Texture cubeTextures[] = {
        { crate},
        { crate_specular}
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
    useShader({0});
    

    Model* model_bag = ModelInit("assets/models/backpack/backpack.obj");

    // Meshs Data
    Vertex cubeVertices[] = {
        // position           // normal            // tex coords
    
        // Front face
        {{-1.0f, -1.0f,  1.0f},   {0.0f,  0.0f,  1.0f},   {0.0f, 0.0f}},
        {{ 1.0f, -1.0f,  1.0f},   {0.0f,  0.0f,  1.0f},   {1.0f, 0.0f}},
        {{ 1.0f,  1.0f,  1.0f},   {0.0f,  0.0f,  1.0f},   {1.0f, 1.0f}},
        {{-1.0f,  1.0f,  1.0f},   {0.0f,  0.0f,  1.0f},   {0.0f, 1.0f}},
    
        // Back face
        {{-1.0f, -1.0f, -1.0f},   {0.0f,  0.0f, -1.0f},   {1.0f, 0.0f}},
        {{ 1.0f, -1.0f, -1.0f},   {0.0f,  0.0f, -1.0f},   {0.0f, 0.0f}},
        {{ 1.0f,  1.0f, -1.0f},   {0.0f,  0.0f, -1.0f},   {0.0f, 1.0f}},
        {{-1.0f,  1.0f, -1.0f},   {0.0f,  0.0f, -1.0f},   {1.0f, 1.0f}},
    
        // Left face
        {{-1.0f, -1.0f, -1.0f},  {-1.0f,  0.0f,  0.0f},   {0.0f, 0.0f}},
        {{-1.0f, -1.0f,  1.0f},  {-1.0f,  0.0f,  0.0f},   {1.0f, 0.0f}},
        {{-1.0f,  1.0f,  1.0f},  {-1.0f,  0.0f,  0.0f},   {1.0f, 1.0f}},
        {{-1.0f,  1.0f, -1.0f},  {-1.0f,  0.0f,  0.0f},   {0.0f, 1.0f}},
    
        // Right face
         {{1.0f, -1.0f, -1.0f},   {1.0f,  0.0f,  0.0f},   {1.0f, 0.0f}},
         {{1.0f, -1.0f,  1.0f},   {1.0f,  0.0f,  0.0f},   {0.0f, 0.0f}},
         {{1.0f,  1.0f,  1.0f},   {1.0f,  0.0f,  0.0f},   {0.0f, 1.0f}},
         {{1.0f,  1.0f, -1.0f},   {1.0f,  0.0f,  0.0f},   {1.0f, 1.0f}},
    
        // Bottom face
        {{-1.0f, -1.0f, -1.0f},   {0.0f, -1.0f,  0.0f},   {0.0f, 1.0f}},
        {{ 1.0f, -1.0f, -1.0f},   {0.0f, -1.0f,  0.0f},   {1.0f, 1.0f}},
        {{ 1.0f, -1.0f,  1.0f},   {0.0f, -1.0f,  0.0f},   {1.0f, 0.0f}},
        {{-1.0f, -1.0f,  1.0f},   {0.0f, -1.0f,  0.0f},   {0.0f, 0.0f}},
    
        // Top face
        {{-1.0f,  1.0f, -1.0f},   {0.0f,  1.0f,  0.0f},   {0.0f, 0.0f}},
        {{ 1.0f,  1.0f, -1.0f},   {0.0f,  1.0f,  0.0f},   {1.0f, 0.0f}},
        {{ 1.0f,  1.0f,  1.0f},   {0.0f,  1.0f,  0.0f},   {1.0f, 1.0f}},
        {{-1.0f,  1.0f,  1.0f},   {0.0f,  1.0f,  0.0f},   {0.0f, 1.0f}},
    };
    
    unsigned int indices[] = {
        0, 1, 2, 2, 3, 0,        // front
        6, 5, 4, 4, 7, 6,        // back
        8, 9,10,10,11, 8,        // left
       14,13,12,12,15,14,        // right
       16,17,18,18,19,16,        // bottom
       22,21,20,20,23,22         // top
    }; 

    unsigned int indicesSkyBox[] = {
        2, 1, 0, 0, 3, 2,        // front
        4, 5, 6, 6, 7, 4,        // back
       10, 9, 8, 8,11,10,        // left
       12,13,14,14,15,12,        // right
       18,17,16,16,19,18,        // bottom
       20,21,22,22,23,20         // top
    };

    Vertex quadVertices[] = {
        // Position               // Normal              // Tex Coords
    
        {{-1.0f, -1.0f, 0.0f},     {0.0f, 0.0f, 1.0f},     {0.0f, 0.0f}},  // Bottom-left
        {{ 1.0f, -1.0f, 0.0f},     {0.0f, 0.0f, 1.0f},     {1.0f, 0.0f}},  // Bottom-right
        {{ 1.0f,  1.0f, 0.0f},     {0.0f, 0.0f, 1.0f},     {1.0f, 1.0f}},  // Top-right
        {{-1.0f,  1.0f, 0.0f},     {0.0f, 0.0f, 1.0f},     {0.0f, 1.0f}},  // Top-left
    };

    float quadVerticess[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerticess), &quadVerticess, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    useShader(screen_shader);
    setInt(screen_shader, "texture1", 0);
    useShader({0});
    
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
        glm::vec3( 0.7f,  0.2f,  2.0f),
        glm::vec3( 2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3( 0.0f,  0.0f, -3.0f),
    };

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
        
        int screen_width, screen_height;
        glfwGetFramebufferSize(window, &screen_width, &screen_height); // TODO: maybe we can do this only when changes happen on the callback

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;


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

        processInput(window);

        float near_plane = 1.0f, far_plane = 7.5f;
        glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);  
        
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);  
        glCullFace(GL_BACK);  
        glFrontFace(GL_CCW);
        
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)screen_width / (float)screen_height, 0.1f, 100.0f);
        glm::mat4 view = GetViewMatrix(camera);

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

        const glm::vec3 OrinalVec = glm::vec3( 0.7f,  0.2f,  2.0f);
        const glm::mat4 rot = glm::rotate(glm::mat4(1.0f), currentFrame, glm::vec3(0.0f,1.0f,0.0f));
        pointLightPositions[0] = rot * glm::vec4(OrinalVec, 1.0f);
        setupLightsForShader(model_shader, dirLight, spot, lightColor, pointLightPositions, ARRAY_SIZE(pointLightPositions));

        useShader(model_shader);
        {

            activateMesh(&cubeMesh, &model_shader);
            // Transformations View/Projection  -------------------------------------
            setMat4(model_shader, "projection",  glm::value_ptr(projection));
            setMat4(model_shader, "view",  glm::value_ptr(view));
            //------------------------------------------------------------------------
            
            setVec3(model_shader, "viewPos", glm::value_ptr(camera.Position));


            for(unsigned int i = 0; i < 10; i++)
            {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, cubePositions[i]);
                float angle = 20.0f * i;
                model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
                // model = glm::rotate(model, currentFrame, glm::vec3(0.0f, 0.0f, 1.0f));
                model = glm::scale(model, glm::vec3(0.5f));
                setMat4(model_shader, "model",  glm::value_ptr(model));
                drawMesh(&cubeMesh, &model_shader);
            }
            
            {
                activateMesh(&quadGrass, &model_shader);
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(0,-3.90 + 1.0,0));
                setMat4(model_shader, "model",  glm::value_ptr(model));
                drawMesh(&quadGrass, &model_shader);
            }
            {
                activateMesh(&quadFloor, &model_shader);
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(0,-4,0));
                model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                // model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, -1.0f));
                model = glm::scale(model, glm::vec3(30.0f, 30.0f, 0.1f));
                setMat4(model_shader, "model",  glm::value_ptr(model));
                drawMesh(&quadFloor, &model_shader);
            }
        }
            


        // Point Light Source

        useShader(light_shader);
            setMat4(light_shader, "projection",  glm::value_ptr(projection));
            setMat4(light_shader, "view",  glm::value_ptr(view));
            activateMesh(&cubeMesh, &light_shader);
            for (unsigned int i = 0; i < ARRAY_SIZE(pointLightPositions); i++)
            {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, pointLightPositions[i]);
                model = glm::scale(model, glm::vec3(0.1f)); // Make it a smaller cube
                setMat4(light_shader, "model",  glm::value_ptr(model));
                drawMesh(&cubeMesh, &light_shader);

            }

        {
            useShader(model_shader);
                setMat4(model_shader, "projection",  glm::value_ptr(projection));
                setMat4(model_shader, "view",  glm::value_ptr(view));
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3( 2.0f,  2.0f,  3.0f));
                model = glm::scale(model, glm::vec3(1.0f));
                setMat4(model_shader, "model",  glm::value_ptr(model));
                setLight("spotLight", spot, model_shader);
    
                DrawModel(model_bag,&model_shader);
        }
        
        {
            glDepthFunc(GL_LEQUAL);
            useShader(skybox_shader);
            const glm::mat4 skybox_view = glm::mat4(glm::mat3(view));
            setMat4(skybox_shader, "projection",  glm::value_ptr(projection));
            setMat4(skybox_shader, "view",  glm::value_ptr(skybox_view));
            drawMesh(&skyboxMesh, &skybox_shader);
            glDepthFunc(GL_LESS);
        }
        
        {
            useShader(window_shader);
            glDepthMask(GL_FALSE);
                setMat4(window_shader, "projection",  glm::value_ptr(projection));
                setMat4(window_shader, "view",  glm::value_ptr(view));
                activateMesh(&quadWindow, &window_shader);
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(-1.0,2.5,-4.0));
                model = glm::rotate(model, glm::radians(75.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                setMat4(window_shader, "model",  glm::value_ptr(model));
                drawMesh(&quadWindow, &window_shader);
            glDepthMask(GL_TRUE);
        }

        // 2. now blit multisampled buffer(s) to normal colorbuffer of intermediate FBO. Image is stored in screenTexture
        glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO);
        glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        // // now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
        glClear(GL_COLOR_BUFFER_BIT);
        if (sRGB){glEnable(GL_FRAMEBUFFER_SRGB);}
        else{glDisable(GL_FRAMEBUFFER_SRGB);}
        // // clear all relevant buffers

        useShader(screen_shader);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, screen_texture.ID);	// use the color attachment texture as the texture of the quad plane
        setInt(screen_shader, "hdr", hdr);
        setFloat(screen_shader, "exposure", exposure);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisable(GL_FRAMEBUFFER_SRGB);

        // useShader(screen_shader);
        // activateMesh(&quadScreen, &screen_shader);
        // drawMesh(&quadScreen, &screen_shader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    deleteShader(model_shader);

    glfwTerminate();
    return 0;
}