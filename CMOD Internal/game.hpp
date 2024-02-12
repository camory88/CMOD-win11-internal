#include "offsets.h"
#include "signature.h"
#include "skCrypter.h"
#include "Vector.h"

#define dwbase (DWORD64)GetModuleHandleA(0)



typedef char* (__fastcall* getplayername_fn)(uint64_t* ent);
getplayername_fn o_getplayername = 0;

typedef bool(__fastcall* islobby_fn)();
islobby_fn o_islobby = 0;

bool is_lobby()
{
	if (o_islobby == 0)
		o_islobby = (islobby_fn)util::ida_signature(dwbase, E("48 8D 0D ? ? ? ? 48 89 48 ? 48 8D 0D ? ? ? ? C7 40 ? ? ? ? ? C7 40"));

	return o_islobby();
}


char* get_player_name(uint64_t* ent)
{
	if (o_getplayername == 0)
		o_getplayername = (getplayername_fn)util::ida_signature(dwbase, E("48 8D 0D 9D FC D5 00"));

	return o_getplayername(ent);
}

uint64_t* get_entiy(uint64_t idx) {
	return *(uint64_t**)(dwbase + cl_entitylist + (idx << 5));
}