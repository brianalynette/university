Introduction to Raytracing and Shading
======================================

Follow the instructions on the [general instructions page](../Rules.md) to set up what you need.



Ex.1: Ray Tracing a Parallelogram
---------------------------
 - New parameters for the parallelogram. These include an origin point, a u vector and v vector.
 - Calculated points u, v, t
 - Checked u, v, t to see if ray intersected with the parallelogram
 - Computed the intersection point and its normal

 ![](img/plane_orthographic.png)

Ray Tracing with Perspective Projection
---------------------------------------
 - Modified ray equation for perspective projection
 - Prepared the ray and checked if it intersected with the parallelogram
 - Computed the intersection point and its normal
 - Difference in the result of a sphere vs parallelogram with respect to perspective:
    - Sphere: no matter what perspective we set up (i.e. camera view point), the sphere will still appear the same.
    - Parallelogram: it is a flat surface. Shifting the perspective will show us a completely different view of the object and thus the appearance.

![](img/plane_perspective.png)

Shading
-------
 - Implemented ambient, specular, and diffuse shading
 - Changed a single matrix (C) to three (R,G,B) to keep track of each colour value in RGB format instead of grey-scale
 - Experimentation showed the following:
    - Using a very large specular exponent (the Phong exponent) resulted in a very small shining spot and a completely black lower half. A small exponent, on the other hand, resulted in a larger white reflection and the opposite of the top colour to appear on the bottom. 
    - Using large values in the RGB spaces for diffuse or specular shading resulted in a much harsher deliniation in colours for the sphere. This is likely due to the overlap of colours resulting in white with RGB.

![](img/shading.png)
