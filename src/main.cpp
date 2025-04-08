#include "texture.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "light.hpp"
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

float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)WINDOW_WIDTH/2.0f;
float lastY = (float)WINDOW_HEIGHT/2.0f;
bool firstMouse = true;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
glm::vec3 lightColor(0.8f, 0.2f, 0.1f);

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
		printf("Failed to initialize GLAD\n");
        return -1;
    }
    // Openg GL Config
    glViewport(0, 0, WINDOW_WIDTH , WINDOW_HEIGHT);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);

    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* vendor = glGetString(GL_VENDOR);
    printf("Renderer: %s\n", renderer);
    printf("Vendor: %s\n", vendor);

    // Create Shaders
    Shader simple_rectangle = createShaderFromFile("shaders/vertex.glsl","shaders/fragment.glsl");
    Shader light_shader = createShaderFromFile("shaders/vertex.glsl","shaders/light_frag.glsl");
    // Create Textures
    Texture crate = createTextureFromFile("assets/textures/container2.png");
    Texture crate_specular = createTextureFromFile("assets/textures/container2_specular.png");
    Texture face = createTextureFromFile("assets/textures/awesomeface.png");

    // Meshs Data
    float vertices[] = {
        // position           // normal            // tex coords
    
        // Front face
        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 1.0f,
    
        // Back face
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
    
        // Left face
        -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
    
        // Right face
         0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
    
        // Bottom face
        -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f,
    
        // Top face
        -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
    };
    unsigned int indices[] = {
        0, 1, 2, 2, 3, 0,        // front
        4, 5, 6, 6, 7, 4,        // back
        8, 9,10,10,11, 8,        // left
       12,13,14,14,15,12,        // right
       16,17,18,18,19,16,        // bottom
       20,21,22,22,23,20         // top
    }; 
    
    // Build VAOs
    unsigned int VAO;
    glGenVertexArrays(1, &VAO); 
    glBindVertexArray(VAO);
    
    unsigned int VBO;
    glGenBuffers(1, &VBO); 
    glBindBuffer(GL_ARRAY_BUFFER, VBO);  
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Vertices
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Texture Coord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);  
    
    // Indices
    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); 
    
    glBindVertexArray(0);


    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    // we only need to bind to the VBO, the container's VBO's data already contains the data.
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // set the vertex attribute 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

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
        glm::vec3( 0.0f,  0.0f, -3.0f)
    };

    // Static Shaders Uniforms

    useShader(simple_rectangle);
        setInt(simple_rectangle, "material.diffuse", 0);
        setInt(simple_rectangle, "material.specular", 2);
        setInt(simple_rectangle, "face", 1);

        setFloat(simple_rectangle, "material.shininess", 32.0f);  
        
        Light dirLight = {
            .type = LIGHT_TYPE_DIRECTIONAL,
            .direction = glm::vec3(-0.2f, -1.0f, -0.3f),
            .ambient = glm::vec3(0.05f),
            .diffuse = glm::vec3(0.4f),
            .specular = glm::vec3(0.5f)
        };
        setLight("dirLight", dirLight, simple_rectangle);
        
        for (int i = 0; i < 4; i++) {
            Light point = {
                .type = LIGHT_TYPE_POINT,
                .position = pointLightPositions[i],
                .ambient = glm::vec3(0.05f),
                .diffuse = lightColor,
                .specular = glm::vec3(1.0f),
                .constant = 1.0f,
                .linear = 0.09f,
                .quadratic = 0.032f
            };
            std::string name = "pointLights[" + std::to_string(i) + "]";
            setLight(name.c_str(), point, simple_rectangle);
        }
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

        processInput(window);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, crate.ID);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, face.ID);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, crate_specular.ID);
        
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)screen_width / (float)screen_height, 0.1f, 100.0f);
        glm::mat4 view = GetViewMatrix(camera);

        useShader(simple_rectangle);
            // Transformations View/Projection  -------------------------------------
            setMat4(simple_rectangle, "projection",  glm::value_ptr(projection));
            setMat4(simple_rectangle, "view",  glm::value_ptr(view));
            //------------------------------------------------------------------------
            
            setVec3(simple_rectangle, "viewPos", glm::value_ptr(camera.Position));
            Light spot = {
                .type = LIGHT_TYPE_SPOT,
                .position = camera.Position,
                .direction = camera.Front,
                .ambient = glm::vec3(0.0f),
                .diffuse = glm::vec3(1.0f),
                .specular = glm::vec3(1.0f),
                .constant = 1.0f,
                .linear = 0.09f,
                .quadratic = 0.032f,
                .cutOff = glm::cos(glm::radians(12.5f)),
                .outerCutOff = glm::cos(glm::radians(15.0f))
            };
            setLight("spotLight", spot, simple_rectangle);

            glm::mat4 model         = glm::mat4(1.0f); 
            model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
            glBindVertexArray(VAO);
            for(unsigned int i = 0; i < 10; i++)
            {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, cubePositions[i]);
                float angle = 20.0f * i;
                model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
                model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
                setMat4(simple_rectangle, "model",  glm::value_ptr(model));
                glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            }



        // Point Light Source

        useShader(light_shader);
            setMat4(light_shader, "projection",  glm::value_ptr(projection));
            setMat4(light_shader, "view",  glm::value_ptr(view));
            glBindVertexArray(lightVAO);
            for (unsigned int i = 0; i < 4; i++)
            {
                model = glm::mat4(1.0f);
                model = glm::translate(model, pointLightPositions[i]);
                model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
                setMat4(light_shader, "model",  glm::value_ptr(model));
                glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

            }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &VBO);
    deleteShader(simple_rectangle);

    glfwTerminate();
    return 0;
}