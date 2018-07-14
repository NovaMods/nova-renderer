#ifndef RENDERER_EXPORT_H
#define RENDERER_EXPORT_H

#if defined(_WIN32)
  #if defined(DLL_EXPORT)
    #define NOVA_API __declspec(dllexport)
  #else
    #if defined(STATIC_LINKAGE)
      #define NOVA_API
    #else
      #define NOVA_API __declspec(dllimport)
    #endif
  #endif
#else
  #if defined(__GNUC__)
    #define NOVA_API __attribute__((visibility("default")))
  #endif
#endif

#endif //RENDERER_EXPORT_H
