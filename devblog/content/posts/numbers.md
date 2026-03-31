+++
date = "2026-03-31T12:00:40+03:00"
draft = false
title = "Calculating Some Numbers"
+++

# Calculating Some Numbers

In order to give my aircraft feel of a true jet, I needed some numbers to add to my physics equations.

I started from the basics, Newtonian force law:

$$F = ma$$

And using vectors of all forces I get:

$$\vec{F}_{net} = \vec{T} + \vec{L} + \vec{D} + \vec{W}$$

Where:

* $\vec{T}$ Thrust
* $\vec{L}$ Lift
* $\vec{D}$ Drag
* $\vec{W}$ Weight

So acceleration using vectors is:

$$\vec{a} = \frac{\vec{F}_{net}}{m}$$

## Drag - The Anti Thrust

Drag definition contain some variables $\rho$ and $A$ that will be treated as a constant to make the equation simpler.

$$D = \frac{1}{2} \cdot \rho \cdot v^2 \cdot C_D \cdot A$$

In my simpler version, $C_D$ is my drag coefficient:

$$D = v^2 \cdot C_D$$

## Lift - Gravity's Enemy

Here again, list definition contain some variables $\rho$ and $S$ that will be treated as constant to make the equation
simpler.

$$L = \frac{1}{2} \cdot \rho \cdot v^2 \cdot C_L \cdot S$$

And again, in my simpler version, $C_L$ is my lift coefficient:

$$L = v^2 \cdot C_L$$

## Weight - Fight the Gravity

Here the formula is much simpler because I know the acceleration.

$$G\approx9.18$$

So in our case:

$$\frac{F}{m}=G$$

---

# Numbers?!

OK, we fooled around with some equations but what about the numbers?

Let's take somthing like F16...

| Property     | Value     | Unit  |
|--------------|-----------|-------|
| Weight       | $120,000$ | $N$   |
| Thrust       | $130,000$ | $N$   |
| Max Speed    | $600$     | $m/s$ |
| Stall Speed  | $65$      | $m/s$ |
| Cruise Speed | $250$     | $m/s$ |

## $C_D$

In max velocity ($600m/s$) drag should be equal to thrust.

$$\vec{T}+\vec{D}=0$$

$$130,000=600^2 \cdot C_D \rightarrow C_D \approx 0.36$$

## $C_L$

In a cruise speed ($250m/s$) lift should be equal to weight.

$$\vec{L}+\vec{W}=0$$

$$120,000=250^2 \cdot C_L \rightarrow C_L=1.92$$

