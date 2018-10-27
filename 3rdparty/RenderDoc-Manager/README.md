This is a small code skeleton heavily based on the work of Temaran (https://github.com/Temaran/UE4RenderDocPlugin) that will integrate RenderDoc (https://github.com/baldurk/renderdoc) in your engine.

Keep in mind that the RenderDoc API is still in its early stage, it's not release quality yet, it may have some issues and might evolve in the following versions.

You just need to call the RenderDocManager constructor before the D3D initialization and you should be able to capture a frame using the StartFrameCapture() and EndFrameCapture() or by pressing the "CaptureKey" defined in the constructor.

The capture options defined in the code example might make the program run much slower, for better performances only activate the options you need.

For more informations: http://www.alexandre-pestana.com/integrating-renderdoc/

This fork has been modified to handle a GLFW window rather than a Win32 window, and is set up for OpenGL
