+++
date = '2026-03-25T00:00:00+00:00'
draft = false
title = "Orientation Is Everything"
cover = "cover/i4.png"
+++

Let's continue exploring model movement. In this example, we will take all three angles into account: roll, pitch, and yaw.

But before we start, let's go back to the basics. What we did in the last post was apply a **transformation** to the model.

(This isn't going to be a linear algebra lesson; you can find more about linear transformations in 3D space in [this article](https://en.wikipedia.org/wiki/Linear_map).)

As a reminder, if we want to apply multiple transformations, we can combine them into a single transformation. We do this by multiplying the transformation matrices together (applied sequentially as Yaw $\rightarrow$ Pitch $\rightarrow$ Roll):

$$M=R_{roll} \cdot R_{pitch} \cdot R_{yaw}$$

* Note: The order of multiplication is crucial because matrix multiplication is **not commutative**.

## Keyboard Input

Before the video demonstration, let's see how we can control the model with keyboard input. We will use the `IsKeyDown` function to check if a key is pressed and update the angles accordingly.

```c++
// setup
auto roll = 0.0f;
auto pitch = 0.0f;
auto yaw = 0.0f;

// and in the main loop:
 
if (IsKeyDown(KEY_RIGHT)) roll += 0.01f;
if (IsKeyDown(KEY_LEFT)) roll -= 0.01f;
if (IsKeyDown(KEY_UP)) pitch += 0.01f;
if (IsKeyDown(KEY_DOWN)) pitch -= 0.01f;
// I used A and D for yaw
if (IsKeyDown(KEY_D)) yaw += 0.01f; 
if (IsKeyDown(KEY_A)) yaw -= 0.01f;
```

While this isn't exactly how we will ultimately control our model, we'll use it as a foundation for our future input system.

Let's wrap the values to stay within a $2\pi$ range. This isn't strictly necessary for the transformation itself, but it keeps the UI display clean and makes the angles easier to understand.

```c++
constexpr auto PI2 = PI * 2;

if (roll > PI2) roll -= PI2;
if (roll < 0) roll += PI2;
if (pitch > PI2) pitch -= PI2;
if (pitch < 0) pitch += PI2;
if (yaw > PI2) yaw -= PI2;
if (yaw < 0) yaw += PI2;
```

Finally, we apply the combined transformation to the model:

```c++
model.transform = MatrixRotateZ(roll) * MatrixRotateX(pitch) * MatrixRotateY(yaw);
```

* Thanks to C++ operator overloading, we can conveniently use the `*` operator to multiply these matrices. "raymath" does provide a dedicated function for matrix multiplication (`MatrixMultiply`), but utilizing the overloaded operators makes the math much more readable.

Now we can see the model rotating across all three axes, fully controlled by keyboard input. I've added visual cues in the video below to identify the keystrokes:

{{< youtube gZcf-Y9sP2s >}}
