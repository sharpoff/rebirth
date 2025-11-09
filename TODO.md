# Current tasks
* EDITOR!!!
	- Gizmo, object manipulation
	- Level (de)serialization (should create level file format)
	- Object adding/removing in runtime
	- Probably i should create custom mesh(or prefab) format. Also i could write custom meshes to gltf files using cgltf write
* Frustum culling (kinda working?)
* Fix PBR and lighting(it feels too dark), add tone mapping
* CSM shadows
* Job system for parallel tasks execution. Speed up scene loading. Maybe add some kind of loading screen system after that

# Later tasks
* Fix sync validation errors
* Add fullscreen mode
* GI
* Convert textures to ktx
* Player controller (jolt's virtual character)
* Depth prepass and z-sort to reduce overdraw. Z-sorting also comes in handy with transparency.

# Maybe?
* Draw indirect
* Integrate meshoptimizer for LOD generation?


==================
Renderer structure implementation:

class RenderDevice(interface?):
	* Holds generic api for resource creation(buffers, textures, swapchain, etc), command submition, and stuff like that

* Create generic classes for Textures, Buffers and stuff like that, backends will inherit from them and add their implementation

Vulkan backend structure:

1. class VulkanRenderDevice(inherits from RenderDevice):
	* Implement Vulkan specific stuff with api from RenderDevice
	* Uses buffer address that passed using push constants(or ubo?)
	* Uses indirect draws
	* Compute culling

TODO:
	* Create architectrue above with simple 1 pass renderer
