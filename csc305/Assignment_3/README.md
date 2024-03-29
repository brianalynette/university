Ex.0: Implement the intersection code
-------------------------------------

### Tasks
Fill the functions `ray_sphere_intersection` and `ray_parallelogram_intersection` with the correct intersection between the ray and the primitives.

### Output with sphere intersection
Returns whether the ray intersects the sphere
    sphere_center:  center center point of the sphere
    sphere_radius: radius radius of the sphere
    ray_origin: origin origin point of the ray
    ray_direction: direction direction vector of the ray
    t: intersection closest intersection point of the ray with the sphere

### Implementation
1. Set up temporary intersection points to test
2. solve quadratic equation
3. Check to see if the intersection point hit anything
4. Solve for t
5. Solve p(t) = o + td (i.e. the parametric form)
6. Solve for normal
7. Return t
* Studied: https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection

### Output with sphere and plane intersection
Code copied from assignment 2 "raytrace_parallelogram" function.

### Images from this section
* Sphere intersection
* Sphere-plane intersection

![sphere_inter](https://user-images.githubusercontent.com/79673623/156946799-3eccb497-7d2d-45fb-816b-b600f0798dfc.png)
![sphere_plane_inter](https://user-images.githubusercontent.com/79673623/156946805-b3123d37-802a-468a-8217-799ee06ad481.png)


Ex.1: Field of View and Perspective Camera
------------------------------------------

### Description
The field of view of a perspective camera represents the angle formed between the camera's center and the sensor (aka the pixel grid through which rays are shot). The focal length is the distance between the camera center and the sensor and is called `f` in the figure above.

### Implementation
1. Calculated image_y (using a diagram of what the focal view looks like)
2. Calculated image_x (based on image_y and the provided aspect_ratio)
3. Updated the values for if the camera has a perspective aspect as opposted to an orthographic one - specifically ray_origin and ray_direction.

### Images from this section
* Field of view
* Perspetive

![field_of_view](https://user-images.githubusercontent.com/79673623/156946811-3402834b-b45f-43b9-85b6-23523a79ae50.png)
![perspective](https://user-images.githubusercontent.com/79673623/156946818-750e5df8-8879-4451-a283-df15b25c6d8d.png)

Ex.2: Shadow Rays
-----------------

### Description
To determine if a point is in the shadow of another or not, you must cast a ray from this point to the different light sources in the scene. If another object lies in between the point and the light source, then this light does not contribute to the point's color.

### Implementation
1. Added implementation to specular vector
2. Implemented `is_light_visible` function
3. Added `calculate_distance` function
4. Updated `shoot_ray` to account for if the light was not found to be visible

### Images from this section
* Shading
* Shadows

![shading](https://user-images.githubusercontent.com/79673623/156946822-8509d773-1463-4367-a244-c7e047ff57a2.png)
![shadows](https://user-images.githubusercontent.com/79673623/156946825-173976d2-7f33-4002-9051-3faf9234c0a2.png)


Ex.3: Reflection
----------------

### Description
To render mirrors, shading must also consider objects that could be reflected by the camera (primary) rays. This can be achieved by shooting new rays from the hit position to the scene with a new direction.

### Implementation
1. Add a black/empty space variable to check against the current `refl_color` variable
2. Check if we have any remaining bounces to hit & decrease counter if we do
3. Create new vector for the interception point and for the reflection direction
4. Implement the equation `r = d - 2(d·n)n`
5. Recursively call `shoot_ray` to bounce the new reflected rays off of the objects in our scene until we reach 0 remaining bounces
6. Scale our reflection_color by the provided refl_color

### Images from this section
* Reflection

![reflection](https://user-images.githubusercontent.com/79673623/156946830-f827378e-bff1-4923-9cb5-b049a8c25282.png)

Ex.4: Perlin Noise
------------------

### Description
Implement the Perlin noise as explained in class.

### Implementation
1. Implemented linear interpolation
2. Implemented `dotGridGradient`
3. Commented out linear interpolation, add provided cubic interpolation equation: with the sphere that has a texture applied, the linear interpolation is not quite as smoothly blended as the gradient interpolation.

### Images from this section
* Perlin noise (linear interpolation)
* Perlin noise (cubic interpolation)

![perlin_linear_interp](https://user-images.githubusercontent.com/79673623/156946840-042bb0e4-02d1-45ae-8359-3cb7266acc7f.png)
![perlin_gradient_interp](https://user-images.githubusercontent.com/79673623/156946844-2cc815f8-e77c-4200-8f53-6921004f9ccb.png)
