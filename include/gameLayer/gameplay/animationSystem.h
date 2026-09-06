#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>

struct Bone
{
	std::string name;
	int parentIndex = -1;
	glm::vec3 position = glm::vec3(0.f);
	glm::quat rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
	glm::vec3 scale = glm::vec3(1.f);

	glm::mat4 localTransform = glm::mat4(1.f);
	glm::mat4 worldTransform = glm::mat4(1.f);
	glm::mat4 inverseBindPose = glm::mat4(1.f);

	glm::mat4 getLocalMatrix() const
	{
		glm::mat4 T = glm::translate(glm::mat4(1.f), position);
		glm::mat4 R = glm::mat4_cast(rotation);
		glm::mat4 S = glm::scale(glm::mat4(1.f), scale);
		return T * R * S;
	}
};

struct SkeletonData
{
	std::vector<Bone> bones;
	std::unordered_map<std::string, int> boneNameToIndex;

	int addBone(const std::string &name, int parentIndex = -1)
	{
		int idx = (int)bones.size();
		Bone bone;
		bone.name = name;
		bone.parentIndex = parentIndex;
		bones.push_back(bone);
		boneNameToIndex[name] = idx;
		return idx;
	}

	int getBoneIndex(const std::string &name) const
	{
		auto it = boneNameToIndex.find(name);
		if (it != boneNameToIndex.end()) return it->second;
		return -1;
	}

	void updateWorldTransforms()
	{
		for (int i = 0; i < (int)bones.size(); i++)
		{
			bones[i].localTransform = bones[i].getLocalMatrix();
			if (bones[i].parentIndex >= 0)
			{
				bones[i].worldTransform = bones[bones[i].parentIndex].worldTransform * bones[i].localTransform;
			}
			else
			{
				bones[i].worldTransform = bones[i].localTransform;
			}
		}
	}

	std::vector<glm::mat4> getFinalPoseMatrices() const
	{
		std::vector<glm::mat4> result(bones.size());
		for (int i = 0; i < (int)bones.size(); i++)
		{
			result[i] = bones[i].worldTransform * bones[i].inverseBindPose;
		}
		return result;
	}
};

struct AnimationKeyframe
{
	float time = 0.f;
	std::vector<glm::vec3> positions;
	std::vector<glm::quat> rotations;
	std::vector<glm::vec3> scales;
};

struct AnimationClip
{
	std::string name;
	float duration = 0.f;
	float ticksPerSecond = 24.f;
	bool loop = true;
	std::vector<AnimationKeyframe> keyframes;

	void sample(float time, SkeletonData &skeleton) const
	{
		if (keyframes.empty()) return;

		time = fmod(time, duration);
		if (time < 0.f) time += duration;

		for (int boneIdx = 0; boneIdx < (int)skeleton.bones.size(); boneIdx++)
		{
			for (int k = 0; k < (int)keyframes.size() - 1; k++)
			{
				const auto &kf0 = keyframes[k];
				const auto &kf1 = keyframes[k + 1];

				if (time >= kf0.time && time <= kf1.time)
				{
					float t = (time - kf0.time) / (kf1.time - kf0.time);
					t = glm::clamp(t, 0.f, 1.f);

					if (boneIdx < (int)kf0.positions.size())
					{
						skeleton.bones[boneIdx].position = glm::mix(kf0.positions[boneIdx], kf1.positions[boneIdx], t);
						skeleton.bones[boneIdx].rotation = glm::slerp(kf0.rotations[boneIdx], kf1.rotations[boneIdx], t);
						skeleton.bones[boneIdx].scale = glm::mix(kf0.scales[boneIdx], kf1.scales[boneIdx], t);
					}
					break;
				}
			}
		}
	}
};

struct BlendTree1D
{
	struct Motion
	{
		const AnimationClip *clip = nullptr;
		float threshold = 0.f;
	};

	std::vector<Motion> motions;
	float blendParameter = 0.f;

	void addMotion(const AnimationClip *clip, float threshold)
	{
		motions.push_back({clip, threshold});
	}

	const AnimationClip *evaluate(float parameter)
	{
		if (motions.empty()) return nullptr;
		if (motions.size() == 1) return motions[0].clip;

		blendParameter = parameter;

		int lower = 0;
		int upper = (int)motions.size() - 1;

		for (int i = 0; i < (int)motions.size(); i++)
		{
			if (motions[i].threshold <= parameter) lower = i;
			if (motions[i].threshold >= parameter && upper == (int)motions.size() - 1) upper = i;
		}

		if (lower == upper) return motions[lower].clip;

		float range = motions[upper].threshold - motions[lower].threshold;
		if (range < 0.001f) return motions[lower].clip;

		float t = (parameter - motions[lower].threshold) / range;
		t = glm::clamp(t, 0.f, 1.f);

		if (t < 0.5f) return motions[lower].clip;
		return motions[upper].clip;
	}
};

struct AnimationState
{
	const AnimationClip *currentClip = nullptr;
	float currentTime = 0.f;
	float blendWeight = 1.f;
	bool isFading = false;
	float fadeTime = 0.f;
	float fadeDuration = 0.f;
	const AnimationClip *nextClip = nullptr;
	float nextTime = 0.f;

	void play(const AnimationClip *clip)
	{
		if (currentClip == clip) return;
		currentClip = clip;
		currentTime = 0.f;
		blendWeight = 1.f;
		isFading = false;
	}

	void crossFade(const AnimationClip *clip, float duration)
	{
		if (currentClip == clip) return;
		nextClip = clip;
		nextTime = 0.f;
		fadeDuration = duration;
		fadeTime = 0.f;
		isFading = true;
	}

	void update(float deltaTime, SkeletonData &skeleton)
	{
		if (!currentClip) return;

		currentTime += deltaTime * currentClip->ticksPerSecond;

		if (isFading)
		{
			fadeTime += deltaTime;
			float t = fadeTime / fadeDuration;
			t = glm::clamp(t, 0.f, 1.f);

			blendWeight = 1.f - t;

			if (t >= 1.f)
			{
				currentClip = nextClip;
				currentTime = nextTime;
				blendWeight = 1.f;
				isFading = false;
				nextClip = nullptr;
			}
		}

		if (currentClip->loop)
		{
			currentTime = fmod(currentTime, currentClip->duration);
			if (currentTime < 0.f) currentTime += currentClip->duration;
		}
		else
		{
			currentTime = glm::clamp(currentTime, 0.f, currentClip->duration);
		}

		currentClip->sample(currentTime, skeleton);
	}
};

struct AnimationStateMachine
{
	struct Transition
	{
		std::string from;
		std::string to;
		float duration = 0.2f;
		std::function<bool()> condition;
	};

	std::unordered_map<std::string, AnimationState> states;
	std::vector<Transition> transitions;
	std::string currentState;

	void addState(const std::string &name, const AnimationClip *clip)
	{
		states[name].play(clip);
	}

	void addTransition(const std::string &from, const std::string &to, float duration, std::function<bool()> condition)
	{
		transitions.push_back({from, to, duration, std::move(condition)});
	}

	void update(float deltaTime, SkeletonData &skeleton)
	{
		for (auto &t : transitions)
		{
			if (t.from == currentState && t.condition())
			{
				if (states.count(t.to))
				{
					states[currentState].crossFade(states[t.to].currentClip, t.duration);
					currentState = t.to;
				}
				break;
			}
		}

		if (states.count(currentState))
		{
			states[currentState].update(deltaTime, skeleton);
		}
	}

	void forceState(const std::string &name)
	{
		if (states.count(name))
		{
			currentState = name;
			states[name].play(states[name].currentClip);
		}
	}
};

struct PlayerAnimator
{
	SkeletonData skeleton;
	AnimationStateMachine stateMachine;

	AnimationClip idleClip;
	AnimationClip walkClip;
	AnimationClip runClip;
	AnimationClip jumpClip;
	AnimationClip fallClip;
	AnimationClip crouchClip;
	AnimationClip swimClip;
	AnimationClip attackClip;

	BlendTree1D locomotionBlend;

	float currentSpeed = 0.f;
	bool isGrounded = true;
	bool isSwimming = false;
	bool isAttacking = false;
	bool isCrouching = false;
	bool isProne = false;
	bool isFlying = false;

	void init();
	void update(float deltaTime);
	void setupHumanoidSkeleton();
	void setupStateMachine();
	void blendFromParameters(float speed, bool grounded, bool swimming, bool crouching);
};
