# Billyprints Demos

Here are some example scripts you can copy and paste into the editor to create functional circuits.

## 1. Half Adder

A simple circuit that adds two single binary digits and produces a Sum and a Carry.

```
In A @ 100, 100
In B @ 100, 250

XOR sum_gate @ 300, 100
AND carry_gate @ 300, 250

Out Sum @ 500, 100
Out Carry @ 500, 250

A -> sum_gate.in1
B -> sum_gate.in2

A -> carry_gate.in1
B -> carry_gate.in2

sum_gate -> Sum
carry_gate -> Carry
```

## 2. RS Latch (Memory)

A basic memory unit using NOR gates. This circuit "remembers" which button was last pressed.
*Note: This relies on the simulator's propagation steps to handle feedback loops.*

```
In Set @ 100, 100 momentary
In Reset @ 100, 300 momentary

NOR nor_top @ 350, 100
NOR nor_bottom @ 350, 300

Out Q @ 550, 100
Out NotQ @ 550, 300

// Cross-coupling feedback loops
Set -> nor_top.in1
nor_bottom.out -> nor_top.in2

Reset -> nor_bottom.in2
nor_top.out -> nor_bottom.in1

// Outputs
nor_top.out -> Q
nor_bottom.out -> NotQ
```

## 3. XOR from NANDs (NAND Logic)

Proving that NAND is a universal gate by building an XOR gate using only NANDs.

```
In A @ 50, 100
In B @ 50, 250

NAND n1 @ 200, 175
NAND n2 @ 350, 100
NAND n3 @ 350, 250
NAND n4 @ 500, 175

Out Output @ 650, 175

// Initial NAND
A -> n1.in1
B -> n1.in2

// Second Stage
A -> n2.in1
n1.out -> n2.in2

n1.out -> n3.in1
B -> n3.in2

// Final Stage
n2.out -> n4.in1
n3.out -> n4.in2

n4.out -> Output
```
