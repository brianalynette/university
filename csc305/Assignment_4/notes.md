# Admin Info
## Name, V#, OS
Briana Johnson
V00929120
macOS, 12.0.1 (Montery)
<br><br />

## Compiler
Configured with: --prefix=/Library/Developer/CommandLineTools/usr --with-gxx-include-dir=/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/c++/4.2.1
Apple clang version 12.0.0 (clang-1200.0.32.27)
Target: x86_64-apple-darwin21.1.0
Thread model: posix
  <br><br />

# Ex .1
* Implemented `find_nearest_object`, `ray_triangle_intersection`
* Updated `raytrace_scene`
<br><br />


# Ex. 2
*Please note: I spent 2 days straight trying to get this code to work and spent hours looking at pseudocode and trying to debug my issues. The code was not working, so I had to comment out the section that calls these functions. Sorry for the inconvenience.*

* Implemented `AABBTree`:
  * split input into two sets (s1 & s2)
  * created a vector array to keep track of indices
  * sorted both lists individually after sorting the centroids (implemented `sort_rows` and `compare_head`)
  * recursively called `build_tree_rec` on each subtree
  * set both roots to have their parent as the tree root
<br><br />
* Created a new function (`build_tree_rec`) which recursively calls itself to build the AABB Tree:
  * checks base case (if there are 2 or fewer items in the array) & returns accordingly
  * creates a temporary box which extends the barycentric coordinates of all remaining items in the array (implemented `get_barycentric`)
  * finds the maximum axis to compare against (implemented `compare_bary`)
  * creates a new parent node for the children you are about to compare
  * makes a temporary variable to keep track of the middle of the list
  * splits the list of remaining indices in two
  * recursivelly calls itself with the first half of the array (left) and the second half of the array (right)
  * sets new values for the parent node
<br><br />
* Did not finish implementation of:
  * `tree_traversal`: would return a list of the leaf nodes that intersected with the ray (so you would only need to call `ray_triangle_intersection`)
  * method 2 in `find_nearest_object` - this is what would have called `tree_traversal`
<br><br />

# Bonus Work
Bonus 1
-------
* Added code to include spheres/parallelograms. 
* Code taken from A3.
<br><br />

Bonus 2
-------
* Added code to include shadows. 
* Code taken from A3.
<br><br />

Bonus 3
-------
* Added code to include reflections. 
* Code taken from A3.
<br><br />
## Thank you markers!