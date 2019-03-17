# Materials

Materials in Nova are a bit lower-level than materials in other engines. In something like Unity you don't really need
to worry about animated meshes, since Unity abstracts that away. In Nova, however, your pipelines must care if they're
going to be used with an animated or a static mesh. You must explicitly bind the animation data UBO in your material's
bindings, although Nova provides some helper functions to deform the mesh