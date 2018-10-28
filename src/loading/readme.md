Holds all files relating to loading resources (shaders or otherwise) from the filesystem

The code in this folder shouldn't have any calls to API-specific thing. We should massage all input code into SPIR-V.
The rendering backends can transpile from that to whatever they need