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

// ---------------------------------------------------------------
// Settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Camera (used only for pitch and zoom)
Camera camera(glm::vec3(0.0f, 1.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Physic
const float GRAVITY = -9.8f;
const float JUMP_STRENGTH = 5.0f;
const float GROUND_LEVEL = 0.0f;

float cameraYaw = 0.0f;

// Player structure
struct Player
{
    glm::vec3 position = glm::vec3(0.0f, -0.4f, 0.0f);
    float yaw = 0.0f;
    float velocityY = 0.0f;
    bool onGround = true;
};
Player player;

struct Box
{
    glm::vec3 position;
    glm::vec3 size;
};

// Function declarations
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
bool CheckCollision(const glm::vec3 &pPos, const glm::vec3 &pSize,
                    const glm::vec3 &bPos, const glm::vec3 &bSize);

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Third Person Camera", NULL, NULL);
    if (!window)
    {
        std::cout << "Failed to create GLFW window\n";
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
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    stbi_set_flip_vertically_on_load(true);

    Shader animShader("anim_model.vs", "anim_model.fs");
    Shader staticShader("model_loading.vs", "model_loading.fs");

    // Model character(FileSystem::getPath("resources/objects/vampire/dancing_vampire.dae"));
    // Animation anim(FileSystem::getPath("resources/objects/vampire/dancing_vampire.dae"), &character);
    // Animator animator(&anim);

    Model character(FileSystem::getPath("resources/objects/Remy/Remy.dae"));

    Animation idleAnim(FileSystem::getPath("resources/objects/Remy/Idle.dae"), &character);
    Animation runAnim(FileSystem::getPath("resources/objects/Remy/Walking.dae"), &character);

    Animator animator(&idleAnim);

    bool isRunning = false;

    Model backpack(FileSystem::getPath("resources/objects/rock/rock.obj"));

    // Distance between camera and player
    float camDistance = 5.0f;

    player.position = glm::vec3(0.0f, 0.0f, 10.0f);
    Box rockBox = {glm::vec3(0.0f, -0.5f, 0.0f), glm::vec3(2.0f, 2.0f, 2.0f)};
    glm::vec3 playerSize(1.0f, 1.0f, 1.0f);

    // Main Loop
    while (!glfwWindowShouldClose(window))
    {
        // Delta time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        bool moving = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ||
                      glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;

        if (moving && !isRunning)
        {
            animator.PlayAnimation(&runAnim);
            isRunning = true;
        }
        else if (!moving && isRunning)
        {
            animator.PlayAnimation(&idleAnim);
            isRunning = false;
        }

        // อัปเดต animation ทุก frame
        animator.UpdateAnimation(deltaTime);

        if (player.onGround && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            player.velocityY = JUMP_STRENGTH;
            player.onGround = false;
        }

        // อัปเดตตำแหน่งแนวดิ่ง
        player.velocityY += GRAVITY * deltaTime;
        player.position.y += player.velocityY * deltaTime;

        // ตรวจสอบชนพื้น
        if (player.position.y <= GROUND_LEVEL)
        {
            player.position.y = GROUND_LEVEL;
            player.velocityY = 0.0f;
            player.onGround = true;
        }

        if (CheckCollision(player.position, playerSize, rockBox.position, rockBox.size))
        {

            glm::vec3 diff = player.position - rockBox.position;
            glm::vec3 overlap = glm::vec3(
                (playerSize.x + rockBox.size.x) * 0.5f - abs(diff.x),
                (playerSize.y + rockBox.size.y) * 0.5f - abs(diff.y),
                (playerSize.z + rockBox.size.z) * 0.5f - abs(diff.z));

            // มีการชน
            if (overlap.x > 0 && overlap.y > 0 && overlap.z > 0)
            {
                // ถ้าชนด้านบนหิน
                if (overlap.y < overlap.x && overlap.y < overlap.z && diff.y > 0.0f)
                {
                    player.position.y = rockBox.position.y + (rockBox.size.y + playerSize.y) * 0.5f;
                    player.velocityY = 0.0f;
                    player.onGround = true;
                }
                else
                {
                    // ชนด้านข้าง
                    if (overlap.x < overlap.z)
                    {
                        // ด้านซ้ายหรือขวา
                        if (diff.x > 0)
                            player.position.x = rockBox.position.x + (rockBox.size.x + playerSize.x) * 0.5f;
                        else
                            player.position.x = rockBox.position.x - (rockBox.size.x + playerSize.x) * 0.5f;
                    }
                    else
                    {
                        // ด้านหน้า-หลัง
                        if (diff.z > 0)
                            player.position.z = rockBox.position.z + (rockBox.size.z + playerSize.z) * 0.5f;
                        else
                            player.position.z = rockBox.position.z - (rockBox.size.z + playerSize.z) * 0.5f;
                    }
                }
            }
            if (player.velocityY < 0.0f && player.position.y > rockBox.position.y)
            {
                player.position.y = rockBox.position.y + rockBox.size.y / 2 + playerSize.y / 2;
                player.velocityY = 0.0f;
                player.onGround = true;
            }
        }

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- Movement Logic ---
        float speed = 2.5f * deltaTime;

        // front vector (ทิศที่ตัวละครหัน)
        glm::vec3 frontDir = glm::normalize(glm::vec3(
            sin(glm::radians(player.yaw)),
            0.0f,
            -cos(glm::radians(player.yaw))));

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            player.position += frontDir * speed;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            player.position -= frontDir * speed;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            player.yaw -= 90.0f * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            player.yaw += 90.0f * deltaTime;

        // float yawDiff = cameraYaw - player.yaw;
        // player.yaw += yawDiff * deltaTime * 5.0f;
        // player.yaw = cameraYaw;

        // --- Camera Position ---
        float pitch = camera.Pitch;
        // glm::vec3 behindDir = glm::normalize(glm::vec3(
        //     -frontDir.x,
        //     0.0f,
        //     -frontDir.z));

        // glm::vec3 camPos = player.position - behindDir * camDistance + glm::vec3(0.0f, 1.0f, 0.0f);
        // camera.Position = camPos;
        // camera.Front = glm::normalize(player.position - camPos);
        glm::vec3 camOffset;
        camOffset.x = sin(glm::radians(cameraYaw)) * cos(glm::radians(camera.Pitch));
        camOffset.y = sin(glm::radians(camera.Pitch));
        camOffset.z = -cos(glm::radians(cameraYaw)) * cos(glm::radians(camera.Pitch));

        camera.Position = player.position - camOffset * camDistance + glm::vec3(0.0f, 3.0f, 0.0f);
        camera.Front = glm::normalize(player.position - camera.Position);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                                (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                                0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // --- Draw Character ---
        animShader.use();
        animShader.setMat4("projection", projection);
        animShader.setMat4("view", view);

        auto transforms = animator.GetFinalBoneMatrices();
        for (int i = 0; i < transforms.size(); ++i)
            animShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, player.position);
        model = glm::scale(model, glm::vec3(0.5f));
        model = glm::rotate(model, glm::radians(-player.yaw + 180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        animShader.setMat4("model", model);
        character.Draw(animShader);

        // --- Draw Backpack ---
        staticShader.use();
        staticShader.setMat4("projection", projection);
        staticShader.setMat4("view", view);

        glm::mat4 rockModel = glm::mat4(1.0f);
        rockModel = glm::translate(rockModel, rockBox.position);
        staticShader.setMat4("model", rockModel);
        backpack.Draw(staticShader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// ---------------------------------------------------------
// Input
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// ---------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// ---------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    static bool firstMouse = true;
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // คว่ำแกน Y ให้ขยับเมาส์ขึ้น = มองขึ้น
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    // --- หมุนตัวละครด้วยเมาส์ซ้าย-ขวา ---
    player.yaw += xoffset;

    // --- ปรับมุมกล้องขึ้นลงด้วยเมาส์ ---
    cameraYaw += xoffset; // กล้องหันรอบตัว
    camera.Pitch += yoffset;
    camera.Pitch = glm::clamp(camera.Pitch, -30.0f, 45.0f);
}

// ---------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

bool CheckCollision(const glm::vec3 &pPos, const glm::vec3 &pSize,
                    const glm::vec3 &bPos, const glm::vec3 &bSize)
{
    return (abs(pPos.x - bPos.x) * 2 < (pSize.x + bSize.x)) &&
           (abs(pPos.y - bPos.y) * 2 < (pSize.y + bSize.y)) &&
           (abs(pPos.z - bPos.z) * 2 < (pSize.z + bSize.z));
}