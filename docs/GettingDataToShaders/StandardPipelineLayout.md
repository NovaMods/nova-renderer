# Standard Pipeline Layout

When you make a pipeline for use in Nova, you have to conform to certain conventions for Nova to be able to use it. Those conventions some come in the form of descriptors that Nova uses for a specific purpose

Pipelines that render static (non-animated) meshes must have the model matrix UBO in a descriptor set by itself

Pipelines that render animated meshes must have the model matrix UBO and the animation data UBO in a descrptor set with nothing else in that set. The model matrix UBO must be first, then the animation data UBO
