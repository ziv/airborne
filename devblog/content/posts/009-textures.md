+++
date = '2026-03-29T00:00:00+00:00'
draft = false
title = "Textures From The Sky"
cover = "cover/i8.png"
+++

We left off the last post with a beautiful 3D terrain, but it had a bit of a flaw. We ran into a problem with coloring
flat plains that happen to sit at or below sea level, they were incorrectly painted as water. In this post, we will
solve that problem and apply the final texture to our terrain.

![heightmap and colormap](/hm-and-hyps.png)

The main issue was that we tried to automatically generate the colors for our terrain based solely on the height of each
point, without any context of whether it was actually sea or land.

But I realized I don't have to calculate that. We live in a world where we have access to high-quality satellite
imagery, and we can use that to our advantage. I fired up QGIS and exported a beautiful satellite image (thanks to Bing
Maps) of the area around Sardinia. I then used that real-world image as the texture for my terrain, and it looks
significantly better.

See for yourself:

![Sardinia with texture](/textured-sardinia.png)