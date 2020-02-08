# How do you descriptor set?

Descriptor sets have vexed me at every step of development. They're new and different and they have a lot of rules which aren't all that obvious. This document will hopefully lay out everything in one convenient place that I - and also you - can refer to

First, let's talk about what we're trying to do

## Use Case

Most renderers need some way for shaders to access resources like textures, buffers, etc. For the Vulkan API, this way is the almighty descriptor set. Descriptor sets, as I understand them, are essentially a pointer to a resource. You update your descriptor sets with your resoruces, then you bind the descriptor sets to your command buffer, then shaders involved in sunsequent drawcalls can look at the descriptors to know what resources they should actually read from. I'm not entirely sure why there's this indirection - and in fact, on AMD GPUs descriptor sets are actually just pointers - but the indireciton exists, and we all have to find a way to deal with it

So, how do we update descriptor sets with our resources? How do we bind descriptor sets to a command buffer? Where do the descriptor sets live?

## Where do descriptor sets live?

Before getting into updating and binding descriptor sets, we're going to talk about where they live. Every developer I've talked to has had a different answer for this question. My answer makes sense for me, but you may want to do something different. Such is Vulkan.

My renderer has a concept of a Material. My Materials have a VkPipeline, and a bunch of VkDescriptorSets that represent all the resources needed by the Material. This includes things the artists care about, like textures and artist-tunable material parameters, along with things like the camera matrices, per-frame information like the current time and player's position, etc. The idea is that I can bind each Material's descriptors atomically, then issue drawcalls for everything using that Material, then bind the next Material's descriptors, all without worrying about only updating some of the descriptors. This is somewhat ineffecient and I'm binding more than I need to, but for now it's fine

I tried having my resoruce own their own descriptor sets. This initially made sense to me, until I realized that descriptor sets were tied to the pipeline layout they're created from. Having one descriptor per resource didn't work, but having one descriptor per resource _per pipeline_ made more sense

## Descriptor Indexing Guide

### Creating your VkDevice

You need to enable descriptor indexing features through the `VkPhysicalDeviceDescriptorIndexingFeatures` struct. This struct should be in the `pNext` chain of your `VkDeviceCreateInfo`. You should only enable the features that you'll actually need. Nova enables dynamic indexing of sampled image arrays, variable descriptor counts, and partially bound descriptors. This allows me to have a single large array of textures to use for texture streaming

```cpp
VkPhysicalDeviceDescriptorIndexingFeatures descriptor_indexing_features{};
descriptor_indexing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;

// Enable non-uniform indexing
descriptor_indexing_features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
descriptor_indexing_features.runtimeDescriptorArray = true;
descriptor_indexing_features.descriptorBindingVariableDescriptorCount = VK_TRUE;
descriptor_indexing_features.descriptorBindingPartiallyBound = VK_TRUE;

VkDeviceCreateInfo device_create_info = ...;
device_create_info.pNext = &descriptor_indexing_features;

VkDevice device;
vkCreateDevice(phys_device, &device_create_info, nullptr, &device);
```

### Creating the descriptor sets

#### Unbounded (variable size) descriptor arrays

Nova uses an unbounded descriptor array for its texture resources. This allows Nova to stream in textures and add them to the array as needed. Materials use indexes to refer to which texture in the array they use. [The Chunk Stories blog](http://chunkstories.xyz/blog/a-note-on-descriptor-indexing/) has a wonderful explaination of why this is a good strategy, I agree with all the points they make

Vulkan has a restriction that an unbounded array must be the last descriptor in the set. This is kinda annoying, but it's something that I can easily check with shader reflection so no worries

You have to tell Vulkan which descriptors will be a variable size array when you create your `VkDescriptorSetLayout`. To do this, you use the `VkDescriptorSetLayoutBindingFlagsCreateInfo` struct. This struct should be in the `pNext` chain of your `VkDescriptorSetLayoutCreateInfo` struct. `VkDescriptorSetLayoutBindingFlagsCreateInfo` has an array of flags for each binding in the descriptor set. Any descriptors which can be an unbounded array must have the flag `VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT` at their index in the flags array. For example, if binding 3 is an unbounded array, descriptor, then `VkDescriptorSetLayoutBindingFlagsCreateInfo.pBindingFlags[2]` must have `VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT`

When one of the descriptors is an unbounded array, the `descriptorCount` member of the `VkDescriptorSetLayoutBinding` struct is an upper bound on the size of the array. This means that the array isn't truly unbounded, but near as I can tell most people say "unbounded", so for the same of convention I use that word as well

#### Partially populated (bound) descriptors

We enabled partially populated descriptors when creating our VkDevice, but we also have to tell Vulkan which descriptors we plan to partially populate

#### Example code

```cpp
VkDescriptorBindingFlags flags[3];
flags[0] = 0;
flags[1] = 0;
flags[2] = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags{};
binding_flags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
binding_flags.bindingCount = 3;
binding_flags.pBindingFlags = flags;

VkDescriptorSetLayoutCreateInfo create_info = ...;
create_info.pNext = &binding_flags;

VkDescriptorSetLayout layout;
vkCreateDescriptorSetLayout(device, &create_info, nullptr, &layout);
```

### Binding resources to unbounded array descriptors
