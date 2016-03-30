#pragma once

#include <Windows.h>
#include <cstdio>
#include <string>
#include <time.h>

const size_t allowedEndSize = 7;
const std::string allowedEnd[allowedEndSize] =
{
	" - Notepad++",
	" - Microsoft Visual Studio",
	" - Google Chrome",
	" - Chat",
	" - Anteckningar",
	" - TightVNC Viewer",
	" - Microsoft Word"
};

const size_t allowedStartSize = 2;
const std::string allowedStart[allowedStartSize] =
{
	"Mumble - ",
	"Skype? - "
};

class SpaceFix{
public:

	SpaceFix(){};
	~SpaceFix(){};

	void display(){

	}

	bool tick(){
		bool handled = false;
		unsigned long long ticks = GetTickCount64();

		if(ticks - lastTick < 1){
			return false;
		}

		if(ticks - lastCheck > 1000){
			lastCheck = ticks;
			if(!(handle = shouldHandleSpace())){
				return false;
			}
		}

		if(!handle){
			return false;
		}

		if((isPressed = (GetAsyncKeyState(VK_SPACE) & 0x8000)) && !wasPressed){
			if(ticks - lastPress < 50){
				handled = true;
				simulateKeyPress(VK_BACK);
			}

			lastPress = ticks;
		}

		wasPressed = isPressed;
		lastTick = ticks;
		return handled;
	}

private:

	bool handle = false;
	bool isPressed;
	bool wasPressed = false;
	unsigned long long lastTick = 0;
	unsigned long long lastPress = 0;
	unsigned long long lastCheck = 0;

	void simulateKeyPress(const BYTE& key){
		keybd_event(key, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
		keybd_event(key, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	}

	std::string activeWindowTitle(){
		char wnd_title[256];

		GetWindowText(GetForegroundWindow(), wnd_title, sizeof(wnd_title));

		return wnd_title;
	}

	bool endsWith(const std::string& s, const std::string& e){
		size_t sl = s.length();

		if(e.length() > sl){
			return false;
		}

		sl -= e.length();

		for(size_t i = 0; i < e.length(); ++i){
			if(e.at(i) != '?' && s.at(sl + i) != e.at(i)){
				return false;
			}
		}

		return true;
	}

	bool startsWith(const std::string& s, const std::string& e){
		if(e.length() > s.length()){
			return false;
		}

		for(size_t i = 0; i < e.length(); i++){
			if(e.at(i) != '?' && s.at(i) != e.at(i)){
				return false;
			}
		}

		return true;
	}

	bool shouldHandleSpace(){
		std::string w = activeWindowTitle();

		for(size_t i = 0; i < allowedEndSize; i++){
			if(endsWith(w, allowedEnd[i])){
				return true;
			}
		}

		for(size_t i = 0; i < allowedStartSize; i++){
			if(startsWith(w, allowedStart[i])){
				return true;
			}
		}

		return false;
	}

};