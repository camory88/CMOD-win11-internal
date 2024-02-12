#pragma once
#include <Windows.h>
#include <iostream>
#include "signature.h"
#include "skCrypter.h"



static bool MainMenu = true;
static bool InfoMenu = true;

bool getKeyDown(int key)
{
	return (GetAsyncKeyState(key) & 0x8000) != 0;
}
