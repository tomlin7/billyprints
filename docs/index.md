You can create as many hierarchies as you like! There is **no hardcoded limit** on how many levels deep you can go (e.g., Gate A inside Gate B inside Gate C...).

However, there are a few practical "engineer's warnings" to keep in mind:

1. Memory Usage: Each time you place a Custom Gate, the app instantiates every single node that defines it. If you create a "Mega Gate" with 1,000 nodes and then place 10 of those into another gate, you've suddenly got 10,000 nodes running in the background.
2. Stack Depth: The `Evaluate()` function uses recursion to pull signals through the hierarchy. While modern computers can handle thousands of recursive calls, a hierarchy that is hundreds of levels deep might eventually cause a "stack overflow" crash.
3. Recursive Paradoxes (Cycles): Be careful not to create a "Logic Loop"—for example, putting Gate A inside Gate B, and then trying to put Gate B back inside Gate A. The app doesn't currently check for this, and it will cause an infinite loop and crash your editor.
In short: For any reasonable circuit (like building a 64-bit CPU from scratch using just AND/NOT), you are perfectly safe! The new caching system I added ensures that even with massive hierarchies, the app only does the "thinking" it absolutely needs to do for each frame.