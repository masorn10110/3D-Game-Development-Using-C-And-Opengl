#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/animator.h>
#include <learnopengl/model_animation.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 800;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

enum AnimState
{
    IDLE = 1,
    IDLE_PUNCH,
    PUNCH_IDLE,
    IDLE_KICK,
    KICK_IDLE,
    IDLE_WALK,
    WALK_IDLE,
    WALK
};

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

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------

    Shader staffShader("model_loading_1.vs", "model_loading_1.fs");

    Model staffModel(FileSystem::getPath("resources/objects/staff/Staff.obj"));

    Shader ourShader("anim_model_1.vs", "anim_model_1.fs");
    // load models
    // -----------
    // idle 3.3, walk 2.06, run 0.83, punch 1.03, kick 1.6
    Model ourModel(FileSystem::getPath("resources/objects/Whiteclown/Whiteclown N Hallin.dae"));
    Animation idleAnimation(FileSystem::getPath("resources/objects/Whiteclown/standing idle.dae"), &ourModel);
    Animation walkAnimation(FileSystem::getPath("resources/objects/Whiteclown/Standing Walk Forward.dae"), &ourModel);
    Animation runAnimation(FileSystem::getPath("resources/objects/Whiteclown/Standing Run Forward.dae"), &ourModel);
    Animation castAnimation(FileSystem::getPath("resources/objects/Whiteclown/Standing 2H Cast Spell 01.dae"), &ourModel);
    Animation attackAnimation(FileSystem::getPath("resources/objects/Whiteclown/Standing 2H Magic Attack 01.dae"), &ourModel);
    Animator animator(&idleAnimation);

    const float PUNCH_TRANSITION_TIME = castAnimation.GetDuration() * 0.2f;
    const float KICK_TRANSITION_TIME = attackAnimation.GetDuration() * 0.3f;
    enum AnimState charState = IDLE;
    float blendAmount = 0.0f;
    // 🟢 แก้ไข: เพิ่มความเร็วในการ Blend
    float blendRate = 0.1f; 

    auto boneMap = ourModel.GetBoneInfoMap();

    int handBoneID = ourModel.GetBoneInfoMap().at("Armature_mixamorig_RightHand").id;

    // ดึง matrix ของมือขวาจาก animator
    glm::mat4 handMatrix = animator.m_FinalBoneMatrices[handBoneID];

    for (auto &b : boneMap)
        std::cout << b.first << std::endl;

    // draw in wireframe
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
            animator.PlayAnimation(&idleAnimation, NULL, 0.0f, 0.0f, 0.0f);
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
            animator.PlayAnimation(&walkAnimation, NULL, 0.0f, 0.0f, 0.0f);
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
            animator.PlayAnimation(&castAnimation, NULL, 0.0f, 0.0f, 0.0f);
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
            animator.PlayAnimation(&attackAnimation, NULL, 0.0f, 0.0f, 0.0f);

        switch (charState)
        {
        case IDLE:
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            {
                blendAmount = 0.0f;
                animator.PlayAnimation(&idleAnimation, &walkAnimation, animator.m_CurrentTime, 0.0f, blendAmount);
                charState = IDLE_WALK;
            }
            else if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
            {
                blendAmount = 0.0f;
                animator.PlayAnimation(&idleAnimation, &castAnimation, animator.m_CurrentTime, 0.0f, blendAmount);
                charState = IDLE_PUNCH;
            }
            else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
            {
                blendAmount = 0.0f;
                animator.PlayAnimation(&idleAnimation, &attackAnimation, animator.m_CurrentTime, 0.0f, blendAmount);
                charState = IDLE_KICK;
            }
            printf("idle \n");
            break;
        case IDLE_WALK:
            blendAmount += blendRate;
            // ใช้ glm::min(blendAmount, 1.0f) แทน fmod(blendAmount, 1.0f) เพื่อให้ Blend หยุดที่ 1.0f
            blendAmount = glm::min(blendAmount, 1.0f); 
            animator.PlayAnimation(&idleAnimation, &walkAnimation, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
            if (blendAmount >= 1.0f)
            {
                blendAmount = 0.0f;
                float startTime = animator.m_CurrentTime2;
                animator.PlayAnimation(&walkAnimation, NULL, startTime, 0.0f, blendAmount);
                charState = WALK;
            }
            printf("idle_walk \n");
            break;
        case WALK:
            animator.PlayAnimation(&walkAnimation, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
            if (glfwGetKey(window, GLFW_KEY_UP) != GLFW_PRESS)
            {
                charState = WALK_IDLE;
            }
            printf("walking\n");
            break;
        case WALK_IDLE:
            blendAmount += blendRate;
            blendAmount = glm::min(blendAmount, 1.0f);
            animator.PlayAnimation(&walkAnimation, &idleAnimation, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
            if (blendAmount >= 1.0f)
            {
                blendAmount = 0.0f;
                float startTime = animator.m_CurrentTime2;
                animator.PlayAnimation(&idleAnimation, NULL, startTime, 0.0f, blendAmount);
                charState = IDLE;
            }
            printf("walk_idle \n");
            break;
        case IDLE_PUNCH:
            blendAmount += blendRate;
            blendAmount = glm::min(blendAmount, 1.0f);
            animator.PlayAnimation(&idleAnimation, &castAnimation, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
            if (blendAmount >= 1.0f)
            {
                blendAmount = 0.0f;
                float startTime = animator.m_CurrentTime2;
                animator.PlayAnimation(&castAnimation, NULL, startTime, 0.0f, blendAmount);
                charState = PUNCH_IDLE;
            }
            printf("idle_punch\n");
            break;
        case PUNCH_IDLE:
            // 🟢 แก้ไข: ใช้ >= 1.0f และใช้ glm::min() ในการ Blend
            if (animator.m_CurrentTime >= PUNCH_TRANSITION_TIME)
            {
                blendAmount += blendRate;
                blendAmount = glm::min(blendAmount, 1.0f);
                animator.PlayAnimation(&castAnimation, &idleAnimation, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
                if (blendAmount >= 1.0f)
                {
                    blendAmount = 0.0f;
                    float startTime = animator.m_CurrentTime2;
                    animator.PlayAnimation(&idleAnimation, NULL, startTime, 0.0f, blendAmount);
                    charState = IDLE;
                }
                printf("punch_idle \n");
            }
            else
            {
                // punching
                printf("punching \n");
            }
            break;
        case IDLE_KICK:
            blendAmount += blendRate;
            blendAmount = glm::min(blendAmount, 1.0f);
            animator.PlayAnimation(&idleAnimation, &attackAnimation, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
            if (blendAmount >= 1.0f)
            {
                blendAmount = 0.0f;
                float startTime = animator.m_CurrentTime2;
                animator.PlayAnimation(&attackAnimation, NULL, startTime, 0.0f, blendAmount);
                charState = KICK_IDLE;
            }
            printf("idle_kick\n");
            break;
        case KICK_IDLE:
            // 🟢 แก้ไข: ใช้ >= 1.0f และใช้ glm::min() ในการ Blend
            if (animator.m_CurrentTime >= KICK_TRANSITION_TIME)
            {
                blendAmount += blendRate;
                blendAmount = glm::min(blendAmount, 1.0f);
                animator.PlayAnimation(&attackAnimation, &idleAnimation, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
                if (blendAmount >= 1.0f)
                {
                    blendAmount = 0.0f;
                    float startTime = animator.m_CurrentTime2;
                    animator.PlayAnimation(&idleAnimation, NULL, startTime, 0.0f, blendAmount);
                    charState = IDLE;
                }
                printf("kick_idle \n");
            }
            else
            {
                // punching
                printf("kicking \n");
            }
            break;
        }

        animator.UpdateAnimation(deltaTime);

        // render
        // ------
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // ดึงเมทริกซ์กระดูกที่ถูกอัปเดตแล้ว
        auto transforms = animator.GetFinalBoneMatrices();
        for (int i = 0; i < transforms.size(); ++i)
            ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

        // 1. Model Matrix ของตัวละคร
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.4f, 0.0f)); // World Position (เท้า)
        model = glm::scale(model, glm::vec3(.5f, .5f, .5f));         // World Scale
        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);

        // --- เริ่มคำนวณ Staff Transform ---

        // 2. ดึง Final Bone Matrix ของมือขวา (จาก transforms ที่เพิ่งถูกส่งไป Shader)
        glm::mat4 handBoneTransform = transforms[handBoneID];

        // 3. World Matrix ของมือ (รวม Model Matrix ของตัวละคร)
        glm::mat4 worldHandMatrix = model * handBoneTransform;

        // 4. Offset Matrix ของ Staff
        glm::mat4 staffOffset = glm::mat4(1.0f);
        staffOffset = glm::rotate(staffOffset, glm::radians(180.0f), glm::vec3(0, 0, 1));
        staffOffset = glm::translate(staffOffset, glm::vec3(1.0f, -0.8f, 1.65f));
        staffOffset = glm::scale(staffOffset, glm::vec3(0.3f));

        // 5. World Matrix ของ Staff
        glm::mat4 staffModelMatrix = worldHandMatrix * staffOffset;

        // 6. วาด Staff
        staffShader.use();
        staffShader.setMat4("projection", projection);
        staffShader.setMat4("view", view);
        staffShader.setMat4("model", staffModelMatrix);
        staffModel.Draw(staffShader);
        ourShader.use();

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// ... (omitted helper functions) ...
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

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
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

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}