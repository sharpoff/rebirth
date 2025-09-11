# General
* Speed up model loading of big scenes(like sponza)
	- Add multithreading
	- Convert textures to dds/ktx?
* Replace GLM with my math lib (also add tests and perf checks for it)
* Add object picking to see object's properties. Also add gizmo to transform an object
* Audio
* Add profiler (Tracy?)

# Renderer
* CSM shadows
* my own simple immediate mode gui implementation.
* add IBL into PBR
* Add different types of lighting (point, spot)
* Postprocessing (hdr, tone mapping, etc.)
* Ray tracing GI(DDGI?). But if hardware doesn't support RT - make a tool to bake it as lightmaps for static objects and use other realtime techniques for dynamic shadows and stuff. Otherwise use it real-time, but it could be harder to implement.
* SSAO
* Bloom
* Text rendering

* Transparency
* Decals
* Particles
* Procedural terrain generation (noise)

* Foliage
* Clouds
* Ocean (fft)
* Fur (shell texturing?)
