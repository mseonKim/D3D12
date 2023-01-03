#pragma once

// Modify this macro depending on a chapter you want to test
#define CH23


#ifdef CH6
#define COLOR
#endif

#ifdef CH7
#define COLOR
#endif

#ifdef CH10
#define WAVE
#define FOG
#endif

#ifdef CH11
#define FOG
#endif

#ifdef CH12
#define WAVE
#define FOG
#endif

#ifdef CH13
#define WAVE
#define FOG
#endif

#ifdef CH14
#define FOG
#endif

#ifdef CH16
#define INSTANCING
#endif

#ifdef CH20
#define SHADOW
#endif

#ifdef CH21
#define SHADOW
#define SSAO
#endif

#ifdef CH23
#define SHADOW
#define SSAO
#endif