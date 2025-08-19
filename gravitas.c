#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h> // gives us usleep(), which lets us pause between frames

// ======================================================
// Vector2 implementation
// ------------------------------------------------------
// In physics programming you almost always need a way to
// handle 2D vectors cleanly. Instead of juggling x and y
// separately, we wrap them in a struct and then write
// helper functions to do vector math (add, subtract, etc).
// ======================================================
typedef struct
{
    float x, y; // x and y components
} Vec2;

// Vector addition: combine two vectors component-wise
Vec2 vec2_add(Vec2 a, Vec2 b) { return (Vec2){a.x + b.x, a.y + b.y}; }

// Vector subtraction: direction from b to a
Vec2 vec2_sub(Vec2 a, Vec2 b) { return (Vec2){a.x - b.x, a.y - b.y}; }

// Multiply vector by scalar (scale its length)
Vec2 vec2_scale(Vec2 a, float s) { return (Vec2){a.x * s, a.y * s}; }

// Dot product: projects one vector onto another, useful for
// checking alignment and relative velocity along a normal
float vec2_dot(Vec2 a, Vec2 b) { return a.x * b.x + a.y * b.y; }

// Vector length: standard Pythagoras
float vec2_len(Vec2 a) { return sqrtf(a.x * a.x + a.y * a.y); }

// Normalize vector: turn it into a unit vector (length 1).
// If the length is zero, return a zero vector to avoid div by 0.
Vec2 vec2_norm(Vec2 a)
{
    float len = vec2_len(a);
    return (len == 0) ? (Vec2){0, 0} : vec2_scale(a, 1.0f / len);
}

// ======================================================
// Circle rigid body
// ------------------------------------------------------
// We are treating each "ball" in our simulation as a rigid
// body with position, velocity, radius, and mass.
// ======================================================
typedef struct
{
    Vec2 pos; // position in the world
    Vec2 vel; // velocity (speed and direction of motion)
    float radius;
    float mass;
} Circle;

// Integrate motion using very simple Euler integration.
// Force -> acceleration -> velocity -> position
void integrate(Circle *c, Vec2 force, float dt)
{
    Vec2 accel = vec2_scale(force, 1.0f / c->mass);
    c->vel = vec2_add(c->vel, vec2_scale(accel, dt));
    c->pos = vec2_add(c->pos, vec2_scale(c->vel, dt));
}

// ======================================================
// Collision detection
// ------------------------------------------------------
// We only support circle-to-circle collisions here. The
// check is: are the centers closer together than the sum
// of the radii? If so, they are overlapping.
// ======================================================
int check_collision(Circle *a, Circle *b, Vec2 *normal, float *penetration)
{
    Vec2 diff = vec2_sub(b->pos, a->pos);
    float dist = vec2_len(diff);
    float radii = a->radius + b->radius;
    if (dist < radii)
    {
        // penetration = how deep the overlap is
        *penetration = radii - dist;
        // normal = unit vector pointing from a to b
        *normal = vec2_norm(diff);
        return 1; // collision happened
    }
    return 0; // no collision
}

// ======================================================
// Collision resolution
// ------------------------------------------------------
// Once we know two circles overlap, we need to push them
// apart and adjust their velocities so they bounce in a
// realistic way. We use an impulse-based method.
// ======================================================
void resolve_collision(Circle *a, Circle *b, Vec2 normal, float penetration)
{
    // relative velocity along the collision normal
    float relVel = vec2_dot(vec2_sub(b->vel, a->vel), normal);

    // If they are already moving apart, nothing to do
    if (relVel > 0)
        return;

    // Restitution controls bounciness (0 = inelastic, 1 = perfectly elastic)
    float e = 0.8f;
    float j = -(1 + e) * relVel;
    j /= (1 / a->mass) + (1 / b->mass);

    // Apply impulse to both bodies (equal and opposite reaction)
    Vec2 impulse = vec2_scale(normal, j);
    a->vel = vec2_sub(a->vel, vec2_scale(impulse, 1 / a->mass));
    b->vel = vec2_add(b->vel, vec2_scale(impulse, 1 / b->mass));

    // Positional correction so they do not sink into each other
    float percent = 0.8f;
    Vec2 correction = vec2_scale(normal, penetration * percent / (1 / a->mass + 1 / b->mass));
    a->pos = vec2_sub(a->pos, vec2_scale(correction, 1 / a->mass));
    b->pos = vec2_add(b->pos, vec2_scale(correction, 1 / b->mass));
}

// ======================================================
// ASCII Renderer
// ------------------------------------------------------
// We draw a grid of WIDTH x HEIGHT characters. Each frame
// clears the screen and redraws the walls plus the circles
// as 'O' characters in their positions. This is a quick
// and dirty way to visualize motion in the terminal.
// ======================================================
#define WIDTH 40
#define HEIGHT 20

void render(Circle *c1, Circle *c2)
{
    char grid[HEIGHT][WIDTH];

    // Fill grid with spaces
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++)
            grid[y][x] = ' ';

    // Convert floating point positions to integer grid coordinates
    int x1 = (int)roundf(c1->pos.x);
    int y1 = (int)roundf(c1->pos.y);
    int x2 = (int)roundf(c2->pos.x);
    int y2 = (int)roundf(c2->pos.y);

    // Plot the circles if they are inside the grid
    if (x1 >= 0 && x1 < WIDTH && y1 >= 0 && y1 < HEIGHT)
        grid[HEIGHT - 1 - y1][x1] = 'O';
    if (x2 >= 0 && x2 < WIDTH && y2 >= 0 && y2 < HEIGHT)
        grid[HEIGHT - 1 - y2][x2] = 'O';

    // Clear terminal screen using ANSI escape codes
    printf("\033[H\033[J");

    // Draw the grid row by row
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            // Draw borders as '#'
            if (y == 0 || y == HEIGHT - 1 || x == 0 || x == WIDTH - 1)
                printf("#");
            else
                printf("%c", grid[y][x]);
        }
        printf("\n");
    }
}

// ======================================================
// Main Simulation Loop
// ------------------------------------------------------
// Here we put everything together. Two circles are spawned
// with opposite velocities. Gravity pulls them down, they
// bounce off walls and the floor, and they collide with
// each other. Each step is drawn to the terminal.
// ======================================================
int main(void)
{
    // Two starting circles, one on the left moving right,
    // one on the right moving left.
    Circle c1 = {{10, 10}, {1, 0}, 1.0f, 2.0f};
    Circle c2 = {{30, 10}, {-1, 0}, 1.0f, 2.0f};

    float dt = 0.1f; // time step

    for (int step = 0; step < 300; step++)
    {
        // Gravity force acting downward on both
        Vec2 gravity1 = {0, -9.8f * c1.mass * dt};
        Vec2 gravity2 = {0, -9.8f * c2.mass * dt};
        integrate(&c1, gravity1, dt);
        integrate(&c2, gravity2, dt);

        // Bounce off the floor
        if (c1.pos.y <= 1)
        {
            c1.pos.y = 1;
            c1.vel.y *= -0.8f; // lose some energy
        }
        if (c2.pos.y <= 1)
        {
            c2.pos.y = 1;
            c2.vel.y *= -0.8f;
        }

        // Bounce off side walls
        if (c1.pos.x <= 1 || c1.pos.x >= WIDTH - 2)
            c1.vel.x *= -0.8f;
        if (c2.pos.x <= 1 || c2.pos.x >= WIDTH - 2)
            c2.vel.x *= -0.8f;

        // Check and resolve circle-circle collision
        Vec2 normal;
        float penetration;
        if (check_collision(&c1, &c2, &normal, &penetration))
        {
            resolve_collision(&c1, &c2, normal, penetration);
        }

        // Render everything
        render(&c1, &c2);

        // Small delay so the motion is visible at human speed
        usleep(50000); // 50 milliseconds
    }
    return 0;
}
