# Notes

How did I calculated the aircraft constants (and a little reminder of physics for me)

$F = ma$

$\vec{F}_{net} = \vec{T} + \vec{L} + \vec{D} + \vec{W}$

* $\vec{T}$ thrust
* $\vec{L}$ lift
* $\vec{D}$ drag
* $\vec{W}$ weight

#### Acceleration

$\vec{a} = \frac{\vec{F}_{net}}{m}$

- Horizontal $a_x = \frac{T - D}{m}$
- Vertical $a_y = \frac{L - W}{m}$

#### Velocity

$\vec{v}_t = \vec{v}_0 + \vec{a} \cdot t$

* $\vec{v}_t$ velocity in a specific time
* $\vec{v}_0$ start velocity
* $t$ time

  $\vec{v}(t) = \int \vec{a}(t) \, dt$

#### Drag

$D = \frac{1}{2} \cdot \rho \cdot v^2 \cdot C_D \cdot A$

I'll treat $\rho$ and $A$ as constant to get a simpler version:

$D = v^2 \cdot C_D$

And $C_D$ is my drag coefficient.

#### Lift

$L = \frac{1}{2} \cdot \rho \cdot v^2 \cdot C_L \cdot S$

I'll treat $\rho$ and $S$ as constant to get a simpler version:

$L = v^2 \cdot C_L$

And $C_L$ is my lift coefficient.

#### Example with numbers

I want to calculate my aircraft $C_D$ and $C_L$.
Let's take F15 numbers (more or less) as an example.

- Weight $15,000Kg$ ($147,000N$)
- Cruise speed $900km/h$ ($250m/s$)
- Thrust $~20,000N$ (estimation)

In cruise speed, acceleration is $0$ so:

$|\vec{T}|+|\vec{D}|=0$

$|\vec{L}|+|\vec{W}|=0$

Then:

$C_L = 147,000/(250^2) = 2.352$

$C_D = 20,000/(250^2) = 0.32$


---

more calculations for later

$a_y = \frac{L - W}{m}$

$a_x = \frac{T - D}{m}$

$v_{max} = \sqrt{\frac{2T}{\rho \cdot C_D \cdot A}}$

$V_{stall} = \sqrt{\frac{2W}{\rho \cdot S \cdot C_{L,max}}}$

or in its simpler versions for the game:

$v_{max} = \sqrt{\frac{2T}{C_D}}$

$V_{stall} = \sqrt{\frac{2W}{C_L}}$
