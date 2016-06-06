# Nova Renderer
##### A complete replacement for Minecraft's renderer

### Development status
The Nova Renderer is currently early in development, so early that I don't think there's anything worth releasing. Yes, you can download the code and try running it yourself, but you have to make some changes to the Minecraft source code to add in hooks for Nova.

#### Roadmap
Here's the things I plan to implment, and the version that they'll be implemented in:

* 1.0: Full Shaders Mod compatibility. You'll be able to take existing shaderpacks, like SUES or Chocapic, and use them in the Nova Renderer. They'll Just Work (tm)
* 1.1: Light shaders for block lighting. Shaderpack suthors will be able to control every aspect of light coming from emissive blocks like torches, glowstone, lava, redstone lamps... any of them. 
 
The Nova Renderer is a complete replacement for Minecraft's default renderer. Minecraft's renderer is nice and all, but it's written in Java (ugh) using the ancient OpenGL 2.1 API. Additionally, the Minecraft Shaders Mod does a number of things in strange ways, and, while incredible, isn't the best-designed code. This mod seeks to fix those problems.

### Built to run fast
The Nova Renderer is written in C++ for a couple reasons. First and foremost, I like C++ better than Java. The way I see it, Java treats you like a child, like you'll hurt yourself if left alone too long, while C++ respects you as an adult and trusts you to make your own decisions. Additionally, optimized C++ can run faster than Java since C++ doesn't require the overhead of a virtual machine (Although the JVM is much better than it used to be).

The choice of language is not the only performance-oriented decision I made. The architecture of the code itself also lend itself to speed. I utilize data-oriented-design to design my data structures such that the CPU can deal with them really quickly. I also take full advantage of OpenGL 4.5's features, like the glMultiDrawIndirect function, in order to ensure as little driver overhead as possible.

### Full of features
The Nova Renderer has a

### Installation ###
Coming soon ... 
