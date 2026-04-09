+++
date = '2026-04-07T12:00:40+03:00'
draft = true
title = 'Landing'
+++

![F35 fr](/i5.png)

Landing is a tricky to implement and test. Come on, landing itself is tricky and require keeping wings leveled, nose up
and not to fast. And all of that is hard to do in a simulator, where you have no real feedback from the aircraft, and
you have to rely on instruments and visual cues.

I crashed a lot of planes while trying to implement landing and I understand I needed a visual aid to help me understand
what is going on.

For easier debugging, I added a virtual landing area to the game, which is a simple box on the ground that represent the
area where the
plane should land.

Only when the plane is inside the box, the landing process will start and the plane will be able to land. This way I can
see if the plane is in the right position and if not, I can adjust my approach.

You can see this landing area in the screenshots below. The first one is on a surface:

![Landing area on a surface](/landing0.png)

And another one on a carrier:

![Landing area on a carrier](/landing1.png)