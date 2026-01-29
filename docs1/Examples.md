# Circuit Examples

This document provides complete, copy-paste ready examples of common digital circuits.

## Basic Gate Library

Start every project by defining or loading these fundamental gates:

```
// === NAND (Universal Gate) ===
define NAND(a, b) -> (out):
  t = a AND b
  out = NOT t
end

// === OR Gate ===
define OR(a, b) -> (out):
  na = NOT a
  nb = NOT b
  t = na AND nb
  out = NOT t
end

// === NOR Gate ===
define NOR(a, b) -> (out):
  na = NOT a
  nb = NOT b
  out = na AND nb
end

// === XOR Gate ===
define XOR(a, b) -> (out):
  na = NOT a
  nb = NOT b
  t1 = a AND nb
  t2 = na AND b
  out = OR(t1, t2)
end

// === XNOR Gate ===
define XNOR(a, b) -> (out):
  t = XOR(a, b)
  out = NOT t
end
```

---

## Combinational Circuits

### 2-to-1 Multiplexer

Selects between two inputs based on a select signal.

```
// MUX2: if sel=0, out=a; if sel=1, out=b
define MUX2(a, b, sel) -> (out):
  nsel = NOT sel
  t1 = a AND nsel
  t2 = b AND sel
  out = OR(t1, t2)
end

// Test circuit
MUX2 mux @ 200, 150
In input_a @ 50, 100
In input_b @ 50, 200
In select @ 50, 300
Out result @ 400, 150

input_a -> mux.in0
input_b -> mux.in1
select -> mux.in2
mux.out -> result
```

### 4-to-1 Multiplexer

```
define MUX4(a, b, c, d, s0, s1) -> (out):
  m1 = MUX2(a, b, s0)
  m2 = MUX2(c, d, s0)
  out = MUX2(m1, m2, s1)
end
```

### 1-to-2 Demultiplexer

Routes one input to one of two outputs based on select.

```
define DEMUX2(input, sel) -> (y0, y1):
  nsel = NOT sel
  y0 = input AND nsel
  y1 = input AND sel
end
```

### 2-to-4 Decoder

```
define Decoder2to4(a, b) -> (y0, y1, y2, y3):
  na = NOT a
  nb = NOT b
  y0 = na AND nb
  y1 = a AND nb
  y2 = na AND b
  y3 = a AND b
end

// Test circuit
Decoder2to4 dec @ 200, 200
In sel0 @ 50, 150
In sel1 @ 50, 250
Out led0 @ 400, 100
Out led1 @ 400, 200
Out led2 @ 400, 300
Out led3 @ 400, 400

sel0 -> dec.in0
sel1 -> dec.in1
dec.out0 -> led0
dec.out1 -> led1
dec.out2 -> led2
dec.out3 -> led3
```

---

## Arithmetic Circuits

### Half Adder

Adds two bits, produces sum and carry.

```
define HalfAdder(a, b) -> (sum, carry):
  sum = XOR(a, b)
  carry = a AND b
end

// Test circuit
HalfAdder ha @ 200, 150
In bit_a @ 50, 100
In bit_b @ 50, 200
Out sum_out @ 400, 100
Out carry_out @ 400, 200

bit_a -> ha.in0
bit_b -> ha.in1
ha.out0 -> sum_out
ha.out1 -> carry_out
```

### Full Adder

Adds three bits (A, B, Carry-in).

```
define FullAdder(a, b, cin) -> (sum, cout):
  s1 = XOR(a, b)
  c1 = a AND b
  sum = XOR(s1, cin)
  c2 = s1 AND cin
  cout = OR(c1, c2)
end
```

### 4-Bit Ripple Carry Adder

```
// Complete 4-bit adder circuit
FullAdder fa0 @ 200, 50
FullAdder fa1 @ 200, 200
FullAdder fa2 @ 200, 350
FullAdder fa3 @ 200, 500

// Inputs: A[3:0], B[3:0], Cin
In a0 @ 50, 25
In b0 @ 50, 75
In a1 @ 50, 175
In b1 @ 50, 225
In a2 @ 50, 325
In b2 @ 50, 375
In a3 @ 50, 475
In b3 @ 50, 525
In cin @ 50, 600

// Outputs: Sum[3:0], Cout
Out s0 @ 400, 50
Out s1 @ 400, 200
Out s2 @ 400, 350
Out s3 @ 400, 500
Out cout @ 400, 600

// Wiring
a0 -> fa0.in0
b0 -> fa0.in1
cin -> fa0.in2
fa0.out0 -> s0

a1 -> fa1.in0
b1 -> fa1.in1
fa0.out1 -> fa1.in2
fa1.out0 -> s1

a2 -> fa2.in0
b2 -> fa2.in1
fa1.out1 -> fa2.in2
fa2.out0 -> s2

a3 -> fa3.in0
b3 -> fa3.in1
fa2.out1 -> fa3.in2
fa3.out0 -> s3
fa3.out1 -> cout
```

### Comparator (1-bit)

Compares two bits, outputs: A>B, A=B, A<B

```
define Compare1(a, b) -> (gt, eq, lt):
  nb = NOT b
  na = NOT a
  gt = a AND nb
  lt = na AND b
  eq = XNOR(a, b)
end
```

---

## Sequential Building Blocks

### SR Latch (NAND-based)

```
// Note: This creates feedback, behavior depends on simulator
define SR_Latch_NAND(s, r) -> (q, qn):
  q = NAND(s, qn)
  qn = NAND(r, q)
end
```

### Gated D Latch

```
define D_Latch(d, enable) -> (q, qn):
  nd = NOT d
  s = d AND enable
  r = nd AND enable
  q = NAND(s, qn)
  qn = NAND(r, q)
end
```

---

## Display Circuits

### 7-Segment Decoder (Simplified - digits 0-3)

```
define Seg7_0to3(b1, b0) -> (a, b, c, d, e, f, g):
  // Decode 2-bit input to 7-segment display
  nb1 = NOT b1
  nb0 = NOT b0

  // Segment logic (simplified for 0-3)
  // 0: abcdef  1: bc  2: abdeg  3: abcdg

  t00 = nb1 AND nb0
  t01 = nb1 AND b0
  t10 = b1 AND nb0
  t11 = b1 AND b0

  a = OR(t00, OR(t10, t11))
  b = OR(t00, OR(t01, OR(t10, t11)))
  c = OR(t00, OR(t01, t11))
  d = OR(t00, OR(t10, t11))
  e = t00
  f = OR(t00, t11)
  g = OR(t10, t11)
end
```

---

## Tips for Building Complex Circuits

### 1. Build Bottom-Up

Start with basic gates, then build more complex components:

```
NOT, AND → NAND, OR, NOR → XOR, XNOR → HalfAdder → FullAdder → 4-bit Adder
```

### 2. Test Incrementally

After defining each gate, create a small test circuit:

```
// Quick test for OR gate
OR test @ 200, 100
In a @ 50, 50
In b @ 50, 150
Out result @ 350, 100

a -> test.in0
b -> test.in1
test.out -> result
```

### 3. Use Meaningful Names

```
// Good
define FullAdder(a, b, cin) -> (sum, cout):

// Less clear
define FA(x, y, z) -> (s, c):
```

### 4. Comment Your Code

```
// 4-bit magnitude comparator
// Inputs: A[3:0], B[3:0]
// Outputs: A_greater, A_equal, A_less
define Compare4(a3, a2, a1, a0, b3, b2, b1, b0) -> (gt, eq, lt):
  // Compare bit by bit from MSB
  ...
end
```
