
#if MESH_TYPE==0
#include <MeshMesh.hlsl>
#elif MESH_TYPE==1
#include <MeshParticle.hlsl>
#elif MESH_TYPE==2
#include <MeshTerrain.hlsl>
#endif
