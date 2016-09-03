#ifndef RENDERER_EXPORT_H
#define RENDERER_EXPORT_H

#if defined(_WIN32)
  #if defined(DLL_EXPORT)
    #define NOVA_EXPORT __declspec(dllexport)
  #else
    #if defined(STATIC_LINKAGE)
      #define NOVA_EXPORT
    #else
      #define NOVA_EXPORT __declspec(dllimport)
    #endif
  #endif
#else
  #if defined(__GNUC__)
    #define NOVA_EXPORT __attribute__((visibility("default")))
  #endif
#endif

#endif //RENDERER_EXPORT_H
