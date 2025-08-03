<!-- TOC -->

- [Index buffers and vertex buffers](#index-buffers-and-vertex-buffers)
- [Descriptors](#descriptors)
    - [🔍 Big Picture: Why Descriptors?](#-big-picture-why-descriptors)
    - [🧱 Key Concepts](#🧱-key-concepts)
        - [1. Descriptor](#1-descriptor)
        - [2. Descriptor Set Layout](#2-descriptor-set-layout)
        - [3. Descriptor Pool](#3-descriptor-pool)
        - [4. Descriptor Set](#4-descriptor-set)
    - [🧪 Analogy](#🧪-analogy)
    - [🛠 Why So Complicated?](#🛠-why-so-complicated)

<!-- /TOC -->

# Index buffers and vertex buffers

A vertex buffer is a GPU memory buffer that stores vertex data, such as:

- Positions
- Normals
- Texture coordinates
- Colors
- Tangents, bitangents, bone weights, etc.

Such as data that can be read from an .fbx file.

An index buffer (or element buffer) is a buffer of integers (indices) that reference positions in the vertex buffer. So it has to exist when there is a vertex buffer.

For example, if we have:

Vertex Buffer
```
0: (x0, y0, z0)
1: (x1, y1, z1)
2: (x2, y2, z2)
3: (x3, y3, z3)
```

, and they are actually 4 adjacent points on a quad (not necessarily planar), then we can have index Buffer:
```
[0, 1, 2, 0, 2, 3]
```

. And we can use indices to refer to the vertices like this:

```
Triangle 1: uses vertices 0, 1, 2
Triangle 2: uses vertices 0, 2, 3
```

, instead of having to store (x0, y0, z0) and (x2, y2, z2) both 2 times.

# Descriptors

## 🔍 Big Picture: Why Descriptors?

In Vulkan, **shaders don't access GPU resources (like buffers or textures) directly.** Instead, you describe what resources the shaders will access via *descriptors*, and then *bind* those to the pipeline at draw time.

Descriptors act like "pointers" in GPU space — telling the GPU:
> "Hey, this is the uniform buffer (or image) the shader should use."

## 🧱 Key Concepts

### 1. Descriptor

A **descriptor** describes a specific GPU resource used in a shader — for example:

- A **uniform buffer** (`VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER`)
- A **combined image sampler** (`VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER`)
- A **storage buffer**, etc.

Each descriptor is like a *slot* in a binding table.

### 2. Descriptor Set Layout

This defines the **interface between your application and the shader** — i.e., what kind of resources your shader expects, and in which binding slots.

**Example:**

```glsl
layout(set = 0, binding = 0) uniform Transformations { ... } ubo;
layout(set = 1, binding = 0) uniform sampler2D tex;
```

In Vulkan, this is reflected in a `VkDescriptorSetLayout` with:
- 1 binding for a uniform buffer (binding 0)
- 1 binding for a combined image sampler (also binding 0, but in a different set)

You create these with `vkCreateDescriptorSetLayout(...)`.

### 3. Descriptor Pool

A `VkDescriptorPool` is like a memory allocator **for descriptor sets**.

Because Vulkan is explicit, you can't just say "give me a descriptor set" — you must first reserve space in a pool:
- You allocate descriptor sets from it.
- You *must specify* how many of each type of descriptor it can contain (e.g., 100 samplers, 50 UBOs).

You create it with `vkCreateDescriptorPool(...)`.

### 4. Descriptor Set

A `VkDescriptorSet` is a **specific instance** of a descriptor layout with concrete resource bindings.

Once allocated, you fill it with actual GPU resource handles using `vkUpdateDescriptorSets(...)`, for example:
- Set binding 0 to a `VkBuffer`
- Set binding 1 to a `VkImageView` and `VkSampler`

Then you bind this descriptor set before drawing using `vkCmdBindDescriptorSets(...)`.

## 🧪 Analogy

Think of it like programming a smart robot chef:

| Vulkan Concept | Analogy |
|---|---|
| **Descriptor Set Layout** | The *recipe* that says "this dish needs a pot (binding 0) and a frying pan (binding 1)" |
| **Descriptor Set** | A specific *set of cookware* on the counter |
| **Descriptor Pool** | A *kitchen warehouse* from which you draw the tools |
| **Descriptor** | The *individual tools* (like a pot or pan) |
| **Binding Descriptors** | Telling the chef, "use this pot for the soup" |

## 🛠 Why So Complicated?

Because Vulkan is **designed for performance and control**:

- You can preallocate and reuse descriptor sets.
- You can batch descriptor updates.
- You avoid implicit state or background memory management (common in OpenGL).

This makes Vulkan faster and more scalable for large applications (e.g., AAA games, simulation tools).