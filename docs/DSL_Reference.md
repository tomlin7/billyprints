# Billyprints DSL Reference

Billyprints uses a simple text-based Domain Specific Language (DSL) to describe digital logic circuits. The script is bidirectional: changes in the script update the visual graph, and changes in the graph update the script.

## Core Concepts

The script defines two main things:
1. **Nodes**: Instances of gates or components.
2. **Connections**: Wires linking outputs to inputs.

## Syntax

### 1. Defining Nodes

Nodes are defined using the `@` symbol.

**Format:**
```
[Type] [Identifier] @ [X], [Y] [Flags]
```

- **Type**: The class of the node (e.g., `AND`, `OR`, `NOT`, `In`, `Out`, or a Custom Gate name).
- **Identifier**: A unique name for this instance (e.g., `n1`, `gateA`, `my_switch`).
- **X, Y**: The position of the node on the canvas (integers).
- **Flags**: Optional modifiers.
    - `momentary`: Only valid for `In` nodes. Makes the button a momentary push-button instead of a toggle switch.

**Examples:**
```
AND gate1 @ 100, 200
In sw1 @ 50, 50 momentary
Out led1 @ 300, 150
NAND logic_gate @ 400, 400
```

### 2. Defining Connections

Connections define how signals flow between nodes.

**Format:**
```
[SourceIdentifier].[OutputSlot] -> [TargetIdentifier].[InputSlot]
```

- **SourceIdentifier**: The ID of the node sending the signal.
- **OutputSlot**: The specific output pin (usually `out` for standard gates).
- **TargetIdentifier**: The ID of the node receiving the signal.
- **InputSlot**: The specific input pin (e.g., `in1`, `in2` for gates, or `in` for outputs).

**Shorthand:**
If a node has a default input/output (like standard gates), you can omit the slot name.
- `gate1 -> gate2` is equivalent to `gate1.out -> gate2.in` (simplification depends on node defaults).
- `In` nodes default to `out`.
- `Out` nodes default to `in`.

**Examples:**
```
// Standard Connection
gate1.out -> led1.in

// Connecting specific slots
sw1.out -> gate1.in1
sw2.out -> gate1.in2

// Implicit/Shorthand (if supported by context)
sw1 -> gate1
gate1 -> led1
```

### 3. Comments

Any line starting with `//` is treated as a comment and ignored by the parser.

```
// This is a comment
AND n1 @ 100, 100
```

## Standard Library

The following node types are built-in:

| Type | Inputs | Outputs | Description |
|------|--------|---------|-------------|
| `In` | 0 | 1 (`out`) | Interactive entry point (Switch/Button). |
| `Out` | 1 (`in`) | 0 | Visual indicator (LED). |
| `AND` | 2 (`in1`, `in2`) | 1 (`out`) | Output is HIGH only if both inputs are HIGH. |
| `OR` | 2 (`in1`, `in2`) | 1 (`out`) | Output is HIGH if at least one input is HIGH. |
| `NOT` | 1 (`in`) | 1 (`out`) | Inverts the input signal. |
| `NAND`| 2 (`in1`, `in2`) | 1 (`out`) | AND followed by NOT. |
| `NOR` | 2 (`in1`, `in2`) | 1 (`out`) | OR followed by NOT. |
| `XOR` | 2 (`in1`, `in2`) | 1 (`out`) | Output is HIGH if inputs are different. |
| `XNOR`| 2 (`in1`, `in2`) | 1 (`out`) | Output is HIGH if inputs are the same. |
