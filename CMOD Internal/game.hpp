#include "offsets.h"
#include "signature.h"
#include "skCrypter.h"

#define dwbase (DWORD64)GetModuleHandleA(0)





typedef bool(__fastcall* islobby_fn)();
islobby_fn o_islobby = 0;

bool is_lobby()
{
	if (o_islobby == 0)
		o_islobby = (islobby_fn)util::ida_signature(dwbase, E("48 83 EC 28 48 8B 0D ? ? ? ? 48 8D 15 ? ? ? ? 48 8B 01 48 3B C2 75 14 83 3D ? ? ? ? ?"));

	return o_islobby();
}
