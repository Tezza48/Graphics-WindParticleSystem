#pragma once
#include <comdef.h>
#include <cstdio>

#define SafeRelease(x) if (x) { x->Release(); x = nullptr;}
#define D3D_CALL(x) if(FAILED(x)){\
	_com_error err(x);\
	std::printf("Error: in file %s at line (%d), hr: %s\n", __FILE__, __LINE__, err.ErrorMessage());\
	\
	}