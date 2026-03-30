# Notes

How did I calculated the aircraft constants (and a little reminder of physics for me)

$F = ma$

$\vec{F}_{net} = \vec{T} + \vec{L} + \vec{D} + \vec{W}$

* $\vec{T}$ thrust
* $\vec{L}$ lift
* $\vec{D}$ drag
* $\vec{W}$ weight

### Acceleration

$\vec{a} = \frac{\vec{F}_{net}}{m}$

- Horizontal $a_x = \frac{T - D}{m}$
- Vertical $a_y = \frac{L - W}{m}$

### Velocity

$\vec{v}_t = \vec{v}_0 + \vec{a} \cdot t$

$\vec{v}(t) = \int \vec{a}(t) \, dt$

* $\vec{v}_t$ velocity in a specific time
* $\vec{v}_0$ start velocity
* $t$ time

### Drag

$D = \frac{1}{2} \cdot \rho \cdot v^2 \cdot C_D \cdot A$

I'll treat $\rho$ and $A$ as constant to get a simpler version, And $C_D$ is my drag coefficient.

$D = v^2 \cdot C_D$

### Lift

$L = \frac{1}{2} \cdot \rho \cdot v^2 \cdot C_L \cdot S$

I'll treat $\rho$ and $S$ as constant to get a simpler version, And $C_L$ is my lift coefficient.

$L = v^2 \cdot C_L$

# Example with numbers

Let's take somthing like F16...

| Property     | Value     | Unit  |
|--------------|-----------|-------|
| Weight       | $120,000$ | $N$   |
| Thrust       | $130,000$ | $N$   |
| Max Speed    | $600$     | $m/s$ |
| Stall Speed  | $65$      | $m/s$ |
| Cruise Speed | $250$     | $m/s$ |

### $C_D$ calculation

In max velocity ($600$) drag should be equal to thrust.

$$|\vec{T}|+|\vec{D}|=0$$

$$130,000=600^2 \cdot C_D \rightarrow C_D \approx 0.36$$

### $C_L$ calculation

In a cruise speed ($250$) lift should be equal to weight.

$$|\vec{L}|+|\vec{W}|=0$$

$$120,000=250^2 \cdot C_L \rightarrow C_L=1.92$$


