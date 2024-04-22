# Mesh Simplification Algorithm
An iterative algorithm to reduce the number of trianges in a 3d mesh (.obj file)
while maintaining the overall shape and silhouette of the original mesh.

# Example
![Alt Text](./assets/MonkeyComparison.png?raw=true "Title")
Takes about 15 seconds to run this example of ~250k to ~6k tris on a Mac M1 CPU

# Usage
The repo comes with a lightweight mesh viewer app for running examples and visualizing the results. The viewer requires glfw and glad. To build the viewer, run the following commands:
```bash
cd mesh_viewer
make all
```
To run the viewer, use 
```bash
./mesh_viewer <path_to_obj_file>
```
The obj file reader is very simple so the obj file needs to be triangulated or it will not load correctly. 

# Mesh Simplification Algorithm

The simplification algorithm is in AutoLOD::genLODMesh. The algorithm works by iteratively collapsing edges which have the smallest cost metric. The cost metric is calculated on each possible edge collapse operation. The cost metric depends on the amount of topological information lost by collapsing the edge (large cost for non-flat surfaces) and the resulting triangle aspect ratio (large cost for long/skinny triangles). The user can supply a parameter called maxSinTheta to balance the importance of maintianing topology vs aspect ratio, a small maxSinTheta will result in more weight applied to the topology, and a large maxSinTheta will apply more weight to the aspect ratio.