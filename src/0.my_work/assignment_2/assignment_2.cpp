#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void generateGalaxy(int numStars, int arms = 3, float radius = 10.0f);
void generateSphere(float radius, int sectorCount, int stackCount);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

// global
std::vector<float> galaxyVertices;
std::vector<float> sphereVertices;
std::vector<unsigned int> sphereIndices;

int main()
{
  // glfw: initialize and configure
  // ------------------------------
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // glfw window creation
  // --------------------
  GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
  if (window == NULL)
  {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

  // tell GLFW to capture our mouse
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // glad: load all OpenGL function pointers
  // ---------------------------------------
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  // configure global opengl state
  // -----------------------------
  glEnable(GL_DEPTH_TEST);

  // build and compile our shader zprogram
  // ------------------------------------
  Shader ourShader("assignment_2.vs", "assignment_2.fs");

  generateGalaxy(2000);
  generateSphere(0.08f, 12, 8);

  unsigned int VBO, VAO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  glDrawArrays(GL_POINTS, 0, galaxyVertices.size() / 3);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, galaxyVertices.size() * sizeof(float), galaxyVertices.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  unsigned int sphereVAO, sphereVBO, sphereEBO;
  glGenVertexArrays(1, &sphereVAO);
  glGenBuffers(1, &sphereVBO);
  glGenBuffers(1, &sphereEBO);

  glBindVertexArray(sphereVAO);
  glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
  glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), sphereVertices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), sphereIndices.data(), GL_STATIC_DRAW);

  unsigned int instanceVBO;
  glGenBuffers(1, &instanceVBO);
  glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
  glBufferData(GL_ARRAY_BUFFER, galaxyVertices.size() * sizeof(float), galaxyVertices.data(), GL_STATIC_DRAW);

  // position
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribDivisor(1, 1);

  // color
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(2);
  glVertexAttribDivisor(2, 1);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  // load image, create texture and generate mipmaps
  int width, height, nrChannels;
  glEnable(GL_PROGRAM_POINT_SIZE);

  // render loop
  // -----------
  while (!glfwWindowShouldClose(window))
  {
    // per-frame time logic
    // --------------------
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // input
    // -----
    processInput(window);

    // render
    // ------
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // activate shader
    ourShader.use();

    // pass projection matrix to shader (note that in this case it could change every frame)
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    ourShader.setMat4("projection", projection);

    // camera/view transformation
    glm::mat4 view = camera.GetViewMatrix();
    ourShader.setMat4("view", view);

    static float angle = 0.0f;
    angle += 0.6f * deltaTime;

    glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    ourShader.setMat4("model", model);

    // render boxes
    // glBindVertexArray(VAO);
    // glDrawArrays(GL_POINTS, 0, galaxyVertices.size() / 3);
    glBindVertexArray(sphereVAO);
    glDrawElementsInstanced(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0, galaxyVertices.size() / 6);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    // -------------------------------------------------------------------------------
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // optional: de-allocate all resources once they've outlived their purpose:
  // ------------------------------------------------------------------------
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);

  // glfw: terminate, clearing all previously allocated GLFW resources.
  // ------------------------------------------------------------------
  glfwTerminate();
  return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera.ProcessKeyboard(FORWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.ProcessKeyboard(BACKWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camera.ProcessKeyboard(LEFT, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
  // make sure the viewport matches the new window dimensions; note that width and
  // height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
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
  float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

  lastX = xpos;
  lastY = ypos;

  camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
  camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void generateGalaxy(int numStars, int arms, float radius)
{
  galaxyVertices.clear();
  srand(static_cast<unsigned>(time(0)));

  float armOffset = 2.0f * M_PI / arms;

  for (int i = 0; i < numStars; i++)
  {
    float r = radius * sqrt(static_cast<float>(rand()) / RAND_MAX);

    int arm = rand() % arms;
    float angle = (r * 1.5f) + arm * armOffset;

    float deviation = ((rand() / (float)RAND_MAX) - 0.5f) * 0.3f;
    float distanceRatio = r / radius;

    float x = r * cos(angle + deviation);
    float y = ((rand() / (float)RAND_MAX) - 0.5f) * 0.2f;
    float z = r * sin(angle + deviation);

    float R = glm::mix(1.0f, 0.5f, distanceRatio);
    float G = glm::mix(0.4f, 0.8f, distanceRatio);
    float B = glm::mix(0.3f, 1.0f, pow(distanceRatio, 0.5f));

    R += ((rand() / (float)RAND_MAX) - 0.5f) * 0.05f;
    G += ((rand() / (float)RAND_MAX) - 0.5f) * 0.05f;
    B += ((rand() / (float)RAND_MAX) - 0.5f) * 0.05f;

    R = glm::clamp(R, 0.0f, 1.0f);
    G = glm::clamp(G, 0.0f, 1.0f);
    B = glm::clamp(B, 0.0f, 1.0f);

    galaxyVertices.push_back(x);
    galaxyVertices.push_back(y);
    galaxyVertices.push_back(z);
    galaxyVertices.push_back(r / radius);
    galaxyVertices.push_back(0.8f + y);
    galaxyVertices.push_back(1.0f);
  }
}

void generateSphere(float radius, int sectorCount, int stackCount)
{
  for (int i = 0; i <= stackCount; ++i)
  {
    float stackAngle = M_PI / 2 - i * M_PI / stackCount; // latitude
    float xy = radius * cosf(stackAngle);
    float z = radius * sinf(stackAngle);

    for (int j = 0; j <= sectorCount; ++j)
    {
      float sectorAngle = j * 2 * M_PI / sectorCount; // longitude

      float x = xy * cosf(sectorAngle);
      float y = xy * sinf(sectorAngle);
      sphereVertices.insert(sphereVertices.end(), {x, y, z});
    }
  }

  // indices
  for (int i = 0; i < stackCount; ++i)
  {
    int k1 = i * (sectorCount + 1);
    int k2 = k1 + sectorCount + 1;

    for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
    {
      sphereIndices.push_back(static_cast<unsigned int>(k1));
      sphereIndices.push_back(static_cast<unsigned int>(k2));
      sphereIndices.push_back(static_cast<unsigned int>(k1 + 1));
      sphereIndices.push_back(static_cast<unsigned int>(k1 + 1));
      sphereIndices.push_back(static_cast<unsigned int>(k2));
      sphereIndices.push_back(static_cast<unsigned int>(k2 + 1));
    }
  }
}