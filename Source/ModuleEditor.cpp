#include "Globals.h"
#include "GL/glew.h"
#include "Application.h"
#include "Globals.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"
#include "ModuleCamera3D.h"
#include "ModuleEditor.h"
#include "ModuleFileSystem.h"
#include "Primitive.h"
#include "ModuleScene.h"
#include "imgui.h"
#include "Dependencies/ImGuizmo-master/ImGuizmo.h"
#include "examples\imgui_impl_sdl.h"
#include "examples\imgui_impl_opengl3.h"
#include "Importer.h"
#include "GameObjects.h"
#include "Component.h"
#include "ComponentMesh.h"
#include "ComponentTransform.h"
#include "ComponentMaterial.h"
#include "ComponentCamera.h"
#include "ModuleResources.h"
#include "ResourceMesh.h"
#include "ResourceTexture.h"
#include "parson.h"
#include "ModuleInput.h"
#include <algorithm>
#include <fstream>
#include "ComponentScript.h"


ModuleEditor::ModuleEditor(bool startEnable) : Module(startEnable),
aboutWindow(false), configWindow(false), consoleWindow(true), inspectorWindow(true), hierarchyWindow(true), demoWindow(false), dockingWindow(true), projectWindow(true), sceneWindow(false), timeWindow(true), resourcesWindow(true), editorWindow(false),
component(0), removeMaterial(true), removeMesh(true), removeCamera(true), removeScript(true), changeTexture(false), changeMesh(false), addedTexture(false), addedMesh(false), loadScene(false),
org("CITM"), scroll(true), selectedFolder(ASSETS_PATH)
{
}

ModuleEditor::~ModuleEditor()
{
}

bool ModuleEditor::Init()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;
	io.ConfigDockingWithShift = false;

	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();
	//ImGui::StyleColorsLight();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Platform/Renderer bindings
	ImGui_ImplSDL2_InitForOpenGL(App->window->window, App->renderer3D->context);
	ImGui_ImplOpenGL3_Init("#version 460"); //TODO: glsl version �?�?

	TextEditorInit(scriptFile.c_str());

	//Start Timers
	return true;
}

update_status ModuleEditor::Update(float dt)
{

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(App->window->window);
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	WindowDocking();
	WindowTime();
	WindowScene();
	WindowAbout();
	WindowConfig();
	WindowConsole();
	WindowInspector();
	WindowHierarchy();
	WindowProject();
	WindowResourcesCount();
	WindowLoadScene();
	WindowDemo();
	
	TextEditorWindow();

	ImGui::Render();
	ImGuiIO& io = ImGui::GetIO();
	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	//glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
	//glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// Update and Render additional Platform Windows
	// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
	//  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
		SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
	}

	return UPDATE_CONTINUE;
}

bool ModuleEditor::CleanUp()
{
	return true;
}

void ModuleEditor::WindowDocking()
{

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar;
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	if (ImGui::Begin("DockSpace", &dockingWindow, window_flags)) {
		// DockSpace
		ImGui::PopStyleVar(3);
		if (dockingWindow)
		{
			ImGuiID dockspace_id = ImGui::GetID("DockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
			MenuBar();
		}

		//TODO: Don't show guizmos during game
		if(!App->IsGameRunning())
			ImGuizmoUpdate();

		ImGui::End();
	}

}

void ModuleEditor::MenuBar()
{
	std::string menuAction = "";

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New Scene")) {
				menuAction = "New";
			}
			if (ImGui::MenuItem("Load Scene", "")) {
				loadScene = true;
			}
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			if (ImGui::MenuItem("Save Scene", "")) {
				App->scene->SaveScene();
			}
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			if (ImGui::MenuItem("Exit", "ESC")) {
				App->closeApp = true;
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit")) {
			if (ImGui::MenuItem("Project Settings...", NULL, configWindow)) {
				configWindow = !configWindow;
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Create")) {
			if (ImGui::BeginMenu("GameObject"))
			{
				if (ImGui::MenuItem("Empty")) {
					App->scene->CreateGameObject("Empty", App->scene->GetRootObject());
					LOG("An Empty Game Object has been created");
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Camera"))
			{
				if (ImGui::MenuItem("Camera")) {
					App->scene->CreateCamera("Camera");
					LOG("Camera has been created");
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Primitives")) {
				if (ImGui::MenuItem("Cube")) {
					
				}
				if (ImGui::MenuItem("Sphere")) {

				}
				if (ImGui::MenuItem("Piramid")) {

				}
				if (ImGui::MenuItem("Cylinder")) {

				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Script"))
			{
				if (ImGui::MenuItem("New Script")) {
					menuAction = "Script";
					LOG("A Script has been created");
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Window"))
		{
			if (ImGui::BeginMenu("Layouts", NULL)) {

				ImGui::EndMenu();
			}
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			if (ImGui::BeginMenu("General")) {
				if (ImGui::MenuItem("Scene", NULL, sceneWindow)) {
					sceneWindow = !sceneWindow;
				}
				if (ImGui::MenuItem("Insperctor", NULL, inspectorWindow)) {
					inspectorWindow = !inspectorWindow;
				}
				if (ImGui::MenuItem("Hierarchy", NULL, hierarchyWindow)) {
					hierarchyWindow = !hierarchyWindow;
				}
				if (ImGui::MenuItem("Console", NULL, consoleWindow)) {
					consoleWindow = !consoleWindow;
				}
				if (ImGui::MenuItem("Assets", NULL, projectWindow)) {
					projectWindow = !projectWindow;
				}
				if (ImGui::MenuItem("Resources", NULL, resourcesWindow)){
					resourcesWindow = !resourcesWindow;
				}
				ImGui::EndMenu();
			}
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			if (ImGui::MenuItem("Demo", NULL, demoWindow)) {
				demoWindow = !demoWindow;
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help")) {
			if (ImGui::MenuItem("About Legacy Engine")) {
				aboutWindow = !aboutWindow;
			}
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			if (ImGui::MenuItem("Download Latest")) {
				ShellExecuteA(NULL, "open", "https://github.com/M1R4B3L/LegacyEngine/releases", NULL, NULL, 3);
			}
			if (ImGui::MenuItem("Report a bug")) {
				ShellExecuteA(NULL, "open", "https://github.com/M1R4B3L/LegacyEngine/issues", NULL, NULL, 3);
			}
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			if (ImGui::MenuItem("Documentation")) {
				ShellExecuteA(NULL, "open", "https://github.com/M1R4B3L/LegacyEngine", NULL, NULL, 3);
			}

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	if(menuAction == "New") ImGui::OpenPopup("CreateNewScene");
	if (menuAction == "Script") ImGui::OpenPopup("CreateNewScript");

	if (ImGui::BeginPopupModal("CreateNewScene"))
	{
		static std::string name = "";
		ImGui::Text("All scene unsaved changes will be lost");
		ImGui::Text("Are you sure?");
		ImGui::InputText("Name", (char*)name.c_str(), 50);
		if (ImGui::Button("Yes"))
		{
			App->scene->GenerateScene(name.c_str());
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::Button("No"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopupModal("CreateNewScript"))
	{
		static char scriptName[64] = "";
		ImGui::InputText("Change", scriptName, 64);	
		ImGui::Text("Script Name:");
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0, 1, 0, 1), scriptName);
		ImGui::Spacing();
		if(ImGui::Button("Create"))
		{
			if (scriptName == "")
			{
				LOG("Need an Script name");
			}
			else
			{
				//std::string name = scriptName;
				App->resources->CreateScriptResource(scriptName);
			}
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}
		
		//TODO scriptName already exist in the folder
		ImGui::EndPopup();
	}
}

void ModuleEditor::WindowLoadScene()
{
	if (loadScene)
	{
		if (ImGui::Begin("Load Scene", &loadScene))
		{
			std::vector<std::string> files;
			std::vector<std::string> dirs;

			//std::string dir((ASSETS_SCENES) ? ASSETS_SCENES : "");
			//dir += "/";
			std::string dir(ASSETS_SCENES);

			App->fileSystem->DiscoverFiles(dir.c_str(), files, dirs);

			std::sort(files.begin(), files.end());

			static std::string sceneName;
			for (std::vector<std::string>::const_iterator itr = files.begin(); itr != files.end(); ++itr)
			{

				if (ImGui::Button((*itr).c_str()))
				{
					sceneName = (*itr);
					ImGui::OpenPopup("Warning");
				}
			}
			if (ImGui::BeginPopupModal("Warning"))
			{
				ImGui::Text("All scene unsaved changes will be lost");
				ImGui::Text("Are you sure?");
				if (ImGui::Button("Yes"))
				{
					char* buffer = nullptr;
					std::string path = ASSETS_SCENES;
					path += sceneName.c_str();
					App->fileSystem->Load(path.c_str(), &buffer);
					JSON_Value* rootValue = json_parse_string(buffer);
					JSON_Object* node = json_value_get_object(rootValue);
					unsigned int uid = json_object_get_number(node, "LIBUID");
					loadScene = false;
					json_value_free(rootValue);
					delete[] buffer;
					App->scene->LoadScene(uid);
					LOG("Successful Loaded scene: %s", sceneName.c_str());
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::Button("No"))
				{
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}
		ImGui::End();
	}
}

void ModuleEditor::WindowAbout()
{
	if (aboutWindow) {
		if (ImGui::Begin("About", &aboutWindow))
		{
			ImGui::BulletText("Legacy Engine:");
			ImGui::Separator();
			ImGui::Text("Developed by Jordi Bach & Adrian Mirabel,\nfor engine subject in CITM Barcelona.");
			ImGui::Separator();
			ImGui::Spacing();
			ImGui::BulletText("Libraries:");
			ImGui::Separator();
			ImGui::Text("STL Wiki");
			ImGui::SameLine();
			if (ImGui::Button("STL")) {
				ShellExecuteA(NULL, "open", "https://www.geeksforgeeks.org/the-c-standard-template-library-stl/?ref=lbp", NULL, NULL, 3);
			}
			ImGui::Spacing();
			ImGui::Text("SDL v2.0.12");
			ImGui::SameLine();
			if (ImGui::Button("SDL")) {
				ShellExecuteA(NULL, "open", "https://www.libsdl.org/download-2.0.php", NULL, NULL, 3);
			}
			ImGui::Spacing();
			ImGui::Text("OpenGL v3.1.0");
			ImGui::SameLine();
			if (ImGui::Button("OpenGL")) {
				ShellExecuteA(NULL, "open", "https://www.opengl.org/", NULL, NULL, 3);
			}
			ImGui::Spacing();
			ImGui::Text("Glew v2.1.0");
			ImGui::SameLine();
			if (ImGui::Button("Glew")) {
				ShellExecuteA(NULL, "open", "http://glew.sourceforge.net/", NULL, NULL, 3);
			}
			ImGui::Spacing();
			ImGui::Text("ImGui v1.79");
			ImGui::SameLine();
			if (ImGui::Button("ImGui")) {
				ShellExecuteA(NULL, "open", "https://github.com/ocornut/imgui", NULL, NULL, 3);
			}
			ImGui::Spacing();
			ImGui::Text("Assimp v3.1.1");
			ImGui::SameLine();
			if (ImGui::Button("Assimp")) {
				ShellExecuteA(NULL, "open", "https://github.com/assimp/assimp", NULL, NULL, 3);
			}
			ImGui::Spacing();
			ImGui::Text("Devil v1.8.0");
			ImGui::SameLine();
			if (ImGui::Button("Devil")) {
				ShellExecuteA(NULL, "open", "http://openil.sourceforge.net/download.php", NULL, NULL, 3);
			}
			ImGui::Spacing();
			ImGui::Text("MathGeoLib v1.5");
			ImGui::SameLine();
			if (ImGui::Button("MathGeoLib")) {
				ShellExecuteA(NULL, "open", "https://github.com/juj/MathGeoLib", NULL, NULL, 3);
			}
			ImGui::Spacing();
			ImGui::Text("JSON Parson v1.1.0");
			ImGui::SameLine();
			if (ImGui::Button("JSON Parson")) {
				ShellExecuteA(NULL, "open", "https://github.com/kgabis/parson", NULL, NULL, 3);
			}
			ImGui::Spacing();
			ImGui::Text("PHYSFS v3.0.2");
			ImGui::SameLine();
			if (ImGui::Button("PHYSFS")) {
				ShellExecuteA(NULL, "open", "https://icculus.org/physfs/", NULL, NULL, 3);
			}
			ImGui::Spacing();
			ImGui::Text("ImGuizmo");
			ImGui::SameLine();
			if (ImGui::Button("ImGuizmo")) {
				ShellExecuteA(NULL, "open", "https://github.com/CedricGuillemet/ImGuizmo", NULL, NULL, 3);
			}
			ImGui::Spacing();
			ImGui::Text("ImGuiColorTextEdit");
			ImGui::SameLine();
			if (ImGui::Button("ImGuiColorTextEdit")) {
				ShellExecuteA(NULL, "open", "https://github.com/BalazsJako/ImGuiColorTextEdit", NULL, NULL, 3);
			}
			ImGui::Spacing();
			ImGui::Text("RCC++");
			ImGui::SameLine();
			if (ImGui::Button("RCC++")) {
				ShellExecuteA(NULL, "open", "https://github.com/RuntimeCompiledCPlusPlus/RuntimeCompiledCPlusPlus", NULL, NULL, 3);
			}
			if (ImGui::CollapsingHeader("License"))
			{
				ImGui::Text("MIT License\n"
					"Copyright(c) 2020 Legacy Engine\n"
					"Permission is hereby granted, free of charge, to any person obtaining a copy\n"
					"of this softwareand associated documentation files(the ""Software""), to deal\n"
					"in the Software without restriction, including without limitation the rights\n"
					"to use, copy, modify, merge, publish, distribute, sublicense, and /or sell\n"
					"copies of the Software, and to permit persons to whom the Software is\n"
					"furnished to do so, subject to the following conditions :\n"
					"The above copyright noticeand this permission notice shall be included in all\n"
					"copies or substantial portions of the Software.\n"
					"THE SOFTWARE IS PROVIDED ""AS IS"", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
					"IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
					"FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE\n"
					"AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
					"LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n"
					"OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n"
					"SOFTWARE.");
			}
			//libraries, 
		}
		ImGui::End();
	}
}

void ModuleEditor::WindowConfig()
{
	if (configWindow) {
		
		if (ImGui::Begin("Configuration", &configWindow, ImGuiWindowFlags_MenuBar)) {

			ImGui::BeginMenuBar();
			if (ImGui::MenuItem("Save"))
			{

			}
			if (ImGui::MenuItem("Reset"))
			{

			}
			if (ImGui::MenuItem("SetAsDefault"))
			{

			}
			ImGui::EndMenuBar();

			ImGui::Spacing();
			if (ImGui::BeginTabBar(""))
			{
				if (ImGui::BeginTabItem("Display")) {
					if (ImGui::CollapsingHeader("Window", ImGuiTreeNodeFlags_DefaultOpen))
					{

						static float brightness = App->window->GetBrightness();
						ImGui::Text("Brightness: ");
						ImGui::SameLine();
						ImGui::TextColored(ImVec4(255, 0, 255, 255), "%.3f", brightness);
						if (ImGui::SliderFloat("Brightness", &brightness, 0, 1, "%.3f")) {
							App->window->SetBrightness(brightness);
						}
						//static float gamma = App->window->GetGamma();

						static int width = App->window->GetSize().x;
						static int height = App->window->GetSize().y;
						ImGui::Separator();
						ImGui::Text("Windows Size: You should not use this");
						ImGui::Text("Width ");
						ImGui::SameLine();
						ImGui::TextColored(ImVec4(255, 0, 255, 255), "%d ", width);
						ImGui::Text("Height ");
						ImGui::SameLine();
						ImGui::TextColored(ImVec4(255, 0, 255, 255), "%d ", height);
						ImGui::SliderInt("Width", &width, 640, 3840);
						ImGui::SliderInt("Height", &height, 480, 2160);
						App->window->SetSize(width, height);
						ImGui::Separator();
						ImGui::Text("Refresh date");
						ImGui::SameLine();
						ImGui::TextColored(ImVec4(255, 0, 255, 255), "%d", App->window->GetRefreshRate());

						static bool fullscreen = App->window->GetFullscreen();
						static bool resizable = App->window->GetResizable();
						static bool bordeless = App->window->GetBorderless();
						static bool fulldesktop = App->window->GetFullscreenDesktop();

						if (ImGui::Checkbox("Fullscreen", &fullscreen))
						{
							App->window->SetFullscreen(fullscreen);
						}
						ImGui::SameLine();
						if (ImGui::Checkbox("Resizable", &resizable))
						{
							App->window->SetResizable(resizable);
						}

						if (ImGui::Checkbox("Borderless", &bordeless))
						{
							App->window->SetBorderless(bordeless);
						}
						ImGui::SameLine();
						if (ImGui::Checkbox("Full Desktop", &fulldesktop))
						{
							App->window->SetFullscreenDesktop(fulldesktop);
						}
					}
					if (ImGui::CollapsingHeader("Application"))
					{
						static char name[128];
						strcpy_s(name, 128, App->GetEngineTitle());
						if (ImGui::InputText("App Name", name, 128, ImGuiInputTextFlags_EnterReturnsTrue))  //We can use ImGuiInputTextFlags_EnterReturnsTrue only if we want to change the name if enter is pressed
							App->SetEngineTitle(name);

						ImGui::InputText("Organization", org, 128);

						ImGui::Separator();
						ImGui::Spacing();

						int max_fps = App->GetFramerate();
						if (ImGui::SliderInt("Max FPS", &max_fps, 0, 144))
							App->CapFramerate(max_fps);

						ImGui::Text("Limit Framerate:");
						ImGui::SameLine();
						ImGui::TextColored(ImVec4(0, 255, 255, 255), "%u", App->GetFramerate());

						char title[25];
						sprintf_s(title, 25, "Framerate %.1f", App->fpsLog[App->fpsLog.size() - 1]);
						ImGui::PlotHistogram("##framerate", &App->fpsLog[0], App->fpsLog.size(), 0, title, 0.0f, 100.0f, ImVec2(310, 100));
						sprintf_s(title, 25, "Milliseconds %.1f", App->msLog[App->msLog.size() - 1]);
						ImGui::PlotHistogram("##milliseconds", &App->msLog[0], App->msLog.size(), 0, title, 0.0f, 50.0f, ImVec2(310, 100));

						//Falta memory pero hem de possar la llibreria del Ric
					}
					ImGui::EndTabItem();

				}

				if (ImGui::BeginTabItem("Render"))
				{

					ImGui::Text("Renderer Options");
					ImGui::Separator();
					ImGui::Spacing();
					bool depth_test = App->renderer3D->GetglEnableFlags(GL_DEPTH_TEST);
					bool cull_face = App->renderer3D->GetglEnableFlags(GL_CULL_FACE);
					bool lightning = App->renderer3D->GetglEnableFlags(GL_LIGHTING);
					bool color_material = App->renderer3D->GetglEnableFlags(GL_COLOR_MATERIAL);
					bool texture2D = App->renderer3D->GetglEnableFlags(GL_TEXTURE_2D);
					bool show_wireframes = App->renderer3D->GetWireframes();
					ImGui::BulletText("General");
					if (ImGui::Checkbox("Depth", &depth_test))
					{
						App->renderer3D->SetglEnbleFlags(GL_DEPTH_TEST, depth_test);
					}
					ImGui::SameLine();
					if (ImGui::Checkbox("Cull Face", &cull_face))
					{
						App->renderer3D->SetglEnbleFlags(GL_CULL_FACE, cull_face);
					}
					ImGui::SameLine();
					if (ImGui::Checkbox("Lightning", &lightning))
					{
						App->renderer3D->SetglEnbleFlags(GL_LIGHTING, lightning);
					}
					ImGui::BulletText("Polygon Mode");
					if (ImGui::Checkbox("Wireframes", &show_wireframes))
					{
						App->renderer3D->SetWireframes(show_wireframes);
					}
					ImGui::BulletText("Material");
					if (ImGui::Checkbox("Color Material", &color_material))
					{
						App->renderer3D->SetglEnbleFlags(GL_COLOR_MATERIAL, color_material);
					}
					ImGui::BulletText("Texture");
					if (ImGui::Checkbox("Texture 2D", &texture2D))
					{
						App->renderer3D->SetglEnbleFlags(GL_TEXTURE_2D, texture2D);
					}
					ImGui::Separator();
					ImGui::Spacing();
					ImGui::Text("Outliner:");
					float outlineWidth = App->renderer3D->outlineWeight;
					if (ImGui::DragFloat("Outline Width", (float*)&outlineWidth, 0.001f, 0.01f, 0.1f))
					{
						App->renderer3D->outlineWeight = outlineWidth;
					}

					ImVec4 colorOutline = ImVec4(App->renderer3D->outlineColor.x, App->renderer3D->outlineColor.y, App->renderer3D->outlineColor.z, App->renderer3D->outlineColor.w);
					if (ImGui::ColorEdit4("Color", &colorOutline.x))
					{
						App->renderer3D->outlineColor = float4(colorOutline.x, colorOutline.y, colorOutline.z, colorOutline.w);
					}
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Controls"))
				{
					if (ImGui::CollapsingHeader("Input", ImGuiTreeNodeFlags_DefaultOpen))
					{

					}
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Software"))
				{
					static Hardware stats;
					SDL_GetVersion(&stats.hardwareSDLVersion);
					stats.hardwareCPU = SDL_GetCPUCount();
					stats.hardwareCPUCache = SDL_GetCPUCacheLineSize();
					stats.hardwareSystemRAM = (float)SDL_GetSystemRAM() / 1024.f; //This is returned in Mb thats why we divide
					stats.hardwareRdtsc = SDL_HasRDTSC() == SDL_TRUE;
					stats.hardware3dnow = SDL_Has3DNow() == SDL_TRUE;
					stats.hardwareAltivec = SDL_HasAltiVec() == SDL_TRUE;
					stats.hardwareAvx = SDL_HasAVX() == SDL_TRUE;
					stats.hardwareAvx2 = SDL_HasAVX2() == SDL_TRUE;
					stats.hardwareMmx = SDL_HasMMX() == SDL_TRUE;
					stats.hardwareSse = SDL_HasSSE() == SDL_TRUE;
					stats.hardwareSse2 = SDL_HasSSE2() == SDL_TRUE;
					stats.hardwareSse3 = SDL_HasSSE3() == SDL_TRUE;
					stats.hardwareSse41 = SDL_HasSSE41() == SDL_TRUE;
					stats.hardwareSse42 = SDL_HasSSE42() == SDL_TRUE;

					ImGui::Text("SDL Version:");
					ImGui::SameLine();
					ImGui::TextColored(ImVec4(0, 255, 0, 255), "%d.%d.%d", stats.hardwareSDLVersion.major, stats.hardwareSDLVersion.minor, stats.hardwareSDLVersion.patch);
					ImGui::Spacing();
					ImGui::Text("OpenGL Version:");
					ImGui::SameLine();
					ImGui::TextColored(ImVec4(0, 255, 0, 255), "%s", glGetString(GL_VERSION));
					ImGui::Separator();
					ImGui::Spacing();
					ImGui::Text("CPUs:");
					ImGui::SameLine();
					ImGui::TextColored(ImVec4(0, 255, 0, 255), "%u (Cache: %ukb)", stats.hardwareCPU, stats.hardwareCPUCache);
					ImGui::Text("System RAM:");
					ImGui::SameLine();
					ImGui::TextColored(ImVec4(0, 255, 0, 255), "%.2fGb", stats.hardwareSystemRAM);
					ImGui::Text("Features:");
					ImGui::SameLine();
					ImGui::TextColored(ImVec4(0, 255, 0, 255), "%s%s%s%s%s",
						stats.hardwareRdtsc ? "RDTSC " : "",
						stats.hardware3dnow ? "3DNOW " : "",
						stats.hardwareAltivec ? "ALTIVEC " : "",
						stats.hardwareAvx ? "AVX " : "",
						stats.hardwareAvx2 ? "AVX2 " : ""
					);
					ImGui::TextColored(ImVec4(0, 255, 0, 255), "%s%s%s%s%s%s",
						stats.hardwareMmx ? "MMX " : "",
						stats.hardwareSse ? "SSE " : "",
						stats.hardwareSse2 ? "SSE2 " : "",
						stats.hardwareSse3 ? "SSE3 " : "",
						stats.hardwareSse41 ? "SSE41 " : "",
						stats.hardwareSse42 ? "SSE42 " : ""
					);
					ImGui::Separator();
					ImGui::Spacing();
					ImGui::Text("GPU:");
					ImGui::SameLine();
					ImGui::TextColored(ImVec4(0, 255, 0, 255), "");
					ImGui::Text("Brand:");
					ImGui::SameLine();
					ImGui::TextColored(ImVec4(0, 255, 0, 255), "");
					ImGui::Text("VRAM Budget:");
					ImGui::SameLine();
					ImGui::TextColored(ImVec4(0, 255, 0, 255), "");
					ImGui::Text("VRAM Usage:");
					ImGui::SameLine();
					ImGui::TextColored(ImVec4(0, 255, 0, 255), "");
					ImGui::Text("VRAM Available:");
					ImGui::SameLine();
					ImGui::TextColored(ImVec4(0, 255, 0, 255), "");
					ImGui::Text("VRAM Reserved:");
					ImGui::SameLine();
					ImGui::TextColored(ImVec4(0, 255, 0, 255), "");

					ImGui::EndTabItem();
				}
			
				ImGui::EndTabBar();
			}
			
		}
		ImGui::End();
	}

}

void ModuleEditor::WindowConsole()
{
	if (consoleWindow)
	{
		if (ImGui::Begin("Console", &consoleWindow)) {


			for (int i = 0; i < stringLog.size(); ++i)
			{
				ImGui::TextUnformatted(stringLog[i]);
			}
			if (scroll == true)
			{
				ImGui::SetScrollHere(1.0f);

				scroll = false;
			}
			
		}
		ImGui::End();
	}
}

void ModuleEditor::WindowInspector()
{
	if (inspectorWindow)
	{
		if (ImGui::Begin("Inspector", &inspectorWindow))
		{
			if (App->scene->GetSelectedObject() != nullptr && (App->scene->GetSelectedObject() != App->scene->GetRootObject())) {
				InspectorComponents(App->scene->GetSelectedObject());

				ImGui::Separator();
				
				ImGui::Combo("##Select", &component, "Add Component\0Transform\0Mesh\0Material\0Camera\0");
				ImGui::SameLine();

				Component* tmp = nullptr;

				if (ImGui::Button("Add"))
				{
					switch (component)
					{
						case 0:
						{
							LOG("This is not a valid component");
							break;
						}
						case 1:
						{
							if (!App->scene->GetSelectedObject()->HasComponent(ComponentType::Transform))
							{
								tmp = new ComponentTransform(App->scene->GetSelectedObject());
								LOG("Successful added a Component Transform");
							}
							else
							{
								LOG("This Game Object already has a Component Transform");
							}
							break;
						}
						case 2:
						{
							if (!App->scene->GetSelectedObject()->HasComponent(ComponentType::Mesh) && App->scene->GetSelectedObject()->HasComponent(ComponentType::Transform))
							{
								addedMesh = true;
								LOG("Select the mesh to add");
							}
							else
							{
								LOG("This Game Object already has a Component Mesh or doesn't have transform");
							}
							break;
						}
						case 3:
						{
							if (!App->scene->GetSelectedObject()->HasComponent(ComponentType::Material) && App->scene->GetSelectedObject()->HasComponent(ComponentType::Mesh))
							{
								addedTexture = true;
								LOG("Select the texture to add");
							}
							else
							{
								LOG("This Game Object already has a Component Material or doesn't have a mesh");
							}
							break;
						}
						case 4:
						{
							if (!App->scene->GetSelectedObject()->HasComponent(ComponentType::Camera))
							{
								tmp = new ComponentCamera();
								App->scene->GetSelectedObject()->AddComponent(tmp);
								LOG("Successful added a Component Camera");
							}
							else
							{
								LOG("This Game Object already has a Component Camera");
							}
							break;
						}

					}
				}

				if (addedMesh == true)
				{
					AddMeshWindow(ASSETS_MESHES);
				}
				else if(addedTexture == true)
				{
					AddTextureWindow(ASSETS_TEXTURES);
				}
				ImGui::Spacing();
				ImGui::Separator();
				if (ImGui::Button("Add Script"))
				{
					ImGui::OpenPopup("Script");
				}
				if (ImGui::BeginPopup("Script"))
				{
					std::vector<std::string> files;
					std::vector<std::string> dirs;

					std::string dir = ASSETS_SCRIPTS;
					dir += "/";

					std::vector<std::string>::const_iterator it;

					App->fileSystem->DiscoverFiles(dir.c_str(), files, dirs);

					std::sort(files.begin(), files.end());

					for (std::vector<std::string>::const_iterator it2 = files.begin(); it2 != files.end(); ++it2)
					{
						if (strstr((*it2).c_str(), ".meta") == nullptr)
						{
							if (ImGui::Selectable((*it2).c_str()))
							{
								std::string metaPath;
								std::string fileName;
								std::string extension;
								App->fileSystem->SplitFilePath((*it2).c_str(), &metaPath, &fileName, &extension);
								GameObject* object = App->scene->GetSelectedObject();
								if (object != nullptr && !object->HasScript(fileName.c_str()))
								{
										metaPath += ASSETS_SCRIPTS + fileName + ".meta";
										char* buffer = nullptr;
										App->fileSystem->Load(metaPath.c_str(), &buffer);
										JSON_Value* rootValue = json_parse_string(buffer);
										JSON_Object* node = json_value_get_object(rootValue);
										unsigned int uid = json_object_get_number(node, "LIBUID");
										delete[] buffer; buffer = nullptr;
										json_value_free(rootValue);

										Component* scriptComponent = new ComponentScript(uid);
										object->AddComponent(scriptComponent);
								}
								else 
									LOG("Error adding script: Game Object already has this script")
							}
						}
					}
					ImGui::EndPopup();
				}
			}
	
		}
		ImGui::End();
	}
}

void ModuleEditor::WindowHierarchy()
{
	if (hierarchyWindow)
	{
		
		if (ImGui::Begin("Hierarchy", &hierarchyWindow))
		{
			if (App->scene->GetRootObject() != nullptr) {

				HierarchyNodes(App->scene->GetRootObject());

			}	
		}
		ImGui::End();
	}
}

void ModuleEditor::InspectorComponents(GameObject* selectedGameObject)
{

	ImGui::Checkbox("##", &selectedGameObject->activeGameObject);
	ImGui::SameLine();
	static char name[128];
	strcpy_s(name, 128, selectedGameObject->GetName());

	ImGuiInputTextFlags flag = ImGuiInputTextFlags_EnterReturnsTrue;

	if (ImGui::InputText("Name", name, 128, flag))
		selectedGameObject->SetName(name);

	for (uint i = 0; i < selectedGameObject->components.size(); i++) {

		Component* currentComponent = selectedGameObject->components[i];

		if (currentComponent != nullptr)
		{
			switch (currentComponent->GetType())
			{
				case ComponentType::Transform:
				{
					InspectorShowTransform((ComponentTransform*)currentComponent);
				} break;

				case ComponentType::Mesh: 
				{
					InspectorShowMesh((ComponentMesh*)currentComponent);
				} break;

				case ComponentType::Material: 
				{
					InspectorShowMaterial((ComponentMaterial*)currentComponent);
				} break;

				case ComponentType::Camera:
				{
					InspectorShowCamera((ComponentCamera*)currentComponent);
				} break;
				case ComponentType::Script: 
				{
					InspectorShowScript((ComponentScript*)currentComponent);
				} break;
			}

			if (currentComponent->GetType() == ComponentType::Unknown)
			{
				LOG("Selected Object %s has invalid component: %s", selectedGameObject->GetName(), currentComponent->GetType());
			}
		}
	}

}

void ModuleEditor::InspectorShowTransform(ComponentTransform* componentTransform)
{

	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		float3 translation, scale;
		Quat rotation;

		componentTransform->GetLocalTransform(translation, scale, rotation);

		if (ImGui::DragFloat3("Position", (float*)&translation, 0.01f, 0.0f, 0.0f, "%.3f")) 
			componentTransform->SetLocalTransform(translation, scale, rotation);


		float3 eulerRotation = rotation.ToEulerXYZ();
		eulerRotation *= (180 / pi);
		if (ImGui::DragFloat3("Rotation", (float*)&eulerRotation, 0.01f, 0.0f, 0.0f, "%.3f")) {

			eulerRotation = (eulerRotation/180) * pi;
			rotation = Quat::FromEulerXYZ(eulerRotation.x,eulerRotation.y,eulerRotation.z);
			componentTransform->SetLocalTransform(translation, scale, rotation);
		}

		if (ImGui::DragFloat3("Scale", (float*)&scale, 0.01f, 0.01f, NULL, "%.3f")) {

			//TODO: Assert Scale < 0.01f
			if (scale.x < 0.01f)
				scale.x = 0.01f;
			if (scale.y < 0.01f)
				scale.y = 0.01f;
			if (scale.z < 0.01f)
				scale.z = 0.01f;
			componentTransform->SetLocalTransform(translation, scale, rotation);
		}
	
	}
}

void ModuleEditor::InspectorShowMesh(ComponentMesh* componentMesh)
{

	//ImGui::Checkbox("##Mesh", &((ComponentMesh*)componentMesh)->activeMesh);

	//ImGui::SameLine();
	if (ImGui::CollapsingHeader("Mesh", &removeMesh, ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Vertex:");
		ImGui::SameLine();
		
		ImGui::TextColored(ImVec4(0, 255, 0, 255), "%u", componentMesh->GetResource()->numVertices);

		ImGui::Text("Triangles:");
		ImGui::Separator();

		ImGui::Checkbox("AABB",&App->scene->GetSelectedObject()->showAABB);
		ImGui::SameLine();
		ImGui::Checkbox("OBB", &App->scene->GetSelectedObject()->showOBB);

		ImGui::Separator();

		if (ImGui::Button("Change Mesh")) 
			changeMesh = true;


		ChangeMeshWindow(ASSETS_MESHES, componentMesh);
	}
	
	if (removeMesh == false)
	{
		ImGui::OpenPopup("Are you sure?");

		if (ImGui::BeginPopupModal("Are you sure?"))
		{
			if(ImGui::Button("Delete", ImVec2(80,0)))
			{
				App->scene->GetSelectedObject()->RemoveComponent(componentMesh);
				LOG("Removed Component Mesh");
				removeMesh = true;
			}
			
			if (ImGui::Button("Cancel", ImVec2(80, 0)))
			{
				ImGui::CloseCurrentPopup();
				removeMesh = true;
			}
			ImGui::EndPopup();
		}
	}

}

void ModuleEditor::InspectorShowMaterial(ComponentMaterial* componentMaterial)
{

	if (ImGui::CollapsingHeader("Material", &removeMaterial, ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Texture Size ");
		//ImGui::SameLine();
		//ImGui::TextColored(ImVec4(0, 255, 0, 255), "%u x %u");

		if (ImGui::ImageButton((ImTextureID)componentMaterial->GetResource()->gpuID, ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0)))
			changeTexture = true;

		ChangeTextureWindow(ASSETS_TEXTURES, componentMaterial);
	}

	if (removeMaterial == false)
	{
		ImGui::OpenPopup("Are you sure?");

		if (ImGui::BeginPopupModal("Are you sure?"))
		{
			if (ImGui::Button("Delete", ImVec2(80, 0)))
			{

				App->scene->GetSelectedObject()->RemoveComponent(componentMaterial);
				LOG("Removed Component Material");
				removeMaterial = true;
			}

			if (ImGui::Button("Cancel", ImVec2(80, 0)))
			{
				ImGui::CloseCurrentPopup();
				removeMaterial = true;
			}
			ImGui::EndPopup();
		}

	}
}

void ModuleEditor::InspectorShowCamera(ComponentCamera* componentCamera)
{
	if (ImGui::CollapsingHeader("Camera", &removeCamera, ImGuiTreeNodeFlags_DefaultOpen))
	{
		float nearPlane = componentCamera->GetNearPlaneDistance();
		if (ImGui::DragFloat("Near Plane", &nearPlane, 1.0f,0.1f,componentCamera->GetFarPlaneDistance(),"%.2f"))
			componentCamera->SetNearPlaneDistance(nearPlane);
	
		float farPlane = componentCamera->GetFarPlaneDistance();
		if (ImGui::DragFloat("Far Plane", &farPlane, 1.0f, componentCamera->GetNearPlaneDistance(), 10000.0f, "%.2f"))
			componentCamera->SetFarPlaneDistance(farPlane);
	
		float fov = componentCamera->GetHoritzontalFOV();
		if (ImGui::DragFloat("FOV", &fov, 1.0f, 0.0f))
			componentCamera->SetHoritzontalFOV(fov);

	}

	if (removeCamera == false)
	{
		ImGui::OpenPopup("Are you sure?");

		if (ImGui::BeginPopupModal("Are you sure?"))
		{
			if (ImGui::Button("Delete", ImVec2(80, 0)))
			{
				App->scene->GetSelectedObject()->RemoveComponent(componentCamera);
				LOG("Removed Component Camera");
				removeCamera = true;
			}

			if (ImGui::Button("Cancel", ImVec2(80, 0)))
			{
				ImGui::CloseCurrentPopup();
				removeCamera = true;
			}
			ImGui::EndPopup();
		}
		
	}
}

void ModuleEditor::InspectorShowScript(ComponentScript* componentScript)
{
	if (ImGui::CollapsingHeader("Script", &removeScript, ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Script:");
		ImGui::TextColored(ImVec4(0,1,0,1),componentScript->GetName());
		ImGui::Spacing();
		if (ImGui::Button("Edit Script"))
		{
			std::string pathCPP = ASSETS_SCRIPTS;
			pathCPP += componentScript->GetName();
			pathCPP += ".cpp";
			scriptFile = pathCPP;

			TextEditorInit(scriptFile.c_str());

			editorWindow = true;
		}
	}

	if (removeScript == false)
	{
		ImGui::OpenPopup("Are you sure?");

		if (ImGui::BeginPopupModal("Are you sure?"))
		{
			if (ImGui::Button("Delete", ImVec2(80, 0)))
			{
				App->scene->GetSelectedObject()->RemoveComponent(componentScript);
				LOG("Removed Component Script");
				removeScript = true;
			}

			if (ImGui::Button("Cancel", ImVec2(80, 0)))
			{
				ImGui::CloseCurrentPopup();
				removeScript = true;
			}
			ImGui::EndPopup();
		}

	}
}

void ModuleEditor::HierarchyNodes(GameObject* gameObject)
{

	const char* name = gameObject->GetName();

	ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen;

	if (gameObject == App->scene->GetRootObject())
		treeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
	if (gameObject->children.empty())
		treeFlags |= ImGuiTreeNodeFlags_Leaf;	//Only applyied to the nodes without childrens (Usually the last one)
	if (gameObject == App->scene->GetSelectedObject())
		treeFlags |= ImGuiTreeNodeFlags_Selected;

	if (ImGui::TreeNodeEx(name, treeFlags)) {

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {

			App->scene->SetGameObjectSelected(gameObject);
		}
		//std::string item = "";
		if (gameObject != App->scene->GetRootObject()) {
			if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
				ImGui::OpenPopup("Delete");

			if (ImGui::BeginPopup("Delete"))
			{
				if (ImGui::Selectable("Delete"))
				{
					//Deleteame esto
					LOG("Succesfully deleted %s", gameObject->GetName());
					App->scene->DeleteGameObject(gameObject);
					
				}
				if (ImGui::Selectable("Create Child"))
					App->scene->CreateGameObject("GameObject 1", gameObject);

				ImGui::EndPopup();
			}

			if (ImGui::BeginDragDropSource()) {
				ImGui::SetDragDropPayload("_TREENODE", &gameObject, sizeof(GameObject));

				ImGui::Text("%s", gameObject->GetName());
				dragDropObject = gameObject;
				ImGui::EndDragDropSource();
			}

			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_TREENODE")) {

					App->scene->SetParent(dragDropObject, gameObject);
					dragDropObject = nullptr;
				}
				ImGui::EndDragDropTarget();
			}
		}
		//TODO: Rename SCENE
		/*else if(gameObject == App->scene->GetRootObject())
		{
			if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
				ImGui::OpenPopup("Rename");
			}
			if (ImGui::BeginPopup("Rename"))
			{
				if (ImGui::Selectable("Rename"))
				{
					item = "Change";
				}
				ImGui::EndPopup();
			}

			if (item == "Change") ImGui::OpenPopup("Change Name");
			if (ImGui::BeginPopup("Change Name"))
			{
				static std::string name = App->scene->GetSceneName();
				ImGui::InputText("Rename", (char*)name.c_str(), 50);
				if(ImGui::Button("Apply")) 
				{
					if (name != App->scene->GetSceneName())
					{
						App->scene->SetSceneName(name.c_str());
						LOG("Succesfully changed Scene Name %s", name.c_str());
					}
					else
					{
						LOG("Name is the same");
					}
				}
				
				ImGui::EndPopup();
			}
		}*/
	
		if (!gameObject->children.empty())
		{
			for (uint i = 0; i < gameObject->children.size(); i++)
			{
				HierarchyNodes(gameObject->children[i]);
			}
		}
		
		ImGui::TreePop();
	}
}

void ModuleEditor::WindowProject()
{
	if (projectWindow)
	{
		if (ImGui::Begin("Assets", &projectWindow))
		{
			ImVec2 smallWindow = {ImGui::GetWindowWidth()*1.0f/3.0f, ImGui::GetWindowHeight()*0.85f};
			ImVec2 bigWindow = {ImGui::GetWindowWidth()*1.9f/3.0f, ImGui::GetWindowHeight()*0.85f};
			
			if (ImGui::BeginChild("##AssetsTree", smallWindow, true))		
				DrawAssetDirectory(ASSETS_PATH);

			ImGui::EndChild();

			ImGui::SameLine();

			if (ImGui::BeginChild("##Assets", bigWindow, true))
				ShowDirFiles(selectedFolder.c_str());

			ImGui::EndChild();

		}
		ImGui::End();
	}
}

void ModuleEditor::DrawAssetDirectory(const char* directory)
{
	std::vector<std::string> files;
	std::vector<std::string> dirs;

	std::string dir((directory) ? directory : "");

	App->fileSystem->DiscoverFiles(dir.c_str(), files, dirs);

	ImGuiDockNodeFlags treeFlags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;

	if (dirs.empty())
		treeFlags |= ImGuiTreeNodeFlags_Leaf;

	if (ImGui::TreeNodeEx(dir.c_str(), treeFlags))
	{
		if (ImGui::IsItemClicked())
			selectedFolder = dir;

		treeFlags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Leaf;
		for (std::vector<std::string>::const_iterator it = dirs.cbegin(); it != dirs.cend(); ++it) {
			
			if (ImGui::TreeNodeEx((*it).c_str(), treeFlags))
			{
				if (ImGui::IsItemClicked())
					selectedFolder = (*it);

				ImGui::TreePop();
			}
		}

		ImGui::TreePop();
	}

}

void ModuleEditor::ShowDirFiles(const char* directory)
{
	std::vector<std::string> files;
	std::vector<std::string> dirs;

	std::string dir((directory) ? directory : "");
	dir += "/";

	std::vector<std::string>::const_iterator it;

	App->fileSystem->DiscoverFiles(dir.c_str(), files, dirs);


	/*if (ImGui::BeginDragDropSource()) {
		ImGui::SetDragDropPayload("_TREENODE", &fileToImport, sizeof(GameObject));

		ImGui::EndDragDropSource();
	}

	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_TREENODE")) {

		}
		ImGui::EndDragDropTarget();
	}*/

	ImGui::Text(dir.c_str());
	ImGui::Separator();
	for (it = dirs.begin(); it != dirs.end(); ++it)
	{
		if (ImGui::Button((*it).c_str()))
		{
			//fileToImport = (*it).c_str();

			selectedFolder = *it;
			//LOG("Current Folder %s", selectedFolder.c_str());
		}

	}

	std::sort(files.begin(), files.end());

	static std::string sceneName;
	for (std::vector<std::string>::const_iterator it2 = files.begin(); it2 != files.end(); ++it2)
	{
		const std::string& str = *it2;

		if(strstr(str.c_str(), ".meta") == nullptr)
		{
			if (ImGui::Button(str.c_str()))
			{
				std::string path;
				std::string fileName;
				std::string extension;
				App->fileSystem->SplitFilePath(str.c_str(), &path, &fileName, &extension);
				GameObject* selected = App->scene->GetSelectedObject();

				if (extension == "fbx" || extension == "FBX")
				{
					path = path + fileName + ".meta";
					char* buffer = nullptr;
					App->fileSystem->Load(path.c_str(), &buffer);
					JSON_Value* rootValue = json_parse_string(buffer);
					JSON_Object* node = json_value_get_object(rootValue);
					unsigned int uid = json_object_get_number(node, "LIBUID");
					//TODO: Import the models on the engine
					//ResourceModel* resource = (ResourceModel*)App->resources->RequestResource(uid, Resource::Type::MODEL);
					json_value_free(rootValue);
					delete[] buffer;
				}
				else if (extension == "png" || extension == "tga")
				{

					if (selected != nullptr)
					{
						path = path + fileName + ".meta";
						char* buffer = nullptr;
						App->fileSystem->Load(path.c_str(), &buffer);
						JSON_Value* rootValue = json_parse_string(buffer);
						JSON_Object* node = json_value_get_object(rootValue);
						unsigned int uid = json_object_get_number(node, "LIBUID");
						ComponentMaterial* componentMaterial = (ComponentMaterial*)selected->GetComponent(ComponentType::Material);
						if (componentMaterial != nullptr)
						{
							componentMaterial->ChangeResource(uid);
						}
						else
						{
							Component* component = new ComponentMaterial(uid);
							selected->AddComponent(component);
						}

						json_value_free(rootValue);
						delete[] buffer;
					}

				}
				else if (extension == "scene")
				{
					//TODO: Save and load scenes
					sceneName = str.c_str();
					ImGui::OpenPopup("Warning");
				}
				else if (extension == "mesh")
				{
					char* buffer = nullptr;
					App->fileSystem->Load(str.c_str(), &buffer);
					JSON_Value* rootValue = json_parse_string(buffer);
					JSON_Object* node = json_value_get_object(rootValue);
					unsigned int uid = json_object_get_number(node, "LIBUID");

					if (selected != nullptr)
					{
						ComponentMesh* componentMesh = (ComponentMesh*)selected->GetComponent(ComponentType::Mesh);
						if (componentMesh != nullptr)
						{
							componentMesh->ChangeResource(uid);
						}
						else
						{
							//We need a component transform for the mesh to work
							if (selected->GetComponent(ComponentType::Transform) == nullptr)
								Component* component = new ComponentTransform(selected);

							Component* component = new ComponentMesh(uid);
							selected->AddComponent(component);
						}
						json_value_free(rootValue);
						delete[] buffer;
					}
					else
					{
						Component* component = new ComponentMesh(uid);
						unsigned int count = ((ComponentMesh*)component)->GetResource()->GetReferenceCount();
						if (count > 1)
						{
							fileName += "(";
							std::string num = std::to_string(count);
							fileName += num + ")";
						}
						GameObject* go = App->scene->CreateTransformGameObject(fileName.c_str(), nullptr);
						go->AddComponent(component);
					}
				}
				else if (extension == "cpp")
				{
					std::string pathCPP = ASSETS_SCRIPTS;
					pathCPP += fileName;
					pathCPP += ".cpp";
					scriptFile = pathCPP;

					TextEditorInit(scriptFile.c_str());

					editorWindow = true;
				}
			}
		}
		//ImGui::SameLine();
	}
	if (ImGui::BeginPopupModal("Warning"))
	{
		ImGui::Text("All scene unsaved changes will be lost");
		ImGui::Text("Are you sure?");
		if (ImGui::Button("Yes"))
		{
			char* buffer = nullptr;
			std::string path = ASSETS_SCENES;
			path += sceneName;
			App->fileSystem->Load(path.c_str(), &buffer);
			JSON_Value* rootValue = json_parse_string(buffer);
			JSON_Object* node = json_value_get_object(rootValue);
			unsigned int uid = json_object_get_number(node, "LIBUID");
			json_value_free(rootValue);
			delete[] buffer;
			App->scene->LoadScene(uid);
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::Button("No"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}



void ModuleEditor::WindowDemo()
{
	if (demoWindow)
	{
		//our state (depenent de si el bool que pasem a la window es true o false s'ensenya la window i si la tanquem imgui posa el bool directament a false)
		ImGui::ShowDemoWindow(&demoWindow);
	}
}

void ModuleEditor::WindowScene()
{
	if (sceneWindow)
	{
		if (ImGui::Begin("Scene", &sceneWindow))
		{
			ImGui::BeginChild("Scene Texture");

			ImGui::Image((ImTextureID)App->renderer3D->sceneTextureBuffer, { ImGui::GetWindowWidth(),ImGui::GetWindowHeight() }, ImVec2(0,1), ImVec2(1,0));
			ImGui::EndChild();
		}
		ImGui::End();
	}
}

void ModuleEditor::WindowTime()
{
	if (timeWindow)
	{
		if (ImGui::Begin("Time", &timeWindow, ImGuiWindowFlags_NoDecoration)) {

			if (ImGui::Button("Translate"))
			{
				gizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
				LOG("Guizmo Operation TRANSLATE");
			}
			ImGui::SameLine();
			if (ImGui::Button("Rotate"))
			{
				gizmoOperation = ImGuizmo::OPERATION::ROTATE;
				LOG("Guizmo Operation ROTATE");
			}
			ImGui::SameLine();
			if (ImGui::Button("Scale"))
			{
				gizmoOperation = ImGuizmo::OPERATION::SCALE;
				LOG("Guizmo Operation SCALE");
			}
			ImGui::SameLine();
			ImGui::InvisibleButton("##", ImVec2(50, 10));
			ImGui::SameLine();

			if (App->IsGameRunning())
			{
				playStop = "Stop";
			}
			else
			{
				playStop = "Play";
			}
			if (ImGui::Button(playStop.c_str()))
			{
				if (App->IsGameRunning())
				{
					App->StopGame();
					LOG("Stop Game");
				}
				else
				{
					App->StartGame();
					LOG("Start Game");
				}
			}
			ImGui::SameLine();
			if (App->IsGameRunning())
			{
				if (App->IsGamePaused())
				{
					pauseResume = "Resume";
				}
				else
				{
					pauseResume = "Pause";
				}
				if (ImGui::Button(pauseResume.c_str()))
				{
					if (App->IsGamePaused())
					{
						App->ResumeGame();
						LOG("Resume Game");
					}
					else
					{
						App->PauseGame();
						LOG("Pause Game");
					}
				}
			}
			ImGui::SameLine();
			ImGui::Text("Start time:");
			ImGui::SameLine();
			float startAppTimer = App->startApp.Read() / 1000.f;;
			ImGui::TextColored(ImVec4(0,1,1,1), "%.3f", startAppTimer);
			ImGui::SameLine();
			ImGui::Text("Game time:");
			ImGui::SameLine();
			float gameTime = App->GetGameTime();
			ImGui::TextColored(ImVec4(0, 1, 1, 1), "%.3f", gameTime);
		}
		ImGui::End();
	}
}

void ModuleEditor::WindowResourcesCount()
{
	if (resourcesWindow)
	{
		if (ImGui::Begin("Resources", &resourcesWindow))
		{

		}
		ImGui::End();
	}
}

void ModuleEditor::ChangeTextureWindow(const char* directory, ComponentMaterial* material)
{
	if (changeTexture)
	{
		if (ImGui::Begin("Change Texture", &changeTexture))
		{

			std::vector<std::string> files;
			std::vector<std::string> dirs;

			std::string dir((directory) ? directory : "");
			dir += "/";

			std::vector<std::string>::const_iterator it;

			App->fileSystem->DiscoverFiles(dir.c_str(), files, dirs);

			std::sort(files.begin(), files.end());

			for (std::vector<std::string>::const_iterator it2 = files.begin(); it2 != files.end(); ++it2)
			{
				if (strstr((*it2).c_str(), ".meta") == nullptr)
				{
					if (ImGui::Button((*it2).c_str()))
					{
						std::string path;
						std::string fileName;
						std::string extension;
						App->fileSystem->SplitFilePath((*it2).c_str(), &path, &fileName, &extension);
						path = path + fileName + ".meta";
						char* buffer = nullptr;
						App->fileSystem->Load(path.c_str(), &buffer);
						JSON_Value* rootValue = json_parse_string(buffer);
						JSON_Object* node = json_value_get_object(rootValue);
						unsigned int uid = json_object_get_number(node, "LIBUID");

						material->ChangeResource(uid);

						changeTexture = false;

						json_value_free(rootValue);
						delete[] buffer;
					}
				}
			}
		}
		ImGui::End();
	}
}


void ModuleEditor::ChangeMeshWindow(const char* directory, ComponentMesh* mesh)
{
	if (changeMesh)
	{
		if (ImGui::Begin("Change Mesh", &changeMesh))
		{
			std::vector<std::string> files;
			std::vector<std::string> dirs;

			std::string dir((directory) ? directory : "");
			dir += "/";

			std::vector<std::string>::const_iterator it;

			App->fileSystem->DiscoverFiles(dir.c_str(), files, dirs);

			std::sort(files.begin(), files.end());

			for (std::vector<std::string>::const_iterator it2 = files.begin(); it2 != files.end(); ++it2)
			{
				if (strstr((*it2).c_str(), ".meta") == nullptr)
				{
					if (ImGui::Button((*it2).c_str()))
					{
						char* buffer = nullptr;
						App->fileSystem->Load((*it2).c_str(), &buffer);
						JSON_Value* rootValue = json_parse_string(buffer);
						JSON_Object* node = json_value_get_object(rootValue);
						unsigned int uid = json_object_get_number(node, "LIBUID");

						mesh->ChangeResource(uid);

						changeMesh = false;

						json_value_free(rootValue);
						delete[] buffer;
					}
				}
			}
		}
		ImGui::End();
	}
}


void ModuleEditor::AddTextureWindow(const char* directory)
{
	if (addedTexture)
	{
		if (ImGui::Begin("Add Texture", &addedTexture))
		{

			std::vector<std::string> files;
			std::vector<std::string> dirs;

			std::string dir((directory) ? directory : "");
			dir += "/";

			std::vector<std::string>::const_iterator it;

			App->fileSystem->DiscoverFiles(dir.c_str(), files, dirs);

			std::sort(files.begin(), files.end());

			for (std::vector<std::string>::const_iterator it2 = files.begin(); it2 != files.end(); ++it2)
			{
				if (strstr((*it2).c_str(), ".meta") == nullptr)
				{
					if (ImGui::Button((*it2).c_str()))
					{
						std::string path;
						std::string fileName;
						std::string extension;
						App->fileSystem->SplitFilePath((*it2).c_str(), &path, &fileName, &extension);
						path = path + fileName + ".meta";
						char* buffer = nullptr;
						App->fileSystem->Load(path.c_str(), &buffer);
						JSON_Value* rootValue = json_parse_string(buffer);
						JSON_Object* node = json_value_get_object(rootValue);
						unsigned int uid = json_object_get_number(node, "LIBUID");

						ComponentMaterial* material = new ComponentMaterial(uid);

						App->scene->GetSelectedObject()->AddComponent(material);

						LOG("Successful added a Component Material");
						addedTexture = false;

						json_value_free(rootValue);
						delete[] buffer;
					}
				}
			}
		}
		ImGui::End();
	}
}

void ModuleEditor::AddMeshWindow(const char* directory)
{
	if (addedMesh)
	{
		if (ImGui::Begin("Add Mesh", &addedMesh))
		{
			std::vector<std::string> files;
			std::vector<std::string> dirs;

			std::string dir((directory) ? directory : "");
			dir += "/";

			std::vector<std::string>::const_iterator it;

			App->fileSystem->DiscoverFiles(dir.c_str(), files, dirs);

			std::sort(files.begin(), files.end());

			for (std::vector<std::string>::const_iterator it2 = files.begin(); it2 != files.end(); ++it2)
			{
				if (strstr((*it2).c_str(), ".meta") == nullptr)
				{
					if (ImGui::Button((*it2).c_str()))
					{
						
						char* buffer = nullptr;
						App->fileSystem->Load((*it2).c_str(), &buffer);
						JSON_Value* rootValue = json_parse_string(buffer);
						JSON_Object* node = json_value_get_object(rootValue);
						unsigned int uid = json_object_get_number(node, "LIBUID");
						ComponentMesh* mesh = new ComponentMesh(uid);

						App->scene->GetSelectedObject()->AddComponent(mesh);

						addedMesh = false;
						LOG("Successful added a Component Mesh");
						json_value_free(rootValue);
						delete[] buffer;
					}
				}
			}
		}
		ImGui::End();
	}
	
}

void ModuleEditor::AddLog(const char* string)
{
	char* newString = strdup(string);
	stringLog.push_back(newString);

	scroll = true;
}

void ModuleEditor::ClearLog()
{
	for (uint i = 0; i < stringLog.size(); ++i)
	{
		free(stringLog[i]);
	}

	stringLog.clear();

	scroll = true;
}

void ModuleEditor::ImGuizmoUpdate()
{
	if (App->input->GetKey(SDL_SCANCODE_W) == KEY_STATE::KEY_UP && !ImGui::GetIO().WantCaptureKeyboard)
		gizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
	if (App->input->GetKey(SDL_SCANCODE_E) == KEY_STATE::KEY_UP && !ImGui::GetIO().WantCaptureKeyboard)
		gizmoOperation = ImGuizmo::OPERATION::ROTATE;
	if (App->input->GetKey(SDL_SCANCODE_R) == KEY_STATE::KEY_UP && !ImGui::GetIO().WantCaptureKeyboard)
		gizmoOperation = ImGuizmo::OPERATION::SCALE;

	GameObject* tmp = App->scene->GetSelectedObject();

	if (tmp)
	{
		ComponentTransform* selectedTransform = (ComponentTransform*)tmp->GetComponent(ComponentType::Transform);

		if (selectedTransform && tmp->HasComponent(ComponentType::Mesh)) 
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			//ImGuiIO& io = ImGui::GetIO();
			//ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
			float windowWidth = (float)ImGui::GetWindowWidth();
			float windowHeight= (float)ImGui::GetWindowHeight();
			ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

			float4x4 viewMatrix = App->camera->GetCamera()->GetGLViewMatrix();
			float4x4 projectionMatrix = App->camera->GetCamera()->GetGLProjectionMatrix();
			float4x4 modelMatrix = selectedTransform->GetGlobalTransform().Transposed();

			float modelPtr[16];
			memcpy(modelPtr, modelMatrix.ptr(), 16 * sizeof(float));
			ImGuizmo::MODE finalMode = (gizmoOperation == ImGuizmo::OPERATION::SCALE ? ImGuizmo::MODE::LOCAL : ImGuizmo::MODE::WORLD);

			ImGuizmo::Manipulate(viewMatrix.ptr(), projectionMatrix.ptr(), gizmoOperation, finalMode, modelPtr);

			if (ImGuizmo::IsUsing()) 
			{
				float4x4 newMatrix;
				newMatrix.Set(modelPtr);
				modelMatrix = newMatrix.Transposed();
				selectedTransform->SetGlobalTransform(modelMatrix);
			}
		}
	}
}

void ModuleEditor::TextEditorInit(const char* name)
{

	auto lang = TextEditor::LanguageDefinition::CPlusPlus();

	// set your own known preprocessor symbols...
	static const char* ppnames[] = { "NULL", "PM_REMOVE",
		"ZeroMemory", "DXGI_SWAP_EFFECT_DISCARD", "D3D_FEATURE_LEVEL", "D3D_DRIVER_TYPE_HARDWARE", "WINAPI","D3D11_SDK_VERSION", "assert" };
	// ... and their corresponding values
	static const char* ppvalues[] = {
		"#define NULL ((void*)0)",
		"#define PM_REMOVE (0x0001)",
		"Microsoft's own memory zapper function\n(which is a macro actually)\nvoid ZeroMemory(\n\t[in] PVOID  Destination,\n\t[in] SIZE_T Length\n); ",
		"enum DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_DISCARD = 0",
		"enum D3D_FEATURE_LEVEL",
		"enum D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE  = ( D3D_DRIVER_TYPE_UNKNOWN + 1 )",
		"#define WINAPI __stdcall",
		"#define D3D11_SDK_VERSION (7)",
		" #define assert(expression) (void)(                                                  \n"
		"    (!!(expression)) ||                                                              \n"
		"    (_wassert(_CRT_WIDE(#expression), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0) \n"
		" )"
	};

	for (int i = 0; i < sizeof(ppnames) / sizeof(ppnames[0]); ++i)
	{
		TextEditor::Identifier id;
		id.mDeclaration = ppvalues[i];
		lang.mPreprocIdentifiers.insert(std::make_pair(std::string(ppnames[i]), id));
	}

	// set your own identifiers
	static const char* identifiers[] = {
		"HWND", "HRESULT", "LPRESULT","D3D11_RENDER_TARGET_VIEW_DESC", "DXGI_SWAP_CHAIN_DESC","MSG","LRESULT","WPARAM", "LPARAM","UINT","LPVOID",
		"ID3D11Device", "ID3D11DeviceContext", "ID3D11Buffer", "ID3D11Buffer", "ID3D10Blob", "ID3D11VertexShader", "ID3D11InputLayout", "ID3D11Buffer",
		"ID3D10Blob", "ID3D11PixelShader", "ID3D11SamplerState", "ID3D11ShaderResourceView", "ID3D11RasterizerState", "ID3D11BlendState", "ID3D11DepthStencilState",
		"IDXGISwapChain", "ID3D11RenderTargetView", "ID3D11Texture2D", "TextEditor" };
	static const char* idecls[] =
	{
		"typedef HWND_* HWND", "typedef long HRESULT", "typedef long* LPRESULT", "struct D3D11_RENDER_TARGET_VIEW_DESC", "struct DXGI_SWAP_CHAIN_DESC",
		"typedef tagMSG MSG\n * Message structure","typedef LONG_PTR LRESULT","WPARAM", "LPARAM","UINT","LPVOID",
		"ID3D11Device", "ID3D11DeviceContext", "ID3D11Buffer", "ID3D11Buffer", "ID3D10Blob", "ID3D11VertexShader", "ID3D11InputLayout", "ID3D11Buffer",
		"ID3D10Blob", "ID3D11PixelShader", "ID3D11SamplerState", "ID3D11ShaderResourceView", "ID3D11RasterizerState", "ID3D11BlendState", "ID3D11DepthStencilState",
		"IDXGISwapChain", "ID3D11RenderTargetView", "ID3D11Texture2D", "class TextEditor" };
	for (int i = 0; i < sizeof(identifiers) / sizeof(identifiers[0]); ++i)
	{
		TextEditor::Identifier id;
		id.mDeclaration = std::string(idecls[i]);
		lang.mIdentifiers.insert(std::make_pair(std::string(identifiers[i]), id));
	}
	editor.SetLanguageDefinition(lang);
	//editor.SetPalette(TextEditor::GetLightPalette());

	// error markers
	TextEditor::ErrorMarkers markers;
	markers.insert(std::make_pair<int, std::string>(6, "Example error here:\nInclude file not found: \"TextEditor.h\""));
	markers.insert(std::make_pair<int, std::string>(41, "Another example error"));
	editor.SetErrorMarkers(markers);

	// "breakpoint" markers
	//TextEditor::Breakpoints bpts;
	//bpts.insert(24);
	//bpts.insert(47);
	//editor.SetBreakpoints(bpts);

	scriptFile = name;

	//	static const char* fileToEdit = "test.cpp";

	{
		std::ifstream t(scriptFile);
		if (t.good())
		{
			std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
			editor.SetText(str);
		}
	}

}

void ModuleEditor::TextEditorWindow()
{

	auto cpos = editor.GetCursorPosition();

	if (editorWindow)
	{
		ImGui::Begin("Text Editor Demo", &editorWindow, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
		ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
		if (ImGui::BeginMenuBar())
		{

			if (ImGui::BeginMenu("Edit"))
			{
				bool ro = editor.IsReadOnly();
				if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
					editor.SetReadOnly(ro);
				ImGui::Separator();

				if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && editor.CanUndo()))
					editor.Undo();
				if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && editor.CanRedo()))
					editor.Redo();

				ImGui::Separator();

				if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor.HasSelection()))
					editor.Copy();
				if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && editor.HasSelection()))
					editor.Cut();
				if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && editor.HasSelection()))
					editor.Delete();
				if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
					editor.Paste();

				ImGui::Separator();

				if (ImGui::MenuItem("Save", "Ctrl-S", nullptr))
				{
					//TODO Save the current 
					std::string changedScript = editor.GetText();
					App->fileSystem->Remove(scriptFile.c_str());
					App->fileSystem->Save(scriptFile.c_str(), changedScript.c_str(), editor.GetText().size());
				}
				ImGui::Separator();

				if (ImGui::MenuItem("Select all", nullptr, nullptr))
					editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor.GetTotalLines(), 0));

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::MenuItem("Dark palette"))
					editor.SetPalette(TextEditor::GetDarkPalette());
				if (ImGui::MenuItem("Light palette"))
					editor.SetPalette(TextEditor::GetLightPalette());
				if (ImGui::MenuItem("Retro blue palette"))
					editor.SetPalette(TextEditor::GetRetroBluePalette());
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
			editor.IsOverwrite() ? "Ovr" : "Ins",
			editor.CanUndo() ? "*" : " ",
			editor.GetLanguageDefinition().mName.c_str(), scriptFile.c_str());

		editor.Render("TextEditor");
		ImGui::End();
	}
}
