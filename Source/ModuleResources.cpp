#include "ModuleResources.h"
#include "Application.h"
#include "ModuleFileSystem.h"
#include "Importer.h"
#include "ResourceTexture.h"
#include "ResourceMesh.h"
#include "ModuleScene.h"
#include "ResourceModel.h"

bool ModuleResources::Start()
{
	return true;
}

update_status ModuleResources::Update(float dt)
{
	return update_status::UPDATE_CONTINUE;
}

bool ModuleResources::CleanUp()
{
	return true;
}

ModuleResources::ModuleResources(bool startEnabled) : Module(startEnabled)
{
}

unsigned int ModuleResources::ImportFile(const char* assetsFile, Resource::Type type, void*neededData)//pot ser que el asset file sigui el meta file?
{
	std::string libPath; std::string metaPath;
	Resource* resource = CreateNewResource(assetsFile, type, libPath, metaPath);
	unsigned int ret = 0;
	char* metaBuffer = nullptr;
	char* libBuffer = nullptr;
	unsigned int libSize = 0;
	unsigned int metaSize = 0;

	switch (resource->GetType()) {
	case Resource::Type::TEXTURE: Importer::Textures::Import(assetsFile, &libBuffer, libSize, &metaBuffer, metaSize, (ResourceTexture*)resource); break;
	//case Resource::Type::SCENE: Importer::Scenes::Import(fileBuffer, resource); break;
	//case Resource::Type::MESH: Importer::Meshes::Import(fileBuffer, resource); break;
	case Resource::Type::MODEL: Importer::Models::ImportFbx(assetsFile, &libBuffer, libSize, &metaBuffer, metaSize, (ResourceModel*)resource); break;
	}

	SaveResource(resource, libPath.c_str(), libBuffer, libSize, metaPath.c_str(), metaBuffer, metaSize);
	ret = resource->GetUID();
	if (metaBuffer != nullptr) {
		delete[] metaBuffer;
		metaBuffer = nullptr;
	}
	if (libBuffer != nullptr) {
		delete[] libBuffer;
		libBuffer = nullptr;
	}
	UnloadResource(resource->GetUID());
	return ret;
}

//Save the library and the meta files of a resource
void ModuleResources::SaveResource(Resource* resource, const char* libPath ,char* libBuffer, unsigned int libSize, const char* metaPath, char* metaBuffer, unsigned int metaSize) 
{
	if(libSize != 0)
		App->fileSystem->Save(libPath, libBuffer, libSize);
	if(metaSize != 0)
		App->fileSystem->Save(metaPath, metaBuffer, metaSize);
}

std::string ModuleResources::GetLibFilePath(Resource* resource)
{
	std::string path;
	switch (resource->GetType()) 
	{
	case Resource::Type::TEXTURE: path = TEXTURES_PATH; break;
	case Resource::Type::MESH: path = MESHES_PATH; break;
	case Resource::Type::MODEL: path = MODELS_PATH; break;
	}
	path += std::to_string(resource->GetUID());
	return path;
}

unsigned int ModuleResources::GenerateNewUID()
{
	//TODO: MD5 ID generation
	return App->scene->GetRandomInt();
}

Resource* ModuleResources::RequestResource(unsigned int uid, Resource::Type type)
{
	std::map<unsigned int, Resource*>::iterator it = resources.find(uid);
	if (it != resources.end())
	{
		if (it->second == nullptr) 
		{
			switch (type) {
			case Resource::Type::MESH: it->second = new ResourceMesh(it->first); break;
			case Resource::Type::MODEL: it->second = new ResourceModel(it->first); break;
			case Resource::Type::TEXTURE: it->second = new ResourceTexture(it->first); break;
			//case Resource::Type::SCENE: it->second = new ResourceScene(it->first); break;
			}

			it->second->LoadInMemory();
		}
		return it->second;
	}
	return nullptr;
}

void ModuleResources::UnrequestResource(unsigned int uid)
{
	std::map<unsigned int, Resource*>::iterator it = resources.find(uid);
	if (it != resources.end())
	{
		if (it->second != nullptr)
		{
			it->second->DecreaseReferenceCount();
			if (it->second->GetReferenceCount() == 0)
				UnloadResource(uid);
		}
	}
}

Resource* ModuleResources::GetResource(unsigned int uid)
{
	std::map<unsigned int, Resource*>::iterator it = resources.find(uid);
	if (it != resources.end())
	{
		return it->second;
	}
	return nullptr;
}

bool ModuleResources::ResourceExists(unsigned int uid)
{
	std::map<unsigned int, Resource*>::iterator it = resources.find(uid);
	if (it != resources.end())
		return true;

	return false;
}

void ModuleResources::UnloadResource(unsigned int uid)
{
	std::map<unsigned int, Resource*>::iterator it = resources.find(uid);
	if (it != resources.end()) 
	{
		delete it->second;
		it->second = nullptr;
	}
}

void ModuleResources::RemoveResource(unsigned int uid)
{
	std::map<unsigned int, Resource*>::iterator it = resources.find(uid);
	if (it != resources.end()) 
	{
		delete it->second;
		resources.erase(it);
	}

	//TODO: Delete the asset, library and meta files releted to the resource
}

void ModuleResources::AddMeshResource(unsigned int uid, Resource* resource)
{
	resources[uid] = resource;
	UnloadResource(resource->GetUID());
}

Resource* ModuleResources::CreateNewResource(const char* assetsPath, Resource::Type type, std::string& libPath, std::string& metaPath)
{
	Resource* ret = nullptr;
	unsigned int uid = GenerateNewUID();
	switch (type) {
	case Resource::Type::TEXTURE: ret = (Resource*) new ResourceTexture(uid); break;
	//case Resource::Type::MESH: ret = (Resource*) new ResourceMesh(uid); break;
	//case Resource::Type::SCENE: ret = (Resource*) new ResourceScene(uid); break;
	case Resource::Type::MODEL: ret = (Resource*) new ResourceModel(uid); break;
	}
	if (ret != nullptr)
	{
		resources[uid] = ret;
		//Creating the meta file path
		std::string fileName;
		App->fileSystem->SplitFilePath(assetsPath, &metaPath, &fileName);
		metaPath += fileName + ".meta";
		libPath = GetLibFilePath(ret);//->fer el save en el library file o nomes crear el file amb el nom del uid?
	}
	return ret;
}