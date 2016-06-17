# Nova Renderer
##### A complete replacement for Minecraft's renderer

[Follow us on Twitter!](https://twitter.com/NovaRendererMC)

### Development status
The Nova Renderer is currently early in development, so early that I don't think there's anything worth releasing. Yes, you can download the code and try running it yourself, but you have to make some changes to the Minecraft source code to add in hooks for Nova.

If you want to see what we have planned, or what has already been done, check out [our Trello board](https://trello.com/b/tf517UY2/nova-renderer).
 
# End Goal
Everything from here on out describes what the Nova Renderer will look like during version 1.0. This doesn't necessarily reflect the current status of the project. Check [our Trello board](https://trello.com/b/tf517UY2/nova-renderer) to see the current status of the Nova Renderer.

The Nova Renderer is a complete replacement for Minecraft's default renderer. Minecraft's renderer is nice and all, but it's written in Java (ugh) using the ancient OpenGL 2.1 API. Additionally, the Minecraft Shaders Mod does a number of things in strange ways, and, while incredible, isn't the best-designed code. This mod seeks to fix those problems.

### Built to run fast
The Nova Renderer is written in C++ for a couple reasons. First and foremost, I like C++ better than Java. The way I see it, Java treats you like a child, like you'll hurt yourself if left alone too long, while C++ respects you as an adult and trusts you to make your own decisions. Additionally, optimized C++ can run faster than Java since C++ doesn't require the overhead of a virtual machine (Although the JVM is much better than it used to be).

The choice of language is not the only performance-oriented decision I made. The architecture of the code itself also lend itself to speed. I utilize data-oriented-design to design my data structures such that the CPU can deal with them really quickly. I also take full advantage of OpenGL 4.5's features, like the glMultiDrawIndirect function, in order to ensure as little driver overhead as possible.

### Full of features
The Nova Renderer has a number of awesome features. Like the Shaders Mod before it, the Nova Renderer allows a user to use shaderpacks, collections of graphics code that alter the way Minecraft looks. Shaderpacks can control every aspect of the rendering process, from water simulation to lighting to post-processing. Shaders can add in shadows, depth of field, physically based rendering, realtime reflections, and a ton of other things. Both Nova and the Shaders Mod support all that.

However, Nova adds in a number of other features, starting with version 1.1. It creates a new shader for block lights, like torches or glowstone, allowing for per-pixel lighting from all light sources and shadows from the closest light sources. Nova adds in a number of new uniform variables, giving shaderpack developers even more power to implement high-quality graphics. Nova brings forth support for tessellation and geometry shaders, allowing for techniques like true displacement mapping and stencil buffer shadows. Nova version 2 will focus on compatibility with existing mods and will provide a way for users to customize the geometry used for each block and entity and define different particle systems. Version 3 brings in compute shaders, allowing for incredibly advanced effects, like order-independent transparency and full scene raytracing. Additionally, Nova will stay up-to-date with the latest versions of OpenGL, bringing cutting-edge features into the hands of shaderpack developers.

Note that the order in which features will be implemented is not set in stone. If lots of people want a particular feature, I'll prioritize that feature and move it to the top of my to-do list.

### Installation ###
Coming soon ... 
