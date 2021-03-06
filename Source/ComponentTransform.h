#pragma once
#include "Component.h"
#include "Dependencies/MathGeolib/MathGeoLib.h"

class __declspec(dllexport) ComponentTransform : public Component
{
public:
	ComponentTransform(GameObject* go);
	ComponentTransform(GameObject* go, float3 iTranslate, float3 iScale, Quat iRotation);
	~ComponentTransform();

	void SetLocalTransform(float3 iTranslate, float3 iScale, Quat iRotation);
	void NewParentLocal(GameObject* newParent);
	void GetLocalTransform(float3& translation, float3& scale, Quat& rotation) const;
	void SetFlag();
	const float4x4 GetGlobalTransform();
	void SetGlobalTransform(float4x4 transform);
	void Save(JSON_Array* componentsArry)const override;
	void Load(JSON_Object* componentObj) override;

private:
	void SetGlobalTransform();

	float3 translation;
	float3 scale;
	Quat rotation;
	float4x4 localTransform;
	float4x4 globalTransform;
	bool globalFlag;
};

