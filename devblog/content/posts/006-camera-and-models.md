+++
date = '2026-03-26T00:00:00+00:00'
draft = false
title = "Models And Cameras, Oh My!"
cover = "cover/i5.png"
+++

In the previous post, we explored how to move a 3D model using angles. However, a camera is not a model. It does not
have a generic `transform` component, so we need to find another way to rotate it.

"raylib" defines a data structure called a "[Quaternion](https://en.wikipedia.org/wiki/Quaternion)". It is a
mathematical construct used to represent rotations in 3D space safely, without suffering
from [gimbal lock](https://en.wikipedia.org/wiki/Gimbal_lock) - a notorious problem that occurs when relying solely on
Euler angles.

## What is a Quaternion?

In mathematics, the quaternion number system extends complex numbers and is heavily used in 3D graphics to calculate
smooth rotations. Simply put, a quaternion is a four-dimensional vector that elegantly represents an orientation in
three-dimensional space.

Quaternions are generally represented in the following form:

$$a+b\,\mathbf{i}+c\,\mathbf{j}+d\,\mathbf{k}$$

where the coefficients $a, b, c, d$ are real numbers, and $1, i, j, k$ are the basis elements (read more
on [Wikipedia](https://en.wikipedia.org/wiki/Quaternion)).

## The Camera's Anatomy

The camera in "raylib" does not store its own rotation state. Instead, its orientation is defined by three vectors:

- **Position:** The exact point where the camera is located in the world.

- **Target (Forward):** The point the camera is looking at.

- **Up:** The direction that tells the camera which way is "up" (preventing the world from rendering upside down).

To effectively control the camera, we can maintain its internal orientation using a **quaternion**. This way, we can
apply rotations to the quaternion and easily extract the new `forward` and `up` vectors to update the camera.

## Rotating the Camera: Step-by-Step

Let's see an example of how to rotate the camera using quaternions.

### 1. Setting Up the Reference Vectors

First, we need to define our "relative world" coordinate system. These are constant vectors that represent the baseline
axes. We also need to set up the camera's initial rotation state.

```c++
// Reference vectors representing the "relative world" coordinate system
const auto worldUp = (Vector3){0.0f, 1.0f, 0.0f}; 
const auto worldForward = (Vector3){0.0f, 0.0f, 1.0f};
const auto worldRight = (Vector3){-1.0f, 0.0f, 0.0f};

// Start with the identity quaternion (represents no rotation)
auto matrix = QuaternionIdentity();

// Extract the current camera directional vectors
auto right = worldRight; 
auto up = camera.up;
auto forward = Vector3Normalize(camera.target); // Simplified for this example

// Arbitrary angles to rotate the camera (in radians)
float pitch = 0.2f; // Rotation around the right vector
float roll = 0.5f;  // Rotation around the forward vector
float yaw = 0.1f;   // Rotation around the up vector
```

### 2. Calculating the Rotations

Just like with the model, we want to apply changes to the pitch, roll, and yaw angles. We calculate a separate
quaternion for each axis rotation:

```c++
const auto qPitch = QuaternionFromAxisAngle(right, pitch);
const auto qRoll = QuaternionFromAxisAngle(forward, roll);
const auto qYaw = QuaternionFromAxisAngle(up, yaw);
```

Next, we combine them to get the final orientation of the camera for this frame:

```c++
const auto orientation = QuaternionMultiply(
    qYaw, 
    QuaternionMultiply(
        qPitch, 
        qRoll));
```

### 3. Updating the State and Applying to Camera

Now, we update our main rotation matrix by multiplying the new orientation quaternion with the current rotation state:

```c++
matrix = QuaternionNormalize(QuaternionMultiply(orientation, matrix));
```

Finally, we calculate the new `forward`, `up`, and `right` vectors based on our updated quaternion matrix, and feed
those
values back into the "raylib" camera:

```c++
// Calculate the new directional vectors after applying the rotation
forward = Vector3Normalize(Vector3RotateByQuaternion(matrix, worldForward));
up = Vector3Normalize(Vector3RotateByQuaternion(matrix, worldUp));
right = Vector3Normalize(Vector3RotateByQuaternion(matrix, worldRight));

// Apply the new orientation to the camera
camera.target = forward;
camera.up = up;
```

By calculating the camera's orientation this way, we can rotate it freely in any direction without ever worrying about
gimbal lock or broken axes!

