=================================
Gravitas: A Physics Engine in C
=================================
Overview
========
Gravitas is a small but meaningful physics simulation written entirely in C.
The idea is simple: two balls fall under gravity inside a box, bounce off the
walls and floor, and collide with each other. The twist is that this happens
in real time, and the positions of the balls are drawn in ASCII characters
inside the terminal. It is both a demonstration of physics principles and a
showcase of how far you can push plain C without external graphics libraries.

# Why This Project Matters

Writing a physics engine forces you to bring together multiple skills:

- Mathematics: vectors, dot products, normalization, length calculations.
- Physics: forces, acceleration, velocity integration, momentum conservation.
- Programming: data structures, loops, functions, modular design.
- Systems: drawing and refreshing output efficiently in a terminal.

This is why Gravitas makes a strong portfolio piece. It shows an ability to
translate mathematical ideas into working C code that runs in real time.

# Key Components

1. Vector2 Library
   A Vec2 struct and helper functions allow all vector math to be clean and
   reusable. Without this, the code would be cluttered with x and y values.

2. Circle Rigid Body
   Each ball is represented as a Circle struct holding position, velocity,
   radius, and mass. This mirrors the design of more advanced engines.

3. Integration
   The program applies gravity as a force, converts it into acceleration, and
   then updates velocity and position every frame. This is Euler integration,
   the simplest numerical method for simulating motion.

4. Collision Detection
   Two circles collide if their centers are closer than the sum of their
   radii. This is efficient and works perfectly for spheres.

5. Collision Resolution
   When circles overlap, an impulse is calculated that pushes them apart and
   adjusts their velocities so they bounce realistically. Restitution controls
   how bouncy the collisions feel.

6. ASCII Rendering
   Each frame, the screen is cleared and redrawn as a grid. Walls are shown as
   '#' and the circles appear as 'O'. This crude visualization lets you watch
   the simulation unfold directly in the terminal.

# How to Build

1. Save the source code as gravitas.c
2. Compile with:
   gcc gravitas.c -o gravitas -lm
3. Run the program:
   ./gravitas

# How to Use

When you run the program, a box appears made of '#' symbols. Two 'O' characters
represent the circles. They start with opposite horizontal velocities, drift
toward each other, and eventually collide. Gravity pulls them downward, so they
also bounce on the floor. The simulation runs for several hundred frames and
then exits.

# Future Extensions

This project is designed to be extended. Here are natural next steps:

- Add more balls and check collisions in pairs.
- Support different shapes like rectangles or polygons using the Separating
  Axis Theorem.
- Include torque, angular momentum, and friction for realism.
- Replace ASCII rendering with graphical rendering using SDL or OpenGL.
