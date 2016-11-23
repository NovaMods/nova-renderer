
if (NOT EXISTS "D:/Documents/Nova Renderer/src/main/cpp/cmake-build-debug/3rdparty/glfw/install_manifest.txt")
  message(FATAL_ERROR "Cannot find install manifest: \"D:/Documents/Nova Renderer/src/main/cpp/cmake-build-debug/3rdparty/glfw/install_manifest.txt\"")
endif()

file(READ "D:/Documents/Nova Renderer/src/main/cpp/cmake-build-debug/3rdparty/glfw/install_manifest.txt" files)
string(REGEX REPLACE "\n" ";" files "${files}")

foreach (file ${files})
  message(STATUS "Uninstalling \"$ENV{DESTDIR}${file}\"")
  if (EXISTS "$ENV{DESTDIR}${file}")
    exec_program("C:/Program Files (x86)/JetBrains/CLion 2016.2.1/bin/cmake/bin/cmake.exe" ARGS "-E remove \"$ENV{DESTDIR}${file}\""
                 OUTPUT_VARIABLE rm_out
                 RETURN_VALUE rm_retval)
    if (NOT "${rm_retval}" STREQUAL 0)
      MESSAGE(FATAL_ERROR "Problem when removing \"$ENV{DESTDIR}${file}\"")
    endif()
  elseif (IS_SYMLINK "$ENV{DESTDIR}${file}")
    EXEC_PROGRAM("C:/Program Files (x86)/JetBrains/CLion 2016.2.1/bin/cmake/bin/cmake.exe" ARGS "-E remove \"$ENV{DESTDIR}${file}\""
                 OUTPUT_VARIABLE rm_out
                 RETURN_VALUE rm_retval)
    if (NOT "${rm_retval}" STREQUAL 0)
      message(FATAL_ERROR "Problem when removing symlink \"$ENV{DESTDIR}${file}\"")
    endif()
  else()
    message(STATUS "File \"$ENV{DESTDIR}${file}\" does not exist.")
  endif()
endforeach()

