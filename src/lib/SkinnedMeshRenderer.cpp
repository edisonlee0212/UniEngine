#include <Application.hpp>
#include <DefaultResources.hpp>
#include "Editor.hpp"
#include "Engine/Rendering/Graphics.hpp"
#include <SkinnedMeshRenderer.hpp>
using namespace UniEngine;
void SkinnedMeshRenderer::RenderBound(glm::vec4& color)
{
	auto scene = GetScene();
	const auto transform = scene->GetDataComponent<GlobalTransform>(GetOwner()).m_value;
	glm::vec3 size = m_skinnedMesh.Get<SkinnedMesh>()->m_bound.Size();
	if (size.x < 0.01f)
		size.x = 0.01f;
	if (size.z < 0.01f)
		size.z = 0.01f;
	if (size.y < 0.01f)
		size.y = 0.01f;
	GizmoSettings gizmoSettings;
	gizmoSettings.m_drawSettings.m_cullFace = false;
	gizmoSettings.m_drawSettings.m_blending = true;
	gizmoSettings.m_drawSettings.m_polygonMode = OpenGLPolygonMode::Line;
	gizmoSettings.m_drawSettings.m_lineWidth = 3.0f;
	Gizmos::DrawGizmoMesh(
		DefaultResources::Primitives::Cube,
		color,
		transform * (glm::translate(m_skinnedMesh.Get<SkinnedMesh>()->m_bound.Center()) * glm::scale(size)),
		1, gizmoSettings);
}

void SkinnedMeshRenderer::GetBoneMatrices()
{
	auto scene = GetScene();
	auto animator = m_animator.Get<Animator>();
	if (!animator)
		return;
	auto skinnedMesh = m_skinnedMesh.Get<SkinnedMesh>();
	if (!skinnedMesh)
		return;
	if (m_ragDoll)
	{
		if (m_ragDollFreeze)
			return;
		
		m_finalResults->m_value.resize(skinnedMesh->m_boneAnimatorIndices.size());
		for (int i = 0; i < m_boundEntities.size(); i++)
		{
			auto entity = m_boundEntities[i].Get();
			if (entity.GetIndex() != 0)
			{
				m_ragDollTransformChain[i] = scene->GetDataComponent<GlobalTransform>(entity).m_value;
				m_ragDollTransformChain[i] *= animator->m_offsetMatrices[i];
			}
		}
		for (int i = 0; i < skinnedMesh->m_boneAnimatorIndices.size(); i++)
		{
			m_finalResults->m_value[i] = m_ragDollTransformChain[skinnedMesh->m_boneAnimatorIndices[i]];
		}
		m_finalResults->Update();
	}
	else
	{
		auto skinnedMesh = m_skinnedMesh.Get<SkinnedMesh>();
		if (animator->m_boneSize == 0)
			return;
		m_finalResults->m_value.resize(skinnedMesh->m_boneAnimatorIndices.size());
		for (int i = 0; i < skinnedMesh->m_boneAnimatorIndices.size(); i++)
		{
			m_finalResults->m_value[i] = animator->m_transformChain[skinnedMesh->m_boneAnimatorIndices[i]];
		}
		m_finalResults->Update();
	}
}

void SkinnedMeshRenderer::DebugBoneRender(const glm::vec4& color, const float& size)
{
	auto scene = GetScene();
	auto owner = GetOwner();
	const auto selfScale = scene->GetDataComponent<GlobalTransform>(owner).GetScale();
	auto animator = m_animator.Get<Animator>();
	if (!animator) return;
	std::vector<glm::mat4> debugRenderingMatrices;
	GlobalTransform ltw;
	if (!m_ragDoll)
	{
		debugRenderingMatrices = animator->m_transformChain;
		ltw = scene->GetDataComponent<GlobalTransform>(owner);
	}
	else {
		debugRenderingMatrices = m_ragDollTransformChain;
	}
	for (int index = 0; index < debugRenderingMatrices.size(); index++)
	{
		debugRenderingMatrices[index] =
			debugRenderingMatrices[index] * glm::inverse(animator->m_offsetMatrices[index]) * glm::inverse(glm::scale(selfScale));
	}
	Gizmos::DrawGizmoMeshInstanced(
		DefaultResources::Primitives::Sphere, color, debugRenderingMatrices, ltw.m_value, size);
}

void SkinnedMeshRenderer::OnInspect()
{
	Editor::DragAndDropButton<Animator>(m_animator, "Animator");
	ImGui::Checkbox("Forward Rendering##SkinnedMeshRenderer", &m_forwardRendering);
	if (!m_forwardRendering)
		ImGui::Checkbox("Receive shadow##SkinnedMeshRenderer", &m_receiveShadow);
	ImGui::Checkbox("Cast shadow##SkinnedMeshRenderer", &m_castShadow);
	Editor::DragAndDropButton<Material>(m_material, "Material");
	Editor::DragAndDropButton<SkinnedMesh>(m_skinnedMesh, "Skinned Mesh");
	if (m_skinnedMesh.Get<SkinnedMesh>())
	{
		if (ImGui::TreeNode("Skinned Mesh:##SkinnedMeshRenderer"))
		{
			static bool displayBound = true;
			ImGui::Checkbox("Display bounds##SkinnedMeshRenderer", &displayBound);
			if (displayBound)
			{
				static auto displayBoundColor = glm::vec4(0.0f, 1.0f, 0.0f, 0.2f);
				ImGui::ColorEdit4("Color:##SkinnedMeshRenderer", (float*)(void*)&displayBoundColor);
				RenderBound(displayBoundColor);
			}
			ImGui::TreePop();
		}
	}
	auto animator = m_animator.Get<Animator>();
	if (animator)
	{
		static bool debugRenderBones = true;
		static float debugRenderBonesSize = 0.5f;
		static glm::vec4 debugRenderBonesColor = glm::vec4(1, 1, 0, 0.5);
		ImGui::Checkbox("Display bones", &debugRenderBones);
		if (debugRenderBones)
		{
			ImGui::DragFloat("Size", &debugRenderBonesSize, 0.01f, 0.01f, 3.0f);
			ImGui::ColorEdit4("Color", &debugRenderBonesColor.x);
			DebugBoneRender(debugRenderBonesColor, debugRenderBonesSize);
		}

		if (ImGui::Checkbox("RagDoll", &m_ragDoll)) {
			if (m_ragDoll) {
				SetRagDoll(m_ragDoll);
			}
		}
		if (m_ragDoll)
		{
			ImGui::Checkbox("Freeze", &m_ragDollFreeze);

			if (ImGui::TreeNode("RagDoll"))
			{
				for (int i = 0; i < m_boundEntities.size(); i++)
				{
					if (Editor::DragAndDropButton(m_boundEntities[i], "Bone: " + animator->m_names[i]))
					{
						auto entity = m_boundEntities[i].Get();
						SetRagDollBoundEntity(i, entity);
					}
				}
				ImGui::TreePop();
			}
		}
	}
}

void SkinnedMeshRenderer::Serialize(YAML::Emitter& out)
{
	out << YAML::Key << "m_forwardRendering" << m_forwardRendering;
	out << YAML::Key << "m_castShadow" << m_castShadow;
	out << YAML::Key << "m_receiveShadow" << m_receiveShadow;

	m_animator.Save("m_animator", out);
	m_skinnedMesh.Save("m_skinnedMesh", out);
	m_material.Save("m_material", out);

	out << YAML::Key << "m_ragDoll" << YAML::Value << m_ragDoll;
	out << YAML::Key << "m_ragDollFreeze" << YAML::Value << m_ragDollFreeze;

	if (!m_boundEntities.empty())
	{
		out << YAML::Key << "m_boundEntities" << YAML::Value << YAML::BeginSeq;
		for (int i = 0; i < m_boundEntities.size(); i++)
		{
			out << YAML::BeginMap;
			m_boundEntities[i].Serialize(out);
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
	}

	if (!m_ragDollTransformChain.empty())
	{
		out << YAML::Key << "m_ragDollTransformChain" << YAML::Value
			<< YAML::Binary(
				(const unsigned char*)m_ragDollTransformChain.data(),
				m_ragDollTransformChain.size() * sizeof(glm::mat4));
	}
}

void SkinnedMeshRenderer::Deserialize(const YAML::Node& in)
{
	m_forwardRendering = in["m_forwardRendering"].as<bool>();
	m_castShadow = in["m_castShadow"].as<bool>();
	m_receiveShadow = in["m_receiveShadow"].as<bool>();

	m_animator.Load("m_animator", in, GetScene());
	m_skinnedMesh.Load("m_skinnedMesh", in);
	m_material.Load("m_material", in);

	m_ragDoll = in["m_ragDoll"].as<bool>();
	m_ragDollFreeze = in["m_ragDollFreeze"].as<bool>();
	auto inBoundEntities = in["m_boundEntities"];
	if (inBoundEntities)
	{
		for (const auto& i : inBoundEntities)
		{
			EntityRef ref;
			ref.Deserialize(i);
			m_boundEntities.push_back(ref);
		}
	}

	if (in["m_ragDollTransformChain"])
	{
		YAML::Binary chains = in["m_ragDollTransformChain"].as<YAML::Binary>();
		m_ragDollTransformChain.resize(chains.size() / sizeof(glm::mat4));
		std::memcpy(m_ragDollTransformChain.data(), chains.data(), chains.size());
	}
}
void SkinnedMeshRenderer::OnCreate()
{
	m_finalResults = std::make_shared<BoneMatrices>();
	SetEnabled(true);
}
void SkinnedMeshRenderer::PostCloneAction(const std::shared_ptr<IPrivateComponent>& target)
{
}
void SkinnedMeshRenderer::Relink(const std::unordered_map<Handle, Handle>& map, const std::shared_ptr<Scene>& scene)
{
	m_animator.Relink(map, scene);
	for (auto& i : m_boundEntities)
	{
		i.Relink(map);
	}
}
void SkinnedMeshRenderer::CollectAssetRef(std::vector<AssetRef>& list)
{
	list.push_back(m_skinnedMesh);
	list.push_back(m_material);
}
bool SkinnedMeshRenderer::RagDoll() const
{
	return m_ragDoll;
}
void SkinnedMeshRenderer::SetRagDoll(bool value)
{
	auto animator = m_animator.Get<Animator>();
	if (value && !animator)
	{
		UNIENGINE_ERROR("Failed! No animator!");
		return;
	}
	m_ragDoll = value;
	if (m_ragDoll)
	{
		auto scene = GetScene();
		// Resize entities
		m_boundEntities.resize(animator->m_transformChain.size());
		// Copy current transform chain
		m_ragDollTransformChain = animator->m_transformChain;
		auto ltw = scene->GetDataComponent<GlobalTransform>(GetOwner()).m_value;
		for (auto& i : m_ragDollTransformChain) {
			i = ltw * i;
		}
	}
}
void SkinnedMeshRenderer::SetRagDollBoundEntity(int index, const Entity& entity, bool resetTransform)
{
	if (!m_ragDoll) {
		UNIENGINE_ERROR("Not ragdoll!");
		return;
	}
	if (index >= m_boundEntities.size()) {
		UNIENGINE_ERROR("Index exceeds limit!");
		return;
	}
	auto scene = GetScene();
	if (scene->IsEntityValid(entity))
	{
		if (resetTransform)
		{
			auto animator = m_animator.Get<Animator>();
			if (animator)
			{
				GlobalTransform globalTransform;
				globalTransform.m_value = m_ragDollTransformChain[index] * glm::inverse(animator->m_offsetMatrices[index]);
				scene->SetDataComponent(entity, globalTransform);
			}
		}
		m_boundEntities[index] = entity;
	}
}
void SkinnedMeshRenderer::SetRagDollBoundEntities(const std::vector<Entity>& entities, bool resetTransform)
{
	if (!m_ragDoll) {
		UNIENGINE_ERROR("Not ragdoll!");
		return;
	}
	for (int i = 0; i < entities.size(); i++) {
		SetRagDollBoundEntity(i, entities[i], resetTransform);
	}
}
size_t SkinnedMeshRenderer::GetRagDollBoneSize() const
{
	if (!m_ragDoll) {
		UNIENGINE_ERROR("Not ragdoll!");
		return 0;
	}
	return m_boundEntities.size();
}
void SkinnedMeshRenderer::OnDestroy()
{
	m_ragDollTransformChain.clear();
	m_boundEntities.clear();
	m_animator.Clear();
	m_finalResults.reset();
	m_skinnedMesh.Clear();
	m_material.Clear();
	m_ragDoll = false;
	m_ragDollFreeze = false;
	m_forwardRendering = false;
	m_castShadow = true;
	m_receiveShadow = true;
}

size_t& BoneMatrices::GetVersion()
{
	return m_version;
}

void BoneMatrices::Update()
{
	m_version++;
}

void BoneMatrices::UploadBones(const std::shared_ptr<SkinnedMesh>& skinnedMesh)
{
	skinnedMesh->UploadBones(m_value);
}
