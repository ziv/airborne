+++
date = '2026-03-29T00:00:00+00:00'
draft = true
title = "Calculating Some Numbers"
+++

![Future fr](/i5.png)

In order to give my aircraft feel of a true jet, I needed some numbers to add to my physics equations.

I opened my physics book and started to write down the equations that I will need to implement in my game. I will not go
into the details of the equations, but I will explain the basic idea behind them.

I started from the basics, Newton's laws of motion, force equals mass times acceleration:

$$F = ma$$

Since I want to get the acceleration in a 3D space, I will use vectors to represent the forces:

$$\vec{F}_{net} = \vec{T} + \vec{L} + \vec{D} + \vec{W}$$

Where:

* $\vec{T}$ Thrust
* $\vec{L}$ Lift
* $\vec{D}$ Drag
* $\vec{W}$ Weight

And in vector form, the equation becomes:

$$\vec{F}_{net} = m \cdot \vec{a}$$

And the acceleration is:

$$\vec{a} = \frac{\vec{F}_{net}}{m}$$

Let's break down each force and see how we can calculate it.

## Drag - The Anti Thrust

[Drag](https://en.wikipedia.org/wiki/Drag_(physics)) is the force that opposes the motion of the aircraft. It is caused
by the air resistance and it is proportional to
the square of the velocity.

$$F_D = \frac{1}{2} \cdot \rho \cdot v^2 \cdot C_D \cdot A$$

Its definition contain some variables $\rho$ and $A$ that will be treated as a constant to make the equation
simpler.

$$F_D = v^2 \cdot C_D$$

In my simpler version, $C_D$ is my drag coefficient.

## Lift - Gravity's Enemy

[Lift](https://en.wikipedia.org/wiki/Lift_(force)) is the force that opposes the weight of the aircraft and it is caused
by the air flowing over the wings. It is also
proportional to the square of the velocity.

$$F_L = \frac{1}{2} \cdot \rho \cdot v^2 \cdot C_L \cdot S$$

Here again, lift definition contain some variables $\rho$ and $S$ that will be treated as constant to make the equation
simpler.

$$F_L = v^2 \cdot C_L$$

And again, in my simpler version, $C_L$ is my lift coefficient.

## Weight - Nobody Beats the Gravity

Here the formula is much simpler since we know the acceleration. Its a constant.

$$G\approx9.18$$

So in our case:

$$F_W = m \cdot G$$

$$\frac{F}{m}=G$$

---

# Numbers?!

OK, we fooled around with some equations but what about the numbers? Let's take some real numbers from a real jet and
see how we can use them to calculate the coefficients.

F-16 (Falcon/Viper) is a good example of a jet that is not too fast and not too slow, and it has a lot of data available
online.

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

---

Now we have the coefficients, we can use them in our equations to calculate the forces and the acceleration of our
aircraft.