#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <learnopengl/shader_s.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3);
void sierpinski(float x1, float y1, float x2, float y2, float x3, float y3, int depth);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

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

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader program
    // ------------------------------------
    Shader ourShader("3.3.shader.vs", "3.3.shader.fs"); // you can name your shader files however you like

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        // positions         // colors
        0.5f, -0.5f, 0.0f, 0.5f, 0.0f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f, // bottom left
        0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.5f    // top
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    // glBindVertexArray(0);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        double xpos, ypos;
        // input
        // -----
        processInput(window);

        const float threshold = 0.05f; // Adjust as needed for sensitivity

        static int selectedVertex = -1;
        static bool wasPressed = false;

        int mouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

        float timeValue = glfwGetTime();
        float greenValue = (sin(timeValue) / 2.0f);
        float blueValue = (sin(timeValue) / 2.0f);
        float redValue = (cos(timeValue) / 2.0f);
        int vertexColorLocation = glGetUniformLocation(ourShader.ID, "myColor");
        glUseProgram(ourShader.ID);
        glUniform3f(vertexColorLocation, redValue, greenValue, blueValue);

        static int depth = 0;
        static float rotation = 0.00f;

        // Handle key input for depth and rotation
        static double lastDepthChangeTime = 0.0;
        double currentTime = glfwGetTime();
        double debounceDelay = 0.2; // 200 ms debounce

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            if (depth < 20 && currentTime - lastDepthChangeTime > debounceDelay) {
            depth++;
            lastDepthChangeTime = currentTime;
            }
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            if (depth > 0 && currentTime - lastDepthChangeTime > debounceDelay) {
            depth--;
            lastDepthChangeTime = currentTime;
            }
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            rotation += 0.02f;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            rotation -= 0.02f;
        }

        // Apply rotation to all vertices
        float centerX = (vertices[0] + vertices[6] + vertices[12]) / 3.0f;
        float centerY = (vertices[1] + vertices[7] + vertices[13]) / 3.0f;
        for (int i = 0; i < 3; ++i) {
            float x = vertices[i * 6 + 0] - centerX;
            float y = vertices[i * 6 + 1] - centerY;
            float newX = x * cos(rotation) - y * sin(rotation);
            float newY = x * sin(rotation) + y * cos(rotation);
            vertices[i * 6 + 0] = newX + centerX;
            vertices[i * 6 + 1] = newY + centerY;
        }
        rotation = 0.0f; // Only apply rotation once per frame

        // Draw Sierpinski triangle with current depth
        
        if (mouseState == GLFW_PRESS)
        {
            glfwGetCursorPos(window, &xpos, &ypos);
            float x_ndc = (2.0f * xpos) / SCR_WIDTH - 1.0f;
            float y_ndc = 1.0f - (2.0f * ypos) / SCR_HEIGHT;

            if (!wasPressed)
            {
                // On initial press, select nearest vertex if within threshold
                float minDist = std::numeric_limits<float>::max();
                int nearestIndex = -1;
                for (int i = 0; i < 3; ++i)
                {
                    float vx = vertices[i * 6 + 0];
                    float vy = vertices[i * 6 + 1];
                    float dist = (vx - x_ndc) * (vx - x_ndc) + (vy - y_ndc) * (vy - y_ndc);
                    if (dist < minDist)
                    {
                        minDist = dist;
                        nearestIndex = i;
                    }
                }
                if (nearestIndex != -1 && minDist < threshold * threshold)
                {
                    selectedVertex = nearestIndex;
                }
                else
                {
                    selectedVertex = -1;
                }
                wasPressed = true;
            }

            // If a vertex is selected, move it with the mouse
            if (selectedVertex != -1)
            {
                vertices[selectedVertex * 6 + 0] = x_ndc;
                vertices[selectedVertex * 6 + 1] = y_ndc;
            }
        }
        
        else
        {
            // On release, clear selection
            selectedVertex = -1;
            wasPressed = false;
        }

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // render the triangle
        ourShader.use();

        sierpinski(vertices[0], vertices[1], vertices[6], vertices[7], vertices[12], vertices[13], depth);

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
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3) {
    float triangleVertices[] = {
        x1, y1, 0.0f,
        x2, y2, 0.0f,
        x3, y3, 0.0f
    };
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void sierpinski(float x1, float y1, float x2, float y2, float x3, float y3, int depth) {
    if (depth == 0) {
        // Base case: draw the current triangle
        drawTriangle(x1, y1, x2, y2, x3, y3);
        return;
    }

    // Calculate midpoints
    float m1x = (x1 + x2) / 2.0f;
    float m1y = (y1 + y2) / 2.0f;
    float m2x = (x2 + x3) / 2.0f;
    float m2y = (y2 + y3) / 2.0f;
    float m3x = (x3 + x1) / 2.0f;
    float m3y = (y3 + y1) / 2.0f;

    // Recursively call for the three corner triangles
    sierpinski(x1, y1, m1x, m1y, m3x, m3y, depth - 1); // Top triangle
    sierpinski(m1x, m1y, x2, y2, m2x, m2y, depth - 1); // Right triangle
    sierpinski(m3x, m3y, m2x, m2y, x3, y3, depth - 1); // Left triangle
}