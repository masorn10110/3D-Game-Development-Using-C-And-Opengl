#ifndef DEMON_H
#define DEMON_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/animator.h>
#include <learnopengl/model_animation.h>
#include <iostream>
#include <algorithm> // For glm::min/max

enum class AnimState
{
    IDLE = 1,
    IDLE_PUNCH,
    PUNCH_IDLE,
    IDLE_KICK,
    KICK_IDLE,
    IDLE_WALK,
    WALK_IDLE,
    WALK,
    HURT,        
    HURT_IDLE,
    DEAD         // 🟢 เพิ่มสถานะ DEAD
};

class Demon {
private:
    Model m_model;
    Animator m_animator;

    // Animations
    Animation m_idleAnim;
    Animation m_walkAnim;
    Animation m_castAnim;   // Punch
    Animation m_attackAnim_01; // Kick
    Animation m_hurtAnim;   // Injury (React Small From Front)
    Animation m_deadAnim;   // 🟢 NEW: Standing React Death Backward
    
    AnimState m_charState = AnimState::IDLE;
    float m_blendAmount = 0.0f;
    float m_blendRate = 0.1f; 
    bool m_isDead = false; // 🟢 NEW: สถานะการตาย
    bool m_fireballEmitted = false;
    
    // Transition times (Calculated based on animation duration)
    const float CAST_TRANSITION_TIME;
    const float ATTACK_TRANSITION_TIME_01;
    const float FIRE_EMIT_TIME;
    const float HURT_ANIM_TIME = 0.5f; 
    
    int m_handBoneID;
    float m_stateTime = 0.0f; 

    // --- State Handler Prototypes ---
    void handleStateIdle(GLFWwindow* window);
    void handleStateIdleWalk();
    void handleStateWalk(GLFWwindow* window);
    void handleStateWalkIdle();
    void handleStateIdlePunch();
    void handleStatePunchIdle();
    void handleStateIdleKick();
    void handleStateKickIdle();
    void handleStateHurt();
    void handleStateHurtIdle();
    void handleStateDead(); // 🟢 NEW Handler
    
    // --- Interrupt Logic ---
    bool checkHurtInterrupt(GLFWwindow* window);

public:
    Demon() :
        m_model(FileSystem::getPath("resources/objects/Whiteclown/Whiteclown N Hallin.dae")),
        m_idleAnim(FileSystem::getPath("resources/objects/Whiteclown/standing idle.dae"), &m_model),
        m_walkAnim(FileSystem::getPath("resources/objects/Whiteclown/Standing Walk Forward.dae"), &m_model),
        m_castAnim(FileSystem::getPath("resources/objects/Whiteclown/Standing 2H Cast Spell 01.dae"), &m_model),
        m_attackAnim_01(FileSystem::getPath("resources/objects/Whiteclown/Standing 2H Magic Attack 01.dae"), &m_model),
        m_hurtAnim(FileSystem::getPath("resources/objects/Whiteclown/Standing React Small From Front.dae"), &m_model), // 🟢 อัปเดต Path
        m_deadAnim(FileSystem::getPath("resources/objects/Whiteclown/Standing React Death Backward.dae"), &m_model),   // 🟢 NEW Path
        m_animator(&m_idleAnim),
        CAST_TRANSITION_TIME(m_castAnim.GetDuration() * 0.2f),
        ATTACK_TRANSITION_TIME_01(m_attackAnim_01.GetDuration() * 0.3f),
        FIRE_EMIT_TIME(m_castAnim.GetDuration() * 0.15f)
    {
        try {
            m_handBoneID = m_model.GetBoneInfoMap().at("Armature_mixamorig_RightHand").id;
        } catch (const std::out_of_range& oor) {
            std::cerr << "Error: Bone 'Armature_mixamorig_RightHand' not found in model." << std::endl;
            m_handBoneID = -1;
        }
    }

    void Update(GLFWwindow* window, float deltaTime);
    void Draw(Shader& shader) { m_model.Draw(shader); }
    int GetHandBoneID() const { return m_handBoneID; }
    const std::vector<glm::mat4>& GetFinalBoneMatrices() const { return m_animator.m_FinalBoneMatrices; }

    // 🟢 Public utility for triggering death (e.g., from external game logic)
    void TriggerDeath() { 
        if (!m_isDead) {
            m_isDead = true; 
            m_blendAmount = 0.0f; 
            // Blend จาก Animation ปัจจุบันไปหา Dead Animation
            m_animator.PlayAnimation(m_animator.m_CurrentAnimation, &m_deadAnim, m_animator.m_CurrentTime, 0.0f, m_blendAmount);
            m_charState = AnimState::DEAD;
        }
    }
    bool IsDead() const { return m_isDead; } // Utility function
};

// --- Demon Implementation ---

bool Demon::checkHurtInterrupt(GLFWwindow* window)
{
    if (m_isDead) return false; // 🟢 ถ้าตายแล้ว ไม่ต้องประมวลผล Interrupt อื่นๆ

    // 🟢 ตรวจสอบการกดปุ่ม 'L' สำหรับ Death (Lethal Hit)
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
    {
        TriggerDeath(); 
        return true;
    }

    // ตรวจสอบการกดปุ่ม 'H' สำหรับ HURT
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
    {
        // อนุญาตให้ถูกขัดจังหวะหากอยู่ในสถานะ IDLE, หรือกำลัง Blend กลับไป IDLE
        if (m_charState == AnimState::IDLE || m_charState == AnimState::WALK_IDLE)
        {
            m_blendAmount = 0.0f;
            Animation* currentAnim = m_animator.m_CurrentAnimation;
            
            // Blend จาก Current Animation ไปยัง HURT
            m_animator.PlayAnimation(currentAnim, &m_hurtAnim, m_animator.m_CurrentTime, 0.0f, m_blendAmount);
            m_charState = AnimState::HURT;
            m_stateTime = 0.0f; // Reset state timer
            return true;
        }
        // ไม่อนุญาตให้ถูกขัดจังหวะหากอยู่ในลำดับ PUNCH/KICK หรือ WALK
        else if (m_charState == AnimState::IDLE_PUNCH || m_charState == AnimState::PUNCH_IDLE ||
                 m_charState == AnimState::IDLE_KICK || m_charState == AnimState::KICK_IDLE ||
                 m_charState == AnimState::WALK || m_charState == AnimState::HURT || m_charState == AnimState::HURT_IDLE)
        {
            return false;
        }
    }
    return false;
}

void Demon::Update(GLFWwindow* window, float deltaTime)
{
    // 🟢 ตรวจสอบสถานะความตายก่อน
    if (m_isDead)
    {
        handleStateDead();
        m_animator.UpdateAnimation(deltaTime);
        return;
    }

    // 1. ตรวจสอบการขัดจังหวะ (Interrupt)
    checkHurtInterrupt(window);

    // 2. State Machine update
    switch (m_charState)
    {
    case AnimState::IDLE:
        handleStateIdle(window);
        break;
    case AnimState::IDLE_WALK:
        handleStateIdleWalk();
        break;
    case AnimState::WALK:
        handleStateWalk(window);
        break;
    case AnimState::WALK_IDLE:
        handleStateWalkIdle();
        break;
    case AnimState::IDLE_PUNCH:
        handleStateIdlePunch();
        break;
    case AnimState::PUNCH_IDLE:
        handleStatePunchIdle();
        break;
    case AnimState::IDLE_KICK:
        handleStateIdleKick();
        break;
    case AnimState::KICK_IDLE:
        handleStateKickIdle();
        break;
    case AnimState::HURT:
        handleStateHurt();
        break;
    case AnimState::HURT_IDLE:
        handleStateHurtIdle();
        break;
    case AnimState::DEAD:
        handleStateDead();
        break;
    }
    
    // 3. Update animator
    m_animator.UpdateAnimation(deltaTime);
}

// ... (State Handlers IDLE to KICK_IDLE โค้ดเดิม) ...

void Demon::handleStateIdle(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        m_blendAmount = 0.0f;
        m_animator.PlayAnimation(&m_idleAnim, &m_walkAnim, m_animator.m_CurrentTime, 0.0f, m_blendAmount);
        m_charState = AnimState::IDLE_WALK;
    }
    else if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
    {
        m_blendAmount = 0.0f;
        m_animator.PlayAnimation(&m_idleAnim, &m_castAnim, m_animator.m_CurrentTime, 0.0f, m_blendAmount);
        m_charState = AnimState::IDLE_PUNCH;
    }
    else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
    {
        m_blendAmount = 0.0f;
        m_animator.PlayAnimation(&m_idleAnim, &m_attackAnim_01, m_animator.m_CurrentTime, 0.0f, m_blendAmount);
        m_charState = AnimState::IDLE_KICK;
    }
}

void Demon::handleStateIdleWalk()
{
    m_blendAmount += m_blendRate;
    m_blendAmount = glm::min(m_blendAmount, 1.0f);
    m_animator.PlayAnimation(&m_idleAnim, &m_walkAnim, m_animator.m_CurrentTime, m_animator.m_CurrentTime2, m_blendAmount);
    if (m_blendAmount >= 1.0f)
    {
        m_blendAmount = 0.0f;
        float startTime = m_animator.m_CurrentTime2;
        m_animator.PlayAnimation(&m_walkAnim, NULL, startTime, 0.0f, m_blendAmount);
        m_charState = AnimState::WALK;
    }
}

void Demon::handleStateWalk(GLFWwindow* window)
{
    m_animator.PlayAnimation(&m_walkAnim, NULL, m_animator.m_CurrentTime, m_animator.m_CurrentTime2, m_blendAmount);
    if (glfwGetKey(window, GLFW_KEY_UP) != GLFW_PRESS)
    {
        m_charState = AnimState::WALK_IDLE;
    }
}

void Demon::handleStateWalkIdle()
{
    m_blendAmount += m_blendRate;
    m_blendAmount = glm::min(m_blendAmount, 1.0f);
    m_animator.PlayAnimation(&m_walkAnim, &m_idleAnim, m_animator.m_CurrentTime, m_animator.m_CurrentTime2, m_blendAmount);
    if (m_blendAmount >= 1.0f)
    {
        m_blendAmount = 0.0f;
        float startTime = m_animator.m_CurrentTime2;
        m_animator.PlayAnimation(&m_idleAnim, NULL, startTime, 0.0f, m_blendAmount);
        m_charState = AnimState::IDLE;
    }
}

void Demon::handleStateIdlePunch()
{
    m_blendAmount += m_blendRate;
    m_blendAmount = glm::min(m_blendAmount, 1.0f);
    m_animator.PlayAnimation(&m_idleAnim, &m_castAnim, m_animator.m_CurrentTime, m_animator.m_CurrentTime2, m_blendAmount);
    if (m_blendAmount >= 1.0f)
    {
        m_blendAmount = 0.0f;
        float startTime = m_animator.m_CurrentTime2;
        m_animator.PlayAnimation(&m_castAnim, NULL, startTime, 0.0f, m_blendAmount);
        m_charState = AnimState::PUNCH_IDLE;
    }
}

void Demon::handleStatePunchIdle()
{
  if (m_animator.m_CurrentTime >= FIRE_EMIT_TIME && !m_fireballEmitted)
    {
        // 🟢 จุดนี้คือจุดที่ Fireball ถูกสร้างขึ้น
        // (ต้องเรียกฟังก์ชันใน main/game loop)
        // ตัวอย่าง: TriggerFireball(); 
        m_fireballEmitted = true;
    }
    if (m_animator.m_CurrentTime >= CAST_TRANSITION_TIME)
    {
        m_blendAmount += m_blendRate;
        m_blendAmount = glm::min(m_blendAmount, 1.0f);
        m_animator.PlayAnimation(&m_castAnim, &m_idleAnim, m_animator.m_CurrentTime, m_animator.m_CurrentTime2, m_blendAmount);
        if (m_blendAmount >= 1.0f)
        {
            m_blendAmount = 0.0f;
            float startTime = m_animator.m_CurrentTime2;
            m_animator.PlayAnimation(&m_idleAnim, NULL, startTime, 0.0f, m_blendAmount);
            m_charState = AnimState::IDLE;
        }
    }
}

void Demon::handleStateIdleKick()
{
    m_blendAmount += m_blendRate;
    m_blendAmount = glm::min(m_blendAmount, 1.0f);
    m_animator.PlayAnimation(&m_idleAnim, &m_attackAnim_01, m_animator.m_CurrentTime, m_animator.m_CurrentTime2, m_blendAmount);
    if (m_blendAmount >= 1.0f)
    {
        m_blendAmount = 0.0f;
        float startTime = m_animator.m_CurrentTime2;
        m_animator.PlayAnimation(&m_attackAnim_01, NULL, startTime, 0.0f, m_blendAmount);
        m_charState = AnimState::KICK_IDLE;
    }
}

void Demon::handleStateKickIdle()
{
    if (m_animator.m_CurrentTime >= ATTACK_TRANSITION_TIME_01)
    {
        m_blendAmount += m_blendRate;
        m_blendAmount = glm::min(m_blendAmount, 1.0f);
        m_animator.PlayAnimation(&m_attackAnim_01, &m_idleAnim, m_animator.m_CurrentTime, m_animator.m_CurrentTime2, m_blendAmount);
        if (m_blendAmount >= 1.0f)
        {
            m_blendAmount = 0.0f;
            float startTime = m_animator.m_CurrentTime2;
            m_animator.PlayAnimation(&m_idleAnim, NULL, startTime, 0.0f, m_blendAmount);
            m_charState = AnimState::IDLE;
        }
    }
}


void Demon::handleStateHurt()
{
    // Play the animation and continue blending to HURT
    m_animator.PlayAnimation(m_animator.m_CurrentAnimation, &m_hurtAnim, m_animator.m_CurrentTime, 0.0f, m_blendAmount);

    m_blendAmount += m_blendRate;
    m_blendAmount = glm::min(m_blendAmount, 1.0f);

    if (m_blendAmount >= 1.0f)
    {
        // Once blending is complete, start timing the fixed duration
        m_stateTime += m_animator.m_DeltaTime;

        if (m_stateTime >= HURT_ANIM_TIME)
        {
            // Time is up, transition to blend back to IDLE
            m_blendAmount = 0.0f;
            m_charState = AnimState::HURT_IDLE;
        }
    }
}

void Demon::handleStateHurtIdle()
{
    // Blend from HURT back to IDLE
    m_blendAmount += m_blendRate;
    m_blendAmount = glm::min(m_blendAmount, 1.0f);
    m_animator.PlayAnimation(&m_hurtAnim, &m_idleAnim, m_animator.m_CurrentTime, m_animator.m_CurrentTime2, m_blendAmount);
    
    if (m_blendAmount >= 1.0f)
    {
        // Transition complete, switch to IDLE state
        m_blendAmount = 0.0f;
        float startTime = m_animator.m_CurrentTime2;
        m_animator.PlayAnimation(&m_idleAnim, NULL, startTime, 0.0f, m_blendAmount);
        m_charState = AnimState::IDLE;
    }
}

void Demon::handleStateDead()
{
    // 🟢 NEW Handler: จัดการสถานะการตาย
    m_animator.PlayAnimation(m_animator.m_CurrentAnimation, &m_deadAnim, m_animator.m_CurrentTime, 0.0f, m_blendAmount);

    m_blendAmount += m_blendRate;
    m_blendAmount = glm::min(m_blendAmount, 1.0f);

    // เมื่อ Blend เสร็จแล้ว ให้ Animation เล่นจนจบและหยุดที่เฟรมสุดท้าย
    if (m_blendAmount >= 1.0f)
    {
        // Lock animation time to the end of the dead animation duration
        if (m_animator.m_CurrentTime >= m_deadAnim.GetDuration())
        {
            m_animator.m_CurrentTime = m_deadAnim.GetDuration();
        }
    }
    // ไม่มีการเปลี่ยนสถานะออกจาก DEAD
}

#endif // DEMON_H