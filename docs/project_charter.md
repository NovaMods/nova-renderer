# Nova Renderer

This document first summarizes the goals of Nova Renderer.

**Note: this document assumes that you're familiar with writing Optifine shaders.**

## Continued ideas

Nova's major goals are of equal importance. All other goals must support at least one of these, and the more they 
support, the better.

### High-performance, real-time rendering engine

Nova is, at its core, a high-performance real-time rendering engine. Nova must run quickly and efficiently, doing the 
minimum amount of work possible. Nova must take full advantage of the hardware it's running on, and must be designed to 
scale gracefully to future hardware with abilities considered unobtainable today.

#### Written in a low-level native language

Nova must be written in a low-level, native language because Nova needs very fine-grained control over the CPU and its
memory layout in order to squeeze every possible bit of speed out of the hardware.

#### Use explict graphics APIs

Nova must use explicit graphics APIs. Explicit APIs both give more control over the GPU and have lower CPU overhead,
 making them the only choice for Nova.

##### Use explicit graphics APIs _directly_

Nova intentionally uses the graphics apis directly because we need as much control over the hardware as possible.

#### Use task-based multithreading

Nova must run well on the CPUs of today as well as the CPUs of tomorrow. While CPU clock speed increases, if that ever
 happens, will  automatically make Nova run faster, increased core count will not. Given recent trends in CPU 
 development, designing for an arbitrary number of threads seems most prudent. Thus, Nova must use task-based 
 multithreading, where small, compartmentalized tasks are sent to a thread pool, the size of which is equal to the
  number of logical CPU cores. This will allow Nova to scale for future hardware in a meaningful way.

#### Scale across multiple GPUs

Nova must scale across the computer's available GPUs. Nova should be able to utilize multiple GPUs at once to deliver 
faster frames to the user. Accomplishing this will be a joint effort between Nova developers and shaderpack developers: 
While I _do_ think it'll be possible to scale automatically, Nova should also provide an interface so that shaderpack 
authors can say "run this on GPU 1" or "run this on any GPU".

#### Custom memory management

Nova should have custom memory management throughout, to ensure that the memory Nova uses is laid out in a CPU 
cache-optimal way. While C++ standard library has fishy support for allocators, the rest of Nova 
certainly can use custom allocators whenever possible.

### User-customizable

Nova is, at its core, a highly customizable rendering engine. Nova must provide a number of ways to be customized, and 
must provide an elegant and efficient customization interface. Nova must be both accessible to newcomers and powerful 
enough for masters. It must provide control in every sense of the word.

#### Run in Minecraft

I created this project as a replacement renderer for Minecraft. While Nova is now going to be useful for many different 
games, Nova will first be a Minecraft renderer. Usefulness for Minecraft must be a primary consideration.

##### Compatible with Optifine shaders

Nova must be able to load Optifine shaders. It must load existing popular shaderpacks like SEUS, Molly, and Continuum 
2.0 and must run those shaders as close to Optifine shaders as possible - except for the bugs. Nova should not have the
same bugs as Optifine.

##### Compatible with Bedrock shaders

Nova must be compatible with Bedrock shaders. This will be hard to design for, because there's no official way to make 
shaders for the Bedrock engine, but Nova's shaderpack format is based on the unoffical way to make Bedrock shaders.

#### Design a new render graph-based shaderpack format

Nova will give more control to shader developers than Optifine. Nova will use a shaderpack format based on the notion of
a render graph. A render graph has multiple render passes, each of which declared the resources that they read from, the 
resources that they write to, and (optionally) information about what GPU to run the render pass on. From that 
information Nova will figure out the most optimal order for the render passes and the most optimal GPUs to run them on, 
if the user has multiple GPUs. It will then render the current scene using this render graph.

In addition to this implementation of a render graph, Nova will have a number of other concepts in its shaderpack:

- Pipeline. Pipelines roughly correspond to a shader in the Optifine shader model, but they let you set all the 
rasterizer state such as if MSAA is enabled, what the stencil test is like, if there's any blending, etc.
- Materials. A material has one or more material passes.
- Material pass. A materials pass is a pipeline, and bindings of resources to that pipeline. Nova will provide a number 
of builtin resources, such as resources to access the swapchain and all the model matrix buffers, and will let users 
bind their own custom resources as well. Resources will be bound to shader variables by associating the resource's name 
with the variable's name.
- Material bindings. Material bindings associate materials with objects by letting the user write a 
simple expression, called a geometry filter, to select what in-game objects they want to use that material.
- Resources. A resource can be either a buffer or a texture. If it is a buffer, it has a name, a scalar size, and a 
type. If it is a texture, it has a name, a vector size including dimensionality, (1D, 2D, 3D, etc), and a type.
  - Buffers may be bound to shader uniform variables which take UBO/CBV or SSBO/UAV inputs. Only buffers smaller than 
  64kb may be bound to a UBO/CBV shader uniform variables, while buffers of any size may be bound to a SSBO/UAV shader 
  uniform variables. Buffers may also be used as the read or write resources of a renderpass.
  - Textures may be either loaded from disk or dynamically generated by shaders. Both loaded textures and dynamic 
  textures may be bound to shader texture uniform variables, while only dynamic textures may be used as the read or
  write resources of a renderpass.
- Options. These will likely be expressed in the Optifine shader options syntax, with optional syntax that's nicer.

#### Run in Fallout: New Vegas

After bringing Nova to Minecraft, I want to bring Nova to Fallout: New Vegas, because that game is in serious need of a 
complete graphical overhaul. F:NV support won't be a priority until after Minecraft support is complete, but developers
 should keep in mind that Nova will eventually be used in F:NV, and thus shouldn't be specific to any one game.

### Fun

I started Nova because I thought it'd be fun. First and foremost, Nova is fun. If Nova once again becomes incredibly 
un-fun, I will either reboot it again or drop it. An unfun Nova isn't worth developing, even if it would fit the other 
two goals better.

#### Vulkan-API

Nova will support one API: Vulkan. This is because dealing with more than one API has lead to wasted time due to designing abstractions instead of actual development.

## New

### Project charter document

One major issue with newcomers working on Nova was that they didn't know all the goals for the project. This was my own 
fault, since I didn't effectively communicate all my goals. For this project, I decided to write a Project Charter, the 
very one you're reading right now, so that I could point newcomers somewhere and say "there's all the goals of Nova. 
Work towards them". It will also help me keep these goals in mind and consider them more strongly in the future.

### Feature branches

When developing C++ Nova, we should use feature branches, where features are as small as possible. I expect we'll have lots
of merges for one "feature" - for example, the RHI structs can be one merge, the Vulkan implementation another, the DX12
implementation yet another...
