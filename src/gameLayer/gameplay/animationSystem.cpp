#include "gameplay/animationSystem.h"
#include <glm/gtx/quaternion.hpp>
#include <cmath>

void PlayerAnimator::setupHumanoidSkeleton()
{
	skeleton = SkeletonData();

	int root = skeleton.addBone("Hips", -1);
	int spine = skeleton.addBone("Spine", root);
	int chest = skeleton.addBone("Chest", spine);
	int neck = skeleton.addBone("Neck", chest);
	int head = skeleton.addBone("Head", neck);

	int lShoulder = skeleton.addBone("LeftShoulder", chest);
	int lUpperArm = skeleton.addBone("LeftUpperArm", lShoulder);
	int lLowerArm = skeleton.addBone("LeftLowerArm", lUpperArm);
	int lHand = skeleton.addBone("LeftHand", lLowerArm);

	int rShoulder = skeleton.addBone("RightShoulder", chest);
	int rUpperArm = skeleton.addBone("RightUpperArm", rShoulder);
	int rLowerArm = skeleton.addBone("RightLowerArm", rUpperArm);
	int rHand = skeleton.addBone("RightHand", rLowerArm);

	int lUpperLeg = skeleton.addBone("LeftUpperLeg", root);
	int lLowerLeg = skeleton.addBone("LeftLowerLeg", lUpperLeg);
	int lFoot = skeleton.addBone("LeftFoot", lLowerLeg);

	int rUpperLeg = skeleton.addBone("RightUpperLeg", root);
	int rLowerLeg = skeleton.addBone("RightLowerLeg", rUpperLeg);
	int rFoot = skeleton.addBone("RightFoot", rLowerLeg);

	for (auto &bone : skeleton.bones)
	{
		bone.inverseBindPose = glm::inverse(bone.getLocalMatrix());
	}

	skeleton.updateWorldTransforms();
}

void PlayerAnimator::init()
{
	setupHumanoidSkeleton();

	idleClip.name = "idle";
	idleClip.duration = 1.0f;
	idleClip.ticksPerSecond = 24.f;
	idleClip.loop = true;

	{
		AnimationKeyframe kf;
		kf.time = 0.f;
		int boneCount = (int)skeleton.bones.size();
		kf.positions.resize(boneCount, glm::vec3(0.f));
		kf.rotations.resize(boneCount, glm::quat(1.f, 0.f, 0.f, 0.f));
		kf.scales.resize(boneCount, glm::vec3(1.f));

		kf.positions[0] = glm::vec3(0.f, 0.f, 0.f);
		kf.rotations[0] = glm::quat(1.f, 0.f, 0.f, 0.f);
		idleClip.keyframes.push_back(kf);

		kf.time = 0.5f;
		kf.positions[0] = glm::vec3(0.f, 0.02f, 0.f);
		idleClip.keyframes.push_back(kf);

		kf.time = 1.0f;
		kf.positions[0] = glm::vec3(0.f, 0.f, 0.f);
		idleClip.keyframes.push_back(kf);
	}

	walkClip.name = "walk";
	walkClip.duration = 0.6f;
	walkClip.ticksPerSecond = 24.f;
	walkClip.loop = true;

	{
		AnimationKeyframe kf0;
		kf0.time = 0.f;
		int boneCount = (int)skeleton.bones.size();
		kf0.positions.resize(boneCount, glm::vec3(0.f));
		kf0.rotations.resize(boneCount, glm::quat(1.f, 0.f, 0.f, 0.f));
		kf0.scales.resize(boneCount, glm::vec3(1.f));

		kf0.rotations[13] = glm::angleAxis(glm::radians(25.f), glm::vec3(1, 0, 0));
		kf0.rotations[16] = glm::angleAxis(glm::radians(-25.f), glm::vec3(1, 0, 0));
		kf0.rotations[6] = glm::angleAxis(glm::radians(-25.f), glm::vec3(1, 0, 0));
		kf0.rotations[10] = glm::angleAxis(glm::radians(25.f), glm::vec3(1, 0, 0));
		walkClip.keyframes.push_back(kf0);

		AnimationKeyframe kf1;
		kf1.time = 0.3f;
		kf1.positions.resize(boneCount, glm::vec3(0.f));
		kf1.rotations.resize(boneCount, glm::quat(1.f, 0.f, 0.f, 0.f));
		kf1.scales.resize(boneCount, glm::vec3(1.f));

		kf1.rotations[13] = glm::angleAxis(glm::radians(-25.f), glm::vec3(1, 0, 0));
		kf1.rotations[16] = glm::angleAxis(glm::radians(25.f), glm::vec3(1, 0, 0));
		kf1.rotations[6] = glm::angleAxis(glm::radians(25.f), glm::vec3(1, 0, 0));
		kf1.rotations[10] = glm::angleAxis(glm::radians(-25.f), glm::vec3(1, 0, 0));
		walkClip.keyframes.push_back(kf1);

		AnimationKeyframe kf2;
		kf2.time = 0.6f;
		kf2.positions.resize(boneCount, glm::vec3(0.f));
		kf2.rotations.resize(boneCount, glm::quat(1.f, 0.f, 0.f, 0.f));
		kf2.scales.resize(boneCount, glm::vec3(1.f));

		kf2.rotations[13] = glm::angleAxis(glm::radians(25.f), glm::vec3(1, 0, 0));
		kf2.rotations[16] = glm::angleAxis(glm::radians(-25.f), glm::vec3(1, 0, 0));
		kf2.rotations[6] = glm::angleAxis(glm::radians(-25.f), glm::vec3(1, 0, 0));
		kf2.rotations[10] = glm::angleAxis(glm::radians(25.f), glm::vec3(1, 0, 0));
		walkClip.keyframes.push_back(kf2);
	}

	runClip.name = "run";
	runClip.duration = 0.4f;
	runClip.ticksPerSecond = 24.f;
	runClip.loop = true;

	{
		int boneCount = (int)skeleton.bones.size();

		AnimationKeyframe kf0;
		kf0.time = 0.f;
		kf0.positions.resize(boneCount, glm::vec3(0.f));
		kf0.rotations.resize(boneCount, glm::quat(1.f, 0.f, 0.f, 0.f));
		kf0.scales.resize(boneCount, glm::vec3(1.f));

		kf0.rotations[13] = glm::angleAxis(glm::radians(40.f), glm::vec3(1, 0, 0));
		kf0.rotations[16] = glm::angleAxis(glm::radians(-40.f), glm::vec3(1, 0, 0));
		kf0.rotations[6] = glm::angleAxis(glm::radians(-35.f), glm::vec3(1, 0, 0));
		kf0.rotations[10] = glm::angleAxis(glm::radians(35.f), glm::vec3(1, 0, 0));
		runClip.keyframes.push_back(kf0);

		AnimationKeyframe kf1;
		kf1.time = 0.2f;
		kf1.positions.resize(boneCount, glm::vec3(0.f));
		kf1.rotations.resize(boneCount, glm::quat(1.f, 0.f, 0.f, 0.f));
		kf1.scales.resize(boneCount, glm::vec3(1.f));

		kf1.rotations[13] = glm::angleAxis(glm::radians(-40.f), glm::vec3(1, 0, 0));
		kf1.rotations[16] = glm::angleAxis(glm::radians(40.f), glm::vec3(1, 0, 0));
		kf1.rotations[6] = glm::angleAxis(glm::radians(35.f), glm::vec3(1, 0, 0));
		kf1.rotations[10] = glm::angleAxis(glm::radians(-35.f), glm::vec3(1, 0, 0));
		runClip.keyframes.push_back(kf1);

		AnimationKeyframe kf2;
		kf2.time = 0.4f;
		kf2.positions.resize(boneCount, glm::vec3(0.f));
		kf2.rotations.resize(boneCount, glm::quat(1.f, 0.f, 0.f, 0.f));
		kf2.scales.resize(boneCount, glm::vec3(1.f));

		kf2.rotations[13] = glm::angleAxis(glm::radians(40.f), glm::vec3(1, 0, 0));
		kf2.rotations[16] = glm::angleAxis(glm::radians(-40.f), glm::vec3(1, 0, 0));
		kf2.rotations[6] = glm::angleAxis(glm::radians(-35.f), glm::vec3(1, 0, 0));
		kf2.rotations[10] = glm::angleAxis(glm::radians(35.f), glm::vec3(1, 0, 0));
		runClip.keyframes.push_back(kf2);
	}

	jumpClip.name = "jump";
	jumpClip.duration = 0.5f;
	jumpClip.ticksPerSecond = 24.f;
	jumpClip.loop = false;

	{
		int boneCount = (int)skeleton.bones.size();

		AnimationKeyframe kf0;
		kf0.time = 0.f;
		kf0.positions.resize(boneCount, glm::vec3(0.f));
		kf0.rotations.resize(boneCount, glm::quat(1.f, 0.f, 0.f, 0.f));
		kf0.scales.resize(boneCount, glm::vec3(1.f));

		kf0.rotations[12] = glm::angleAxis(glm::radians(-20.f), glm::vec3(1, 0, 0));
		kf0.rotations[15] = glm::angleAxis(glm::radians(20.f), glm::vec3(1, 0, 0));
		jumpClip.keyframes.push_back(kf0);

		AnimationKeyframe kf1;
		kf1.time = 0.25f;
		kf1.positions.resize(boneCount, glm::vec3(0.f));
		kf1.rotations.resize(boneCount, glm::quat(1.f, 0.f, 0.f, 0.f));
		kf1.scales.resize(boneCount, glm::vec3(1.f));

		kf1.rotations[7] = glm::angleAxis(glm::radians(-60.f), glm::vec3(1, 0, 0));
		kf1.rotations[10] = glm::angleAxis(glm::radians(-60.f), glm::vec3(1, 0, 0));
		kf1.rotations[12] = glm::angleAxis(glm::radians(10.f), glm::vec3(1, 0, 0));
		kf1.rotations[15] = glm::angleAxis(glm::radians(-10.f), glm::vec3(1, 0, 0));
		jumpClip.keyframes.push_back(kf1);

		AnimationKeyframe kf2;
		kf2.time = 0.5f;
		kf2.positions.resize(boneCount, glm::vec3(0.f));
		kf2.rotations.resize(boneCount, glm::quat(1.f, 0.f, 0.f, 0.f));
		kf2.scales.resize(boneCount, glm::vec3(1.f));

		kf2.rotations[12] = glm::angleAxis(glm::radians(15.f), glm::vec3(1, 0, 0));
		kf2.rotations[15] = glm::angleAxis(glm::radians(-15.f), glm::vec3(1, 0, 0));
		jumpClip.keyframes.push_back(kf2);
	}

	fallClip.name = "fall";
	fallClip.duration = 0.8f;
	fallClip.ticksPerSecond = 24.f;
	fallClip.loop = true;

	{
		int boneCount = (int)skeleton.bones.size();

		AnimationKeyframe kf0;
		kf0.time = 0.f;
		kf0.positions.resize(boneCount, glm::vec3(0.f));
		kf0.rotations.resize(boneCount, glm::quat(1.f, 0.f, 0.f, 0.f));
		kf0.scales.resize(boneCount, glm::vec3(1.f));

		kf0.rotations[7] = glm::angleAxis(glm::radians(45.f), glm::vec3(1, 0, 0));
		kf0.rotations[10] = glm::angleAxis(glm::radians(45.f), glm::vec3(1, 0, 0));
		kf0.rotations[12] = glm::angleAxis(glm::radians(15.f), glm::vec3(1, 0, 0));
		kf0.rotations[15] = glm::angleAxis(glm::radians(-15.f), glm::vec3(1, 0, 0));
		fallClip.keyframes.push_back(kf0);

		AnimationKeyframe kf1;
		kf1.time = 0.4f;
		kf1.positions.resize(boneCount, glm::vec3(0.f));
		kf1.rotations.resize(boneCount, glm::quat(1.f, 0.f, 0.f, 0.f));
		kf1.scales.resize(boneCount, glm::vec3(1.f));

		kf1.rotations[7] = glm::angleAxis(glm::radians(30.f), glm::vec3(1, 0, 0));
		kf1.rotations[10] = glm::angleAxis(glm::radians(30.f), glm::vec3(1, 0, 0));
		fallClip.keyframes.push_back(kf1);

		AnimationKeyframe kf2;
		kf2.time = 0.8f;
		kf2.positions.resize(boneCount, glm::vec3(0.f));
		kf2.rotations.resize(boneCount, glm::quat(1.f, 0.f, 0.f, 0.f));
		kf2.scales.resize(boneCount, glm::vec3(1.f));

		kf2.rotations[7] = glm::angleAxis(glm::radians(45.f), glm::vec3(1, 0, 0));
		kf2.rotations[10] = glm::angleAxis(glm::radians(45.f), glm::vec3(1, 0, 0));
		fallClip.keyframes.push_back(kf2);
	}

	crouchClip.name = "crouch";
	crouchClip.duration = 1.0f;
	crouchClip.ticksPerSecond = 24.f;
	crouchClip.loop = true;

	{
		int boneCount = (int)skeleton.bones.size();

		AnimationKeyframe kf;
		kf.time = 0.f;
		kf.positions.resize(boneCount, glm::vec3(0.f));
		kf.rotations.resize(boneCount, glm::quat(1.f, 0.f, 0.f, 0.f));
		kf.scales.resize(boneCount, glm::vec3(1.f));

		kf.positions[0] = glm::vec3(0.f, -0.3f, 0.f);
		kf.rotations[12] = glm::angleAxis(glm::radians(15.f), glm::vec3(1, 0, 0));
		kf.rotations[15] = glm::angleAxis(glm::radians(15.f), glm::vec3(1, 0, 0));
		crouchClip.keyframes.push_back(kf);
	}

	swimClip.name = "swim";
	swimClip.duration = 0.8f;
	swimClip.ticksPerSecond = 24.f;
	swimClip.loop = true;

	{
		int boneCount = (int)skeleton.bones.size();

		AnimationKeyframe kf0;
		kf0.time = 0.f;
		kf0.positions.resize(boneCount, glm::vec3(0.f));
		kf0.rotations.resize(boneCount, glm::quat(1.f, 0.f, 0.f, 0.f));
		kf0.scales.resize(boneCount, glm::vec3(1.f));

		kf0.rotations[7] = glm::angleAxis(glm::radians(40.f), glm::vec3(1, 0, 0));
		kf0.rotations[10] = glm::angleAxis(glm::radians(-40.f), glm::vec3(1, 0, 0));
		swimClip.keyframes.push_back(kf0);

		AnimationKeyframe kf1;
		kf1.time = 0.4f;
		kf1.positions.resize(boneCount, glm::vec3(0.f));
		kf1.rotations.resize(boneCount, glm::quat(1.f, 0.f, 0.f, 0.f));
		kf1.scales.resize(boneCount, glm::vec3(1.f));

		kf1.rotations[7] = glm::angleAxis(glm::radians(-40.f), glm::vec3(1, 0, 0));
		kf1.rotations[10] = glm::angleAxis(glm::radians(40.f), glm::vec3(1, 0, 0));
		swimClip.keyframes.push_back(kf1);

		AnimationKeyframe kf2;
		kf2.time = 0.8f;
		kf2.positions.resize(boneCount, glm::vec3(0.f));
		kf2.rotations.resize(boneCount, glm::quat(1.f, 0.f, 0.f, 0.f));
		kf2.scales.resize(boneCount, glm::vec3(1.f));

		kf2.rotations[7] = glm::angleAxis(glm::radians(40.f), glm::vec3(1, 0, 0));
		kf2.rotations[10] = glm::angleAxis(glm::radians(-40.f), glm::vec3(1, 0, 0));
		swimClip.keyframes.push_back(kf2);
	}

	attackClip.name = "attack";
	attackClip.duration = 0.4f;
	attackClip.ticksPerSecond = 24.f;
	attackClip.loop = false;

	{
		int boneCount = (int)skeleton.bones.size();

		AnimationKeyframe kf0;
		kf0.time = 0.f;
		kf0.positions.resize(boneCount, glm::vec3(0.f));
		kf0.rotations.resize(boneCount, glm::quat(1.f, 0.f, 0.f, 0.f));
		kf0.scales.resize(boneCount, glm::vec3(1.f));

		kf0.rotations[10] = glm::angleAxis(glm::radians(-90.f), glm::vec3(1, 0, 0));
		attackClip.keyframes.push_back(kf0);

		AnimationKeyframe kf1;
		kf1.time = 0.15f;
		kf1.positions.resize(boneCount, glm::vec3(0.f));
		kf1.rotations.resize(boneCount, glm::quat(1.f, 0.f, 0.f, 0.f));
		kf1.scales.resize(boneCount, glm::vec3(1.f));

		kf1.rotations[10] = glm::angleAxis(glm::radians(60.f), glm::vec3(1, 0, 0));
		attackClip.keyframes.push_back(kf1);

		AnimationKeyframe kf2;
		kf2.time = 0.4f;
		kf2.positions.resize(boneCount, glm::vec3(0.f));
		kf2.rotations.resize(boneCount, glm::quat(1.f, 0.f, 0.f, 0.f));
		kf2.scales.resize(boneCount, glm::vec3(1.f));

		attackClip.keyframes.push_back(kf2);
	}

	setupStateMachine();
}

void PlayerAnimator::setupStateMachine()
{
	stateMachine.addState("idle", &idleClip);
	stateMachine.addState("walk", &walkClip);
	stateMachine.addState("run", &runClip);
	stateMachine.addState("jump", &jumpClip);
	stateMachine.addState("fall", &fallClip);
	stateMachine.addState("crouch", &crouchClip);
	stateMachine.addState("swim", &swimClip);
	stateMachine.addState("attack", &attackClip);

	stateMachine.addTransition("idle", "walk", 0.15f, [this]()
	{ return currentSpeed > 0.1f && isGrounded && !isSwimming && !isCrouching; });

	stateMachine.addTransition("idle", "run", 0.15f, [this]()
	{ return currentSpeed > 5.f && isGrounded && !isSwimming && !isCrouching; });

	stateMachine.addTransition("idle", "jump", 0.1f, [this]()
	{ return !isGrounded && !isSwimming; });

	stateMachine.addTransition("idle", "crouch", 0.1f, [this]()
	{ return isCrouching && isGrounded; });

	stateMachine.addTransition("idle", "swim", 0.2f, [this]()
	{ return isSwimming; });

	stateMachine.addTransition("walk", "idle", 0.2f, [this]()
	{ return currentSpeed < 0.1f && isGrounded; });

	stateMachine.addTransition("walk", "run", 0.15f, [this]()
	{ return currentSpeed > 5.f && isGrounded && !isSwimming; });

	stateMachine.addTransition("walk", "jump", 0.1f, [this]()
	{ return !isGrounded && !isSwimming; });

	stateMachine.addTransition("walk", "swim", 0.2f, [this]()
	{ return isSwimming; });

	stateMachine.addTransition("run", "walk", 0.15f, [this]()
	{ return currentSpeed < 5.f && currentSpeed > 0.1f && isGrounded; });

	stateMachine.addTransition("run", "idle", 0.2f, [this]()
	{ return currentSpeed < 0.1f && isGrounded; });

	stateMachine.addTransition("run", "jump", 0.1f, [this]()
	{ return !isGrounded && !isSwimming; });

	stateMachine.addTransition("run", "swim", 0.2f, [this]()
	{ return isSwimming; });

	stateMachine.addTransition("jump", "fall", 0.2f, [this]()
	{ return isGrounded || (currentSpeed < -0.1f && !isGrounded); });

	stateMachine.addTransition("jump", "idle", 0.2f, [this]()
	{ return isGrounded; });

	stateMachine.addTransition("fall", "idle", 0.15f, [this]()
	{ return isGrounded; });

	stateMachine.addTransition("fall", "swim", 0.2f, [this]()
	{ return isSwimming; });

	stateMachine.addTransition("crouch", "idle", 0.15f, [this]()
	{ return !isCrouching && isGrounded; });

	stateMachine.addTransition("crouch", "walk", 0.15f, [this]()
	{ return !isCrouching && currentSpeed > 0.1f && isGrounded; });

	stateMachine.addTransition("swim", "idle", 0.3f, [this]()
	{ return !isSwimming && isGrounded; });

	stateMachine.addTransition("swim", "walk", 0.3f, [this]()
	{ return !isSwimming && currentSpeed > 0.1f && isGrounded; });

	stateMachine.addTransition("idle", "attack", 0.05f, [this]()
	{ return isAttacking; });

	stateMachine.addTransition("walk", "attack", 0.05f, [this]()
	{ return isAttacking; });

	stateMachine.addTransition("run", "attack", 0.05f, [this]()
	{ return isAttacking; });

	stateMachine.addTransition("attack", "idle", 0.3f, [this]()
	{ return !isAttacking; });

	stateMachine.currentState = "idle";
}

void PlayerAnimator::blendFromParameters(float speed, bool grounded, bool swimming, bool crouching)
{
	currentSpeed = speed;
	isGrounded = grounded;
	isSwimming = swimming;
	isCrouching = crouching;
}

void PlayerAnimator::update(float deltaTime)
{
	stateMachine.update(deltaTime, skeleton);
	skeleton.updateWorldTransforms();
}
