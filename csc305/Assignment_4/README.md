Ray Tracing: Triangle Meshes and AABB Trees
===========================================

The goal of this assignment is to implement ray tracing for a triangle mesh, and implement acceleration structures to make the computation faster.

### Using Eigen

In all exercises you will need to do operations with vectors and matrices. To simplify the code, you will use [Eigen](http://eigen.tuxfamily.org/).
Have a look at the [Getting Started](http://eigen.tuxfamily.org/dox/GettingStarted.html) page of Eigen as well as the [Quick Reference](http://eigen.tuxfamily.org/dox/group__QuickRefPage.html}) page for a reference of the basic matrix operations supported.

### Preparing the Environment

Follow instructions the [general rules](../Rules.md) to setup what you need for the assignment.


Ex.1: Triangle Mesh Ray Tracing
-------------------------------

In this exercise I implemented the ray-tracing of a triangle mesh.

### Tasks

1. Fill the starting code to complete the function implementing the intersection of a ray and a triangle.
2. Fill the starting code to implement the ray-tracing of a triangle mesh.
3. Implement the `find_nearest_object` function and remember to return a boolean and not the index of the closest triangle.
4. Implement the correct `image_y` and `image_x`.

Ex.2: AABB Trees
----------------

In this exercise I attempted to implement an acceleration structure to speed-up the rendering of an image via ray-tracing. The implementation unfortunately was not completed, however I left the skeleton of it in this assignment.

### Sorting Criteria

We need a criteria to split the set of input triangles S into two subsets S1 and S2. We propose to simply sort the input triangles based on the coordinate of their centroid along the longest axis of the box spanned by those centroids. Then, S1 will hold the left half (rounded up), and S2 will hold the right half (rounded down).

### Tasks

1. Implement the intersection test of a ray and an axis-aligned bounding box. This test should be very simple. In particular, there should be no need to solve any linear system here.
2. Implement the top-down or the bottom-up construction method described above.
3. Update the intersection code of a ray and a mesh to use this newly created BVH. In particular, now you should only need to test the intersection for leaf nodes whose bounding box also intersects the input ray.

Starting Code
-------------

After compiling the code following the process described in the [general instructions](../../RULES.md), you can launch the program from command-line as follows:

```
mkdir build; cd build; cmake ..; make
./assignment4
```
Once you complete the assignment, you should see result a picture generated in your folder.

Running the completed code should give you the following result:

![](images/bunny.png?raw=true)

NB: To visualize the input 3D models, it is suggested to use a software such as [Meshlab](http://www.meshlab.net/).
