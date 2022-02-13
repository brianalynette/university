Introduction to Raytracing and Shading
======================================

Follow the instructions on the [general instructions page](../Rules.md) to set up what you need.

Ex.1: Basic Ray Tracing
-----------------------

#### Ray Tracing a Parallelogram

1. Set up the parameters of the parallelogram (position of the corner, plus one vector for each side of the parallelogram)
2. Create a function to check if a ray intersects with an arbitrary parallelogram.
3. Calculate the intersection between the ray and the parallelogram using the function defined above.
4. Compute the normal of that intersection point.



#### Ray Tracing with Perspective Projection

5. Modify the ray-sphere intersection to follow the generic case we saw in class.
6. Modify the ray equation for perspective projection.
7. Compare the difference in the result for a sphere for a parallelogram (you can also create a scene with multiple objects).


Ex.2: Shading
-------------

### Description

In this second exercise, you will implement the shading equations introduced in class.

### Tasks

1. Implement the basic shading components discussed in class: ambient, specular, and diffuse.
2. Add RGB components instead of the current grey-scale one.
3. Experiment with the different parameters and observe their effect on the ray-traced shapes.
