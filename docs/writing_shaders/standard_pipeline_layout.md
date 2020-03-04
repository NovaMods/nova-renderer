# Standard Pipeline Layout

Nova expect that your pipelines will have a specific pipeline layout. Using this layout ensures that your pipelines will be usable by Nova's builtin render passes. if you don't use Nova's standard pipeline layout for whatever reason, you'll have to make a custom render pass to bind data to your pipeline

To make life easier for developers, Nova provides a file you can include to use the standard pipeline layout. Simply include `/nova/shaders/builtin/standard_pipeline_layout.hlsl`

The rest of this document explains the contents of that file

## High Level

Nova's standard pipeline layout facilitates Nova's material model

Nova's material model has two distinct concepts: materials and material instances. Materials are a pipeline and the parameters to that pipeline, and material instances are arguments for the pipeline. This is pretty heavily inspired by Unreal Engine 4

Nova creates one buffer for each material. This buffer is large enough to hold all the parameters for all the material instances of that material. For instance, if a material has three `float4` parameters and five material instances, the material's buffer will have enough space for fifteen `float4`s

Nova also stores all the textures in an unbounded descriptor array. When a material has a texture parameter, Nova will upload that texture to a texture array. The material instance will have an integer for each texture, which your shaders can use to index into the texture array

## Show me the code

```hlsl
/*!
 * \brief All the push constants that are available to a shader that uses the standard pipeline layout
 */
[[vk::push_constant]]
struct StandardPushConstants {
    /*!
     * \brief Index of the camera that the current drawcall is rendering to
     */
    uint camera_index;

    /*!
     * \brief Index of the material buffer that holds the material data for this shader
     */
    uint material_index;

    /*!
     * \brief Index of the material instance that holds the material parameters for this shader
     */
    uint material_instance_index;
} constants;

/*!
 * \brief Point sampler you can use to sample any texture
 */
[[vk::binding(0, 0)]]
SamplerState point_sampler;

/*!
 * \brief Bilinear sampler you can use to sample any texture
 */
[[vk::binding(1, 0)]]
SamplerState bilinear_filter;

/*!
 * \brief Trilinear sampler you can use to sample any texture
 */
[[vk::binding(2, 0)]]
SamplerState trilinear_filter;

/*!
 * \brief Array of all the textures that are available for a shader to sample from
 */
[[vk::binding(0, 1)]]
Texture2D textures[];

/*!
 * \brief Array of all the buffers for all of Nova's materials
 */
[[vk::binding(0, 2)]]
ConstantBuffer material_buffers;
```
