---
name: animation-system
description: Use when creating or modifying the animation system for entities - skeleton, bones, animation clips, state machine, blend tree, procedural animation. Triggers on "animação", "skeleton", "bones", "animation system", "character animation".
---

# Animation System Skill

Quando o usuário pedir para criar/modificar animações de entidades, use este sistema.

## Estrutura do Sistema

### Arquivos Principais

- **Header**: `include/gameLayer/gameplay/animationSystem.h`
- **Implementação**: `src/gameLayer/gameplay/animationSystem.cpp`

### Componentes

1. **Bone** - Osso individual com posição, rotação, escala e hierarquia pai
2. **SkeletonData** - Coleção de ossos com hierarquia e transforms
3. **AnimationKeyframe** - Frame de animação com poses de todos os ossos
4. **AnimationClip** - Sequência de keyframes com duração e loop
5. **BlendTree1D** - Blend entre múltiplos clips baseado em parâmetro
6. **AnimationState** - Estado individual com suporte a cross-fade
7. **AnimationStateMachine** - Máquina de estados com transições condicionais

## Hierarquia do Esqueleto Humanoide

```
Hips (0)
├── Spine (1)
│   └── Chest (2)
│       ├── Neck (3)
│       │   └── Head (4)
│       ├── LeftShoulder (5)
│       │   └── LeftUpperArm (6)
│       │       └── LeftLowerArm (7)
│       │           └── LeftHand (8)
│       └── RightShoulder (9)
│           └── RightUpperArm (10)
│               └── RightLowerArm (11)
│                   └── RightHand (12)
├── LeftUpperLeg (13)
│   └── LeftLowerLeg (14)
│       └── LeftFoot (15)
└── RightUpperLeg (16)
    └── RightLowerLeg (17)
        └── RightFoot (18)
```

## Mapeamento para Matrizes do Modelo

O renderer usa um array de 6 matrizes `glm::mat4 m[6]`:
- `m[0]` = Head (índice 4)
- `m[1]` = Body/Hips (índice 0)
- `m[2]` = Right Leg (índice 15)
- `m[3]` = Left Leg (índice 12)
- `m[4]` = Right Arm (índice 9)
- `m[5]` = Left Arm (índice 6)

## Criar Nova Animação

### Passo 1: Definir AnimationClip

```cpp
AnimationClip myAnim;
myAnim.name = "myAnimation";
myAnim.duration = 1.0f;        // segundos
myAnim.ticksPerSecond = 24.f;  // FPS da animação
myAnim.loop = true;            // ou false para uma vez
```

### Passo 2: Criar Keyframes

Cada keyframe precisa ter dados para TODOS os ossos:

```cpp
int boneCount = (int)skeleton.bones.size();

AnimationKeyframe kf;
kf.time = 0.0f;                    // tempo no clip
kf.positions.resize(boneCount);    // posição de cada osso
kf.rotations.resize(boneCount);    // rotação de cada osso
kf.scales.resize(boneCount);       // escala de cada osso

// Definir pose para cada osso
kf.rotations[BONE_INDEX] = glm::angleAxis(
    glm::radians(45.f),           // ângulo em graus
    glm::vec3(1, 0, 0)           // eixo de rotação
);
```

### Passo 3: Adicionar ao PlayerAnimator

```cpp
// Em PlayerAnimator::init()
myAnim.name = "myAnimation";
// ... keyframes ...
animator.myAnimClip = myAnim;  // Adicionar campo no PlayerAnimator
```

### Passo 4: Adicionar Estado à Máquina

```cpp
void PlayerAnimator::setupStateMachine()
{
    stateMachine.addState("myState", &myAnimClip);
    
    stateMachine.addTransition("idle", "myState", 0.2f, [this]()
    { 
        return minhaCondicao; 
    });
}
```

## Criar Animação Procedural (sem keyframes)

Para animações dinâmicas calculadas em runtime:

```cpp
void PlayerClient::setEntityMatrix(glm::mat4 *m)
{
    // Rotação procedural baseada em tempo
    float angle = sin(time * speed) * maxAngle;
    m[BONE_INDEX] = m[BONE_INDEX] * glm::rotate(angle, glm::vec3(1,0,0));
}
```

## Parâmetros de Entrada do Animator

O `PlayerAnimator` recebe estes dados em `update()`:

```cpp
animator.currentSpeed = velocity;    // velocidade horizontal
animator.isGrounded = onGround;      // no chão
animator.isSwimming = inWater;       // na água
animator.isCrouching = crouching;    // agachado
animator.isProne = prone;            // deitado
animator.isFlying = flying;          // voando
animator.isAttacking = attacking;    // atacando
```

## Transições da Máquina de Estados

Estados disponíveis:
- `idle` → parado
- `walk` → andando (0.1 < speed < 5)
- `run` → correndo (speed > 5)
- `jump` → pulando (!grounded)
- `fall` → caindo (velocity.y < 0)
- `crouch` → agachado
- `swim` → nadando
- `attack` → atacando

## Adicionar Nova Parte ao Modelo

Para adicionar uma nova parte (ex: rabo para animais):

1. Adicionar índice no `Model`:
```cpp
std::int8_t tailIndex = -1;
```

2. Adicionar bone ao skeleton:
```cpp
int tail = skeleton.addBone("Tail", hipIndex);
```

3. Animar no `setEntityMatrix`:
```cpp
if (m[tailIndex].length() > 0 && pose.size() > tail)
    m[tailIndex] = m[tailIndex] * glm::mat4_cast(pose[tail].rotation);
```

## Exemplo: Animação de Corrida

```cpp
AnimationClip runClip;
runClip.name = "run";
runClip.duration = 0.4f;
runClip.loop = true;

int boneCount = (int)skeleton.bones.size();

// Frame 0: perna direita à frente
AnimationKeyframe kf0;
kf0.time = 0.f;
kf0.positions.resize(boneCount, glm::vec3(0.f));
kf0.rotations.resize(boneCount, glm::quat(1.f, 0.f, 0.f, 0.f));
kf0.scales.resize(boneCount, glm::vec3(1.f));

kf0.rotations[12] = glm::angleAxis(glm::radians(40.f), glm::vec3(1,0,0)); // RLeg
kf0.rotations[15] = glm::angleAxis(glm::radians(-40.f), glm::vec3(1,0,0)); // LLeg
kf0.rotations[7] = glm::angleAxis(glm::radians(35.f), glm::vec3(1,0,0));  // RArm
kf0.rotations[10] = glm::angleAxis(glm::radians(-35.f), glm::vec3(1,0,0)); // LArm

runClip.keyframes.push_back(kf0);

// Frame 1: perna esquerda à frente (inverter)
AnimationKeyframe kf1;
kf1.time = 0.2f;
kf1.positions.resize(boneCount, glm::vec3(0.f));
kf1.rotations.resize(boneCount, glm::quat(1.f, 0.f, 0.f, 0.f));
kf1.scales.resize(boneCount, glm::vec3(1.f));

kf1.rotations[12] = glm::angleAxis(glm::radians(-40.f), glm::vec3(1,0,0));
kf1.rotations[15] = glm::angleAxis(glm::radians(40.f), glm::vec3(1,0,0));
kf1.rotations[7] = glm::angleAxis(glm::radians(-35.f), glm::vec3(1,0,0));
kf1.rotations[10] = glm::angleAxis(glm::radians(35.f), glm::vec3(1,0,0));

runClip.keyframes.push_back(kf1);
```

## Verificação

1. Compilar: `cmake --build build -j4`
2. Testar em-game: movimentar o player e verificar animações
3. Verificar transições suaves entre estados
4. Debug: usar F3 para ver estado atual (se implementado)

## Notas

- O sistema atual é procedural (seno/cosseno) para compatibilidade
- O novo sistema usa keyframes para animações mais complexas
- `setEntityMatrix` é chamado pelo renderer a cada frame
- As matrizes vêm do modelo 3D já transformadas
- Cada osso aplica sua rotação em cima da matriz existente
