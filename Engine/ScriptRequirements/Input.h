#pragma once
//#include "SDL_scancode.h"
#include "SDL/SDL.h"


class __declspec(dllimport) Input {
public:

	enum KEY_STATE {
		KEY_IDLE = 0,
		KEY_DOWN,
		KEY_REPEAT,
		KEY_UP
	};

	enum MOUSE_BUTTONS {
		MOUSE_LEFT_BUTTON = 1,
		MOUSE_MIDDLE_BUTTON = 2,
		MOUSE_RIGHT_BUTTON = 3,
	};

	bool GetKeyDown(const SDL_Scancode& key);
	bool GetKeyUp(const SDL_Scancode& key);
	bool GetKeyRepeat(const SDL_Scancode& key);

	int GetMouseX();
	int GetMouseY();
	int GetMouseWheel();

	bool GetMouseButtonDown(MOUSE_BUTTONS& button);
	bool GetMouseButtonUp(MOUSE_BUTTONS& button);
	bool GetMouseButtonRepeat(MOUSE_BUTTONS& button);

};