# Bedrock Shaders Documentation

Bedrock Shaders implement a customizable, data-driven rendering pipeline that allows end users to provide resourcepacks to tell Minecraft how to draw different objects. Each object type has a .material file which sets up the rendering pipeline state, specifies what shaders to use, and defines a number of rendering states that can be used to render objects of that type. These .material files provide a high degree of customizability.

## .material files

A .material file is a JSON file which tells Bedrock how to render a given object. We’re going to step through one of the default .material files to get a better example of how it works. The full file is available [here](https://github.com/jocopa3/Materials/blob/master/sky.material), but I’ll reproduce the relevant parts of it.

### States

```json
{
  "sun_moon": {
    ...
  },

  "sun_moon_below_water:sun_moon": {
    ...
  },

  "stars": {
    ...
  },

  "stars_below_water:stars": {
    ...
  },

  "skyplane": {
    ...
  },

  "skyplane_below_water:skyplane": {
    ...
  },

  "end_sky": {
    ...
  }
}
```

The first thing one might notice when looking at a .material file is that each file contains a number of top-level JSON objects. This file, `sky.material`, defines states to use when rendering the sky. Each one corresponds to a specific object in Minecraft: `sun_moon` is used for the sun and moon, `stars` is used for the stars, `skyplane` is used for the skyplane, and `end_sky` is used for the end sky. That's simple enough.

The other three states are slighly more complex. The `:` in the middle of the state name can be read as "inherits from". The state `sun_and_moon_below_water` inherits from the state `sun_moon`, which means that `sun_and_moon_below_water` has all the attributes and bits of rendering pipeline state set in `sun_moon`, plus whatever is defined in `sun_and_moon_below_water`. Attributes set in a child state override those in the parent state.

Let's step through one of these states to get an idea of how they work

### Rendering states

```json
{
    "sun_moon": {
        "states": [
            "DisableDepthWrite",
            "DisableAlphaWrite",
            "Blending"
        ],

        "blendSrc": "SourceAlpha",
        "blendDst": "One",

        "vertexShader" : "shaders/uv.vertex",
        "vrGeometryShader": "shaders/uv.geometry",
        "fragmentShader" : "shaders/texture_ccolor.fragment",

        "vertexFields": [
            { "field": "Position" },
            { "field": "UV0" }
        ],
        "msaaSupport": "MSAA"
    }
}
```

Rendering pipeline states are set in the `states` array of a state. These tell the renderer what tests should be enabled or disabled, which faces should be culled, and much more. In this example, `DisableDepthWrite` means that the `sun_moon` state won't write to the depth buffer, `DisableAlphaWrite` means that the `sun_moon` state won't write alpha, and `Blending` means that alpha blending is enabled for the `sun_moon` state. Many other rendering states are available.

### Blending Function

```json
{
    "sun_moon": {
        ...
        "blendSrc": "SourceAlpha",
        "blendDst": "One",
        ...
    }
}
```

The blending function is controlled through the `blendSrc` and `blendDst` fields. `blendSrc` tells Bedrock how to interpret the source alpha value, while `blendDst` tells Bedrock how to interpert the destination alpha value. Blending is explained in detail on the [Khronos wiki](https://www.khronos.org/opengl/wiki/Blending), and is the same for OpenGL and DirectX.

### Shaders

```json
{
    "sun_moon": {
        ...
        "vertexShader" : "shaders/uv.vertex",
        "vrGeometryShader": "shaders/uv.geometry",
        "fragmentShader" : "shaders/texture_ccolor.fragment",
        ...
    }
}
```

.material files let you specify what shaders to use for a given state, and this snippet shows you how to do that. There's theee shaders supported right now: vertex shaders, geometry shaders, and fragment shaders. Currently, geometry shaders are mostly useful for VR although I expect shaderpack developers to find some really creative things to do.

As one might expect, `vertexShader` specifies the filepath to the vertex shader, `vrGeometryShader` specifies the filepath to the geometry shader to use in VR, and `fragmentShader` specifies the fragment shader to use.

The Bedrock engine runs on a variety of platforms, and the shaders need to be written in the correct shading language for the target platform. There's currently no logic to handle this in Bedrock, though, so you'll need to write separate shaders for each platform you want your shaderpack to support.

### Vertex fields

```json
{
    "sun_moon": {
        ...
        "vertexFields": [
            { "field": "Position" },
            { "field": "UV0" }
        ],
        ...
    }
}
```

The `vertexFields` array specifies what vertex attributes your shaders need. In this example, the shaders take the vertex `Position` and `UV0`, which is the primary UV channel. This is presumably used by Bedrock to build vertex buffers to feed into the shaders.

### MSAA

```json
{
    "sun_moon": {
        ...
        "msaaSupport": "MSAA"
    }
}
```

The last thing set up in the `sun_moon` state is Multi-Sample AntiAliasing (MSAA). It's enabled for the `sun_moon` state, but it can also be disabled if needed.

## Summary

We've now walked through a .material file. Hopefully you now have a basic handle on how .material files work. I'm going to try and add full documentation for what all can be done in a .material file, but this should work for now.