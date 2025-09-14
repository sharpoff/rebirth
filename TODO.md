# Current tasks
* Add Tracy profiler. Why subway station is lagging?
* Add object picking to see object's properties. Also add gizmo to transform objects
* Implement frustum culling
* Build basic AI citizens that would walk around.
* Dont render when app is minimized.
* Fix directional light shadows
* Chracter controller (jolt virtual character)
* Ragdolls and constraints

# General
* Speed up model loading of big scenes(like sponza)
	- Add multithreading
	- Convert textures to dds/ktx?
* Audio support
* Gamepad support?

# Renderer
* CSM shadows
* Add different types of lighting (point, spot)
* add IBL PBR
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
