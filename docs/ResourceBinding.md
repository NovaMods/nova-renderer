How does Nova bind resources?

Well

There's a few kinds of resources that Nova binds

# Camera matrices

Nova stores all camera matrices in one large buffer. Each shader can index into this array however it wants to know 
what camera matrices it should use

Cameras may be changed however you want. Right before Nova submits its command list to the GPU, it loops through all
active cameras, calculates their matrices, and writes those to the camera buffer. This allows render passes in the
render graph to update camera data - for instance, the UI render pass might update its camera to reflect screen
resizing

# Textures

Nova stores all its textures in a single large array. The array of textures is set to the GPU before the render graph
is executed. This makes the binding code a bit simpler, at the expensive of one frame of latency for any texture
streaming accomplished in a render pass

# Material data

Material data, like camera matrices, is stored in a single large buffer. However, material data is different in that 
each shader views the material data buffer as an array of its ow material struct. I do some trickery with the material
buffer to make this possible - when you need to upload data for your material, Nova makes an aligned allocation from 
the material buffer for the space you need

Example:

```cpp
struct MaterialA {
    float4 albedo;
};

struct MaterialB {
    float4 albedo;
    float4 specular;
};

const auto& [idx1, material1] = nova.allocate_material<MaterialA>();
const auto& [idx2, material2] = nova.allocate_material<MaterialA>();
const auto& [idx3, material3] = nova.allocate_material<MaterialB>();
```

The material buffer might look like this:

```
|          material1             |          material2             |                                material3                       |
|********************************|********************************|****************************************************************|
```

`idx1` is 1, `idx2` is 2, and `idx3` is also 2 - but when you index into the material buffer assuming it's an array of 
`MaterialB` instances, you'll get the material you expect

Hopefully I explained that...

## Materials that use textures

When a material needs to refer to a texture, it's material data struct should use the `TextureId` type. This is an 
opaque handle into a texture in the texture array (a `uint32_t` in practice). In theory any shader can read from any 
texture it wants, but in practice the order of textures in the array isn't well-defined and you'll probably just cause 
yourself a lot of headaches if you try to guess at texture IDs

# Samplers

Nova currently provides three samplers - point, bilinear, and trilinear. None of these wrap their UV coordinates. 
Hopefully in the future I'll get cool enough to have better samplers

# Render targets

Shaders in Nova may sample render targets. Simple declare a `Texture2D` shader variable with the name of the render 
target you wish to sample (these names are the same as the names of the render targets in your renderpack resources 
file). Nova will read that variable and bind the render target to your shader whenever it's used

There's an assumption here that you need to access render targets in unique shaders, such as post-processing or 
deferred lighting shaders, and that material shaders won't need to access any render targets. As long as you don't
break that assumption, both our lives will be easier
