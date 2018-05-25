# Nova Renderer
##### A complete replacement for Minecraft's renderer

[Follow us on Twitter!](https://twitter.com/NovaRendererMC)  
[Talk to us on Discord!](https://discordapp.com/invite/014ux6siiDogn9FLz)  
[There's even a blog now!](https://continuum.graphics/blog/?category=Nova)

### Development status
The Nova Renderer is currently early in development, so early that I don't think there's anything worth releasing. Yes, you can download the code and try running it yourself, but you have to make some changes to the Minecraft source code to add in hooks for Nova.

If you want to see what we have planned, or what has already been done, check out [the GitHub projects page](https://github.com/NovaMods/nova-renderer/projects).
 
# End Goal
Everything from here on out describes what the Nova Renderer will look like during version 1.0. This doesn't necessarily reflect the current status of the project. Check [the GitHub projects page](https://github.com/NovaMods/nova-renderer/projects) to see the current status of the Nova Renderer.

The Nova Renderer is a complete replacement for Minecraft's default renderer. Minecraft's renderer is nice and all, but it's written in Java (ugh) using the ancient OpenGL 2.1 API. Additionally, the Minecraft Shaders Mod does a number of things in strange ways, and, while incredible, isn't the best-designed code. This mod seeks to fix those problems.

### Built to run fast
The Nova Renderer is written in C++ for a couple reasons: speed and control. By using C++ I can work with the raw graphics API without needing to go through a language translation layer like LWJGL, avoiding overhead from that. Additionally, I have full control over memory allocation. I don't need to try to babysit a virtual machine to keep it from stuttering - I allocate memory when I need it and deallocate when I don't.

C++ also gives me the option to bring Nova to other games. I want to port Nova into Fallout: New Vegas, building up a shader modding community around another game.

The choice of language is not the only performance-oriented decision I made. The architecture of the code itself also lend itself to speed. I utilize data-oriented-design to design my data structures such that the CPU can deal with them really quickly. I also take full advantage of Vulkan's features, like indirect drawing, in order to ensure that Nova runs as fast as possible.

### Full of features
The Nova Renderer has a number of awesome features. Like the Shaders Mod before it, the Nova Renderer allows a user to use shaderpacks, collections of graphics code that alter the way Minecraft looks. Shaderpacks can control every aspect of the rendering process, from water simulation to lighting to post-processing. Shaders can add in shadows, depth of field, physically based rendering, realtime reflections, and a ton of other things. Both Nova and the Shaders Mod support all that.

However, Nova adds in a number of other features, starting with version 1.1. It creates a new shader for block lights, like torches or glowstone, allowing for per-pixel lighting from all light sources and shadows from the closest light sources. Nova adds in a number of new uniform variables, giving shaderpack developers even more power to implement high-quality graphics. Nova brings forth support for tessellation and geometry shaders, allowing for techniques like true displacement mapping and stencil buffer shadows. Nova version 2 will focus on compatibility with existing mods and will provide a way for users to customize the geometry used for each block and entity and define different particle systems. Version 3 brings in compute shaders, allowing for incredibly advanced effects, like order-independent transparency and full scene raytracing. Additionally, Nova will stay up-to-date with the latest versions of Vulkan, bringing cutting-edge features into the hands of shaderpack developers.

Note that the order in which features will be implemented is not set in stone. If lots of people want a particular feature, I'll prioritize that feature and move it to the top of my to-do list.

### High Compatibility
At E3 2017, Microsoft announced the Super Duper Graphics Pack, and some accompanying tech to allow end users to write their own shaders for Minecraft on Windows 10, Xbox, Switch, and Pocket Edition as part of the new Bedrock engine. I decided to make Nova fully compatible with Bedrock's shader tech, which helped greatly in converting Nova into a data driven renderer.

### Data-Driven
Nova lets shader developers define the passes that Nova makes over the scene. This provides shader developers a great deal of flexibility with how objects are drawn to the screen. More details coming as I implement more features.

### Installation ###
[Do this](https://github.com/NovaMods/nova-renderer/wiki/Installing,-building,-and-running-on-Windows)
