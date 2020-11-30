#include "GameObjects.h"
#include "Component.h"
#include "ComponentMaterial.h"
#include "ComponentMesh.h"
#include "ComponentTransform.h"
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "Globals.h"

GameObject::GameObject() : parent(nullptr), name("No name")
{
}

GameObject::GameObject(GameObject * iParent, const char* iName, float3 transf, float3 scale, Quat rot) : parent(iParent), name(iName) 
{
	if (parent) 
	{
		parent->children.push_back(this);
	}
}

GameObject::~GameObject()
{
	std::vector<Component*>::iterator component = components.begin();
	for (component;component!= components.end();component++)	
		delete (*component);
	components.clear();
}

void GameObject::AddComponent(Component* c)
{
	//TODO:Check if we already have one of this type
	c->SetGameObject(*this);	
	components.emplace_back(c);
}

/*void GameObject::Start()
{
	std::vector<Component*>::iterator it = components.begin();
	for (it; it != components.end(); it++)
		(*it)->Start();
}*/

void GameObject::Update(float dt)
{
	std::vector<Component*>::iterator it = components.begin();
	for (it; it != components.end(); it++)
		(*it)->Update(dt);

	std::vector<GameObject*>::iterator itr = children.begin();
	for (itr; itr != children.end(); itr++) 
	{
		(*itr)->Update(dt);
		(*itr)->Draw();
	}
}

void GameObject::Draw()
{
	const ComponentTransform* transformComponent = (ComponentTransform*)GetComponent(ComponentType::Transform);
	const ComponentMesh* meshComponent = (ComponentMesh*)GetComponent(ComponentType::Mesh);

	if (meshComponent && transformComponent)
	{
		unsigned int mesh = 0;
		unsigned int material = 0;
		mesh = meshComponent->GetVAO();

		const ComponentMaterial* materialComponent = (ComponentMaterial*)GetComponent(ComponentType::Material);
		if (materialComponent)
			material = materialComponent->GetID();

		float3 pos, scale;
		Quat rot;

		((ComponentTransform*)this->GetComponent(ComponentType::Transform))->GetGlobalTransform().Decompose(pos,rot,scale);
		LOG("%s position x%f y%f z%f", name.c_str(), pos.x, pos.y, pos.z);
		LOG("%s rot x%f y%f z%f", name.c_str(), rot.x, rot.y, rot.z);

		

		App->renderer3D->Draw(transformComponent->GetGlobalTransform().Transposed(), mesh, meshComponent->GetNumIndices(), material);
	}
}

const GameObject* GameObject::GetParent() const
{
	return parent;
}

const Component* GameObject::GetComponent(ComponentType type) const
{
	std::vector<Component*>::const_iterator it = components.cbegin();
	for (it; it != components.cend(); ++it) {
		if ((*it)->GetType() == type)
			return (*it);
	}

	return nullptr;
}

bool GameObject::HasComponent(ComponentType type) const
{
	std::vector<Component*>::const_iterator it = components.cbegin();
	for (it; it != components.cend(); ++it) {
		if ((*it)->GetType() == type)
			return true;
	}

	return false;
}

const char* GameObject::GetName() const
{
	return name.c_str();
}

void GameObject::SetName(char* newName)
{
	name = newName;
}