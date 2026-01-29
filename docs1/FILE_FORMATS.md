# Billyprints File Formats

## Scene Files (.bps)

Scene files store the complete node graph layout including all nodes and their connections.

### Format: BPS Version 2 (Current)

```
HEADER
  Offset  Size     Description
  0       4        Magic number: "BPS2" (ASCII)

DEPENDENCY SECTION (New in V2)
          size_t   Custom gate type count

  For each custom gate type used:
          size_t   Type name length
          N bytes  Type name (e.g., "HalfAdder", "MyXOR")

NODES SECTION
          size_t   Node count

  For each node:
          size_t   Type name length
          N bytes  Type name (e.g., "AND", "NOT", "In", "Out", or custom gate name)
          8 bytes  Position (ImVec2: 2 x float32)
          int      Input slot count
          int      Output slot count

CONNECTIONS SECTION
          size_t   Connection count

  For each connection:
          int      Input node ID (0-indexed, matches node order)
          size_t   Input slot name length
          N bytes  Input slot name (e.g., "in", "in0", "in1")
          int      Output node ID
          size_t   Output slot name length
          N bytes  Output slot name (e.g., "out", "out0", "out1")
```

### Format: BPS Version 1 (Legacy)

```
HEADER
  Offset  Size     Description
  0       4        Magic number: "BPS1" (ASCII)

NODES SECTION
  4       size_t   Node count

  For each node:
          size_t   Type name length
          N bytes  Type name
          8 bytes  Position (ImVec2: 2 x float32)

CONNECTIONS SECTION
          size_t   Connection count

  For each connection:
          int      Input node ID
          size_t   Input slot name length
          N bytes  Input slot name
          int      Output node ID
          size_t   Output slot name length
          N bytes  Output slot name
```

**Note:** BPS1 files are still supported for loading (backward compatibility). New saves always use BPS2.

### Missing Gate Handling (BPS2)

When loading a BPS2 scene with missing custom gates:

1. **Warning banner** appears listing missing gate names
2. **Placeholder nodes** are created for missing gates:
   - Red/maroon color scheme
   - Title shows "? GateName"
   - Connections preserved
   - Slot counts preserved (from BPS2 format)
3. **Automatic upgrade** - Loading the gate library converts placeholders to real gates

### Limitations

- **Custom gate definitions are NOT embedded** - Load `.bin` files before scenes
- **PinIn states not saved** - All inputs reset to default on load
- **Canvas zoom/pan position not saved**

### Usage

- **Save:** File > Save Scene (Ctrl+S)
- **Load:** File > Open Scene (Ctrl+O)

---

## Custom Gates Files (.bin)

Custom gate files store user-defined gate definitions that can be reused across scenes.

### Format

```
HEADER
          size_t   Gate definition count

FOR EACH GATE DEFINITION:

  Name & Color:
          size_t   Name length
          N bytes  Name string (e.g., "MyXOR", "HalfAdder")
          ImU32    Color (4 bytes, RGBA format)

  Internal Nodes:
          size_t   Node count

    For each node:
          size_t   Type name length
          N bytes  Type name
          8 bytes  Position (ImVec2)
          int      Node ID (internal reference)

  Internal Connections:
          size_t   Connection count

    For each connection:
          int      Input node ID
          size_t   Input slot name length
          N bytes  Input slot name
          int      Output node ID
          size_t   Output slot name length
          N bytes  Output slot name

  Interface Pins:
          size_t   Input pin count
          N ints   Input pin node IDs (references to PinIn nodes)
          size_t   Output pin count
          N ints   Output pin node IDs (references to PinOut nodes)
```

### Usage

- **Save:** File > Save Custom Gates...
- **Load:** File > Load Custom Gates...

---

## Data Types Reference

| Type    | Size (bytes) | Description                          |
|---------|--------------|--------------------------------------|
| size_t  | 8            | Unsigned integer (64-bit on x64)     |
| int     | 4            | Signed integer (32-bit)              |
| float   | 4            | Single-precision floating point      |
| ImVec2  | 8            | Two floats (x, y)                    |
| ImU32   | 4            | Unsigned 32-bit color (RGBA)         |

---

## Best Practices

1. **Save custom gates before scenes** - If you create custom gates and use them in a scene, save the gates file first
2. **Load custom gates before scenes** - When opening a project, load the `.bin` file before the `.bps` file
3. **Use matching filenames** - Consider naming related files similarly (e.g., `myproject.bps` and `myproject_gates.bin`)
