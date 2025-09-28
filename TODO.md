# Current tasks
* Indirect drawing
* Frustum culling
* Depth prepass and z-sort to reduce overdraw. Z-sorting also comes in handy with transparency.
* Fix directional light shadows
* CSM shadows
* Job system for parallel tasks execution. Speed up scene loading. Maybe add loading screen after that
* Fix sync validation errors
* Reduce skybox overdraw by drawing it last with depth set to 1.0

# After current tasks
* Convert textures to ktx, add support for that
* Integrate meshoptimizer. Also tryout it's gltfpack tool(it can also convert textures to ktx format)
* Add object picking to see object's properties. Also add gizmo to transform objects
* Chracter controller (jolt's virtual character)
* Probably i should separate static and dynamic meshes in drawScene, so i don't have to reupdate meshDraws(separate it to dynamic and static) for static meshes.

# General
* ECS
* Audio support
* Gamepad support?
* DDS texture format support
* Ragdolls and constraints
* Build basic AI citizens that would walk around.
* Merge scenes

# Renderer
* Create several ubo/ssbo/etc per *frame in flight*. Currently not all needed objects are created for every *frame in flight*.
* Implement proper LODs
* Add different types of lighting (point, spot)
* add IBL PBR
* Postprocessing (hdr, tone mapping, etc.)
* Ray tracing GI(DDGI?). Baked or not?
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
