# Vulkan starter

Learning how Vulkan API makes graphics.

<img src="https://pbs.twimg.com/media/GioHlduXoAACmxc?format=jpg&name=medium" alt="Vulkan meme" width="384">

## Notes

### Index buffers and vertex buffers

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

