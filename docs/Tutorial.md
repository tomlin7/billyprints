# Billyprints Tutorial: Your First Circuit

This tutorial will guide you through creating a simple interactive circuit using the script editor. We will build a basic **Logic Tester** that lights up an LED when two buttons are pressed.

## Step 1: Open the Script Editor

1. Launch Billyprints.
2. If the script editor on the right is collapsed, click the `<` button at the top-right corner to expand it.

## Step 2: Add Components

In the script editor, we will define our components. We need two switches (Inputs), one AND gate, and one LED (Output).

Clear the current script and paste the following:

```
In switch1 @ 100, 100
In switch2 @ 100, 250
AND logic @ 300, 175
Out led @ 500, 175
```

Click **Apply** (or the refresh icon). You should see four nodes appear on the canvas.

## Step 3: Connect the Wires

Now we need to tell the signals where to go. We want `switch1` and `switch2` to connect to the inputs of the `logic` gate, and the result to go to the `led`.

Add these lines to the bottom of your script:

```
switch1.out -> logic.in1
switch2.out -> logic.in2
logic.out -> led.in
```

Click **Apply**. You will see wires appear connecting your nodes.

## Step 4: Test It!

1. Locate the **switch1** and **switch2** nodes on the canvas.
2. Click the button inside `switch1`. It should toggle to **ON**.
3. The `led` is still **OFF** (Low).
4. Now click `switch2` to toggle it **ON**.
5. The `led` should light up (High/Bright)!

## Bonus: Momentary Buttons

Want a button that only stays ON while you hold it? Update your switch definitions:

```
In switch1 @ 100, 100 momentary
In switch2 @ 100, 250 momentary
```

Click **Apply**. Now the buttons will say **PUSH** and only send a signal while you are clicking and holding them.
