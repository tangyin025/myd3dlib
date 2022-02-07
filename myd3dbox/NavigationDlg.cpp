// NavigationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MainApp.h"
#include "NavigationDlg.h"
#include "afxdialogex.h"
#include "MainFrm.h"
#include "RecastDump.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshBuilder.h"
#include "DetourCommon.h"
#include "Terrain.h"
#include "mesh\GuTriangleMesh.h"
#include "convex\GuConvexMesh.h"


// CNavigationDlg dialog

enum SamplePartitionType
{
	SAMPLE_PARTITION_WATERSHED,
	SAMPLE_PARTITION_MONOTONE,
	SAMPLE_PARTITION_LAYERS,
};

IMPLEMENT_DYNAMIC(CNavigationDlg, CDialogEx)

CNavigationDlg::CNavigationDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CNavigationDlg::IDD, pParent)
	, m_bindingBox(-100, 100)
	, m_cellSize(0.3f)
	, m_cellHeight(0.2f)
	, m_agentHeight(2.0f)
	, m_agentRadius(0.6f)
	, m_agentMaxClimb(0.9f)
	, m_agentMaxSlope(45.0f)
	, m_regionMinSize(8)
	, m_regionMergeSize(20)
	, m_edgeMaxLen(12.0f)
	, m_edgeMaxError(1.3f)
	, m_vertsPerPoly(6)
	, m_detailSampleDist(6.0f)
	, m_detailSampleMaxError(1.0f)
	, m_filterLowHangingObstacles(TRUE)
	, m_filterLedgeSpans(TRUE)
	, m_filterWalkableLowHeightSpans(TRUE)
	, m_partitionType(SAMPLE_PARTITION_WATERSHED)
	, m_maxTiles(0)
	, m_maxPolysPerTile(0)
	, m_tileSize(32.0f)
{
}

CNavigationDlg::~CNavigationDlg()
{
}

void CNavigationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_bindingBox.m_min.x);
	DDX_Text(pDX, IDC_EDIT2, m_bindingBox.m_min.y);
	DDX_Text(pDX, IDC_EDIT3, m_bindingBox.m_min.z);
	DDX_Text(pDX, IDC_EDIT4, m_bindingBox.m_max.x);
	DDX_Text(pDX, IDC_EDIT5, m_bindingBox.m_max.y);
	DDX_Text(pDX, IDC_EDIT6, m_bindingBox.m_max.z);
	DDX_Text(pDX, IDC_EDIT7, m_cellSize);
	DDX_Text(pDX, IDC_EDIT8, m_cellHeight);
	DDX_Text(pDX, IDC_EDIT9, m_agentHeight);
	DDX_Text(pDX, IDC_EDIT10, m_agentRadius);
	DDX_Text(pDX, IDC_EDIT11, m_agentMaxClimb);
	DDX_Text(pDX, IDC_EDIT12, m_agentMaxSlope);
	DDX_Text(pDX, IDC_EDIT13, m_regionMinSize);
	DDX_Text(pDX, IDC_EDIT14, m_regionMergeSize);
	DDX_Text(pDX, IDC_EDIT15, m_edgeMaxLen);
	DDX_Text(pDX, IDC_EDIT16, m_edgeMaxError);
	DDX_Text(pDX, IDC_EDIT17, m_vertsPerPoly);
	DDX_Text(pDX, IDC_EDIT18, m_detailSampleDist);
	DDX_Text(pDX, IDC_EDIT19, m_detailSampleMaxError);
	DDX_Check(pDX, IDC_CHECK1, m_filterLowHangingObstacles);
	DDX_Check(pDX, IDC_CHECK2, m_filterLedgeSpans);
	DDX_Check(pDX, IDC_CHECK3, m_filterWalkableLowHeightSpans);
	DDX_Radio(pDX, IDC_RADIO1, m_partitionType);
	DDX_Text(pDX, IDC_EDIT20, m_tileSize);
}


BEGIN_MESSAGE_MAP(CNavigationDlg, CDialogEx)
END_MESSAGE_MAP()


// CNavigationDlg message handlers

void CNavigationDlg::doLog(const rcLogCategory category, const char* msg, const int len)
{
	if (GetCurrentThreadId() == my::D3DContext::getSingleton().m_d3dThreadId)
	{
		theApp.m_EventLog(msg);
	}
}

class BuildTileMeshTask : public my::ParallelTask
{
public:
	/// These are just sample areas to use consistent values across the samples.
	/// The use should specify these base on his needs.
	enum SamplePolyAreas
	{
		SAMPLE_POLYAREA_GROUND,
		SAMPLE_POLYAREA_WATER,
		SAMPLE_POLYAREA_ROAD,
		SAMPLE_POLYAREA_DOOR,
		SAMPLE_POLYAREA_GRASS,
		SAMPLE_POLYAREA_JUMP,
	};
	enum SamplePolyFlags
	{
		SAMPLE_POLYFLAGS_WALK = 0x01,		// Ability to walk (ground, grass, road)
		SAMPLE_POLYFLAGS_SWIM = 0x02,		// Ability to swim (water).
		SAMPLE_POLYFLAGS_DOOR = 0x04,		// Ability to move through doors.
		SAMPLE_POLYFLAGS_JUMP = 0x08,		// Ability to jump.
		SAMPLE_POLYFLAGS_DISABLED = 0x10,		// Disabled polygon
		SAMPLE_POLYFLAGS_ALL = 0xffff	// All abilities.
	};
	int tx;
	int ty;
	float bmin[3];
	float bmax[3];
	const CNavigationDlg* pdlg;
	rcContext* m_ctx;
	rcConfig m_cfg;
	boost::shared_ptr<rcHeightfield> m_solid;
	//boost::shared_array<char> m_triareas;
	boost::shared_ptr<rcCompactHeightfield> m_chf;
	boost::shared_ptr<rcContourSet> m_cset;
	boost::shared_ptr<rcPolyMesh> m_pmesh;
	boost::shared_ptr<rcPolyMeshDetail> m_dmesh;
	int dataSize;
	unsigned char* data;

	BuildTileMeshTask(int _tx, int _ty, const float* _bmin, const float* _bmax, const CNavigationDlg* _pdlg, rcContext* ctx)
		: tx(_tx)
		, ty(_ty)
		, pdlg(_pdlg)
		, m_ctx(ctx)
		, dataSize(0)
		, data(NULL)
	{
		dtVcopy(bmin, _bmin);
		dtVcopy(bmax, _bmax);
	}

	virtual void DoTask(void)
	{
		//if (!m_geom || !m_geom->getMesh() || !m_geom->getChunkyMesh())
		//{
		//	m_ctx->log(RC_LOG_ERROR, "buildNavigation: Input mesh is not specified.");
		//	return;
		//}

		int m_tileMemUsage = 0;
		int m_tileBuildTime = 0;

		//cleanup();

		//const float* verts = m_geom->getMesh()->getVerts();
		//const int nverts = m_geom->getMesh()->getVertCount();
		//const int ntris = m_geom->getMesh()->getTriCount();
		//const rcChunkyTriMesh* chunkyMesh = m_geom->getChunkyMesh();

		// Init build configuration from GUI
		memset(&m_cfg, 0, sizeof(m_cfg));
		m_cfg.cs = pdlg->m_cellSize;
		m_cfg.ch = pdlg->m_cellHeight;
		m_cfg.walkableSlopeAngle = pdlg->m_agentMaxSlope;
		m_cfg.walkableHeight = (int)ceilf(pdlg->m_agentHeight / m_cfg.ch);
		m_cfg.walkableClimb = (int)floorf(pdlg->m_agentMaxClimb / m_cfg.ch);
		m_cfg.walkableRadius = (int)ceilf(pdlg->m_agentRadius / m_cfg.cs);
		m_cfg.maxEdgeLen = (int)(pdlg->m_edgeMaxLen / pdlg->m_cellSize);
		m_cfg.maxSimplificationError = pdlg->m_edgeMaxError;
		m_cfg.minRegionArea = (int)rcSqr(pdlg->m_regionMinSize);		// Note: area = size*size
		m_cfg.mergeRegionArea = (int)rcSqr(pdlg->m_regionMergeSize);	// Note: area = size*size
		m_cfg.maxVertsPerPoly = (int)pdlg->m_vertsPerPoly;
		m_cfg.tileSize = (int)pdlg->m_tileSize;
		m_cfg.borderSize = m_cfg.walkableRadius + 3; // Reserve enough padding.
		m_cfg.width = m_cfg.tileSize + m_cfg.borderSize * 2;
		m_cfg.height = m_cfg.tileSize + m_cfg.borderSize * 2;
		m_cfg.detailSampleDist = pdlg->m_detailSampleDist < 0.9f ? 0 : pdlg->m_cellSize * pdlg->m_detailSampleDist;
		m_cfg.detailSampleMaxError = pdlg->m_cellHeight * pdlg->m_detailSampleMaxError;

		// Expand the heighfield bounding box by border size to find the extents of geometry we need to build this tile.
		//
		// This is done in order to make sure that the navmesh tiles connect correctly at the borders,
		// and the obstacles close to the border work correctly with the dilation process.
		// No polygons (or contours) will be created on the border area.
		//
		// IMPORTANT!
		//
		//   :''''''''':
		//   : +-----+ :
		//   : |     | :
		//   : |     |<--- tile to build
		//   : |     | :  
		//   : +-----+ :<-- geometry needed
		//   :.........:
		//
		// You should use this bounding box to query your input geometry.
		//
		// For example if you build a navmesh for terrain, and want the navmesh tiles to match the terrain tile size
		// you will need to pass in data from neighbour terrain tiles too! In a simple case, just pass in all the 8 neighbours,
		// or use the bounding box below to only pass in a sliver of each of the 8 neighbours.
		rcVcopy(m_cfg.bmin, bmin);
		rcVcopy(m_cfg.bmax, bmax);
		m_cfg.bmin[0] -= m_cfg.borderSize * m_cfg.cs;
		m_cfg.bmin[2] -= m_cfg.borderSize * m_cfg.cs;
		m_cfg.bmax[0] += m_cfg.borderSize * m_cfg.cs;
		m_cfg.bmax[2] += m_cfg.borderSize * m_cfg.cs;

		// Reset build times gathering.
		m_ctx->resetTimers();

		// Start the build process.
		m_ctx->startTimer(RC_TIMER_TOTAL);

		m_ctx->log(RC_LOG_PROGRESS, "Building navigation:");
		m_ctx->log(RC_LOG_PROGRESS, " - %d x %d cells", m_cfg.width, m_cfg.height);
		//m_ctx->log(RC_LOG_PROGRESS, " - %.1fK verts, %.1fK tris", nverts / 1000.0f, ntris / 1000.0f);

		// Allocate voxel heightfield where we rasterize our input data to.
		m_solid.reset(rcAllocHeightfield(), rcFreeHeightField);
		if (!m_solid)
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'solid'.");
			return;
		}
		if (!rcCreateHeightfield(m_ctx, *m_solid, m_cfg.width, m_cfg.height, m_cfg.bmin, m_cfg.bmax, m_cfg.cs, m_cfg.ch))
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not create solid heightfield.");
			return;
		}

		//// Allocate array that can hold triangle flags.
		//// If you have multiple meshes you need to process, allocate
		//// and array which can hold the max number of triangles you need to process.
		//m_triareas.reset(new unsigned char[chunkyMesh->maxTrisPerChunk]);
		//if (!m_triareas)
		//{
		//	m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'm_triareas' (%d).", chunkyMesh->maxTrisPerChunk);
		//	return;
		//}

		//float tbmin[2], tbmax[2];
		//tbmin[0] = m_cfg.bmin[0];
		//tbmin[1] = m_cfg.bmin[2];
		//tbmax[0] = m_cfg.bmax[0];
		//tbmax[1] = m_cfg.bmax[2];
		//int cid[512];// TODO: Make grow when returning too many items.
		//const int ncid = rcGetChunksOverlappingRect(chunkyMesh, tbmin, tbmax, cid, 512);
		//if (!ncid)
		//	return;

		//m_tileTriCount = 0;

		//for (int i = 0; i < ncid; ++i)
		//{
		//	const rcChunkyTriMeshNode& node = chunkyMesh->nodes[cid[i]];
		//	const int* ctris = &chunkyMesh->tris[node.i * 3];
		//	const int nctris = node.n;

		//	m_tileTriCount += nctris;

		//	memset(m_triareas, 0, nctris * sizeof(unsigned char));
		//	rcMarkWalkableTriangles(this, m_cfg.walkableSlopeAngle,
		//		verts, nverts, ctris, nctris, m_triareas);

		//	if (!rcRasterizeTriangles(this, verts, nverts, ctris, m_triareas, nctris, *m_solid, m_cfg.walkableClimb))
		//		return;
		//}

		//if (!m_keepInterResults)
		//{
		//	delete[] m_triareas;
		//	m_triareas = 0;
		//}

		struct Callback : public my::OctNode::QueryCallback
		{
			const CNavigationDlg* pdlg;
			rcContext* m_ctx;
			BuildTileMeshTask* ptask;
			my::AABB bindingBox;
			const float walkableThr;
			Callback(const CNavigationDlg* _pdlg, rcContext* ctx, BuildTileMeshTask* _ptask, const my::AABB& _bindingBox, float _walkableThr)
				: pdlg(_pdlg)
				, m_ctx(ctx)
				, ptask(_ptask)
				, bindingBox(_bindingBox)
				, walkableThr(_walkableThr)
			{
			}
			virtual void OnQueryEntity(my::OctEntity* oct_entity, const my::AABB& aabb, my::IntersectionTests::IntersectionType)
			{
				const Actor* actor = dynamic_cast<Actor*>(oct_entity);
				ASSERT(actor);
				if (!actor->m_PxActor || !actor->m_PxActor->is<physx::PxRigidStatic>())
				{
					return;
				}
				Actor::ComponentPtrList::const_iterator cmp_iter = actor->m_Cmps.begin();
				for (; cmp_iter != actor->m_Cmps.end(); cmp_iter++)
				{
					const Component* cmp = cmp_iter->get();
					switch (cmp->m_PxShapeGeometryType)
					{
					case physx::PxGeometryType::eSPHERE:
					case physx::PxGeometryType::eCAPSULE:
					{
						m_ctx->log(RC_LOG_PROGRESS, "buildNavigation: unsupported collision shape"); // ! rcAddSpan
						continue;
					}
					case physx::PxGeometryType::ePLANE:
					{
						physx::PxPlaneGeometry plane_geom;
						VERIFY(cmp->m_PxShape->getPlaneGeometry(plane_geom));
						physx::PxTransform localPose = cmp->m_PxShape->getLocalPose();
						my::Matrix4 World = my::Matrix4::Compose(
							my::Vector3::one, (my::Quaternion&)localPose.q, (my::Vector3&)localPose.p) * my::Matrix4::Compose(my::Vector3::one, actor->m_Rotation, actor->m_Position);
						my::Plane plane = my::Plane(1, 0, 0, 0).transform(World.inverse().transpose());
						if (D3DXToDegree(acos(my::Vector3::CosTheta(plane.normal, my::Vector3::unitY))) > pdlg->m_agentMaxSlope)
						{
							continue;
						}
						const float ics = 1.0f / ptask->m_cfg.cs;
						const float ich = 1.0f / ptask->m_cfg.ch;
						int x1 = (int)((ptask->m_cfg.bmax[0] - ptask->m_cfg.bmin[0]) * ics);
						int z1 = (int)((ptask->m_cfg.bmax[2] - ptask->m_cfg.bmin[2]) * ics);
						for (int x = 0; x < x1; x++)
						{
							for (int z = 0; z < z1; z++)
							{
								float sx = ptask->m_cfg.bmin[0] + (x + 0.5f) * ptask->m_cfg.cs;
								float sz = ptask->m_cfg.bmin[2] + (z + 0.5f) * ptask->m_cfg.cs;
								float sy = -(plane.a * sx + plane.c * sz + plane.d) / plane.b;
								if (sy < ptask->m_cfg.bmin[1] || sy >= ptask->m_cfg.bmax[1])
								{
									continue;
								}
								float sh = my::Max(fabs(plane.normal.x * ptask->m_cfg.cs / plane.normal.y), fabs(plane.normal.z * ptask->m_cfg.cs / plane.normal.y));
								unsigned short smin = my::Clamp((int)floorf((sy - sh - ptask->m_cfg.bmin[1]) * ich), 0, RC_SPAN_MAX_HEIGHT);
								unsigned short smax = my::Max((int)ceilf((sy - ptask->m_cfg.bmin[1]) * ich), smin + 1);
								rcAddSpan(m_ctx, *ptask->m_solid, x, z, smin, smax, plane.normal.y > walkableThr ? RC_WALKABLE_AREA : 0, ptask->m_cfg.walkableClimb);
							}
						}
						break;
					}
					case physx::PxGeometryType::eBOX:
					{
						physx::PxBoxGeometry geom;
						VERIFY(cmp->m_PxShape->getBoxGeometry(geom));
						my::Mesh mesh;
						mesh.CreateBox(geom.halfExtents.x * 2, geom.halfExtents.y * 2, geom.halfExtents.z * 2);
						D3DVERTEXELEMENT9 d3delems[MAX_FVF_DECL_SIZE];
						mesh.GetDeclaration(d3delems);
						my::D3DVertexElementSet elems(d3delems);
						physx::PxTransform localPose = cmp->m_PxShape->getLocalPose();
						my::Matrix4 World = my::Matrix4::Compose(
							my::Vector3::one, (my::Quaternion&)localPose.q, (my::Vector3&)localPose.p) * my::Matrix4::Compose(my::Vector3::one, actor->m_Rotation, actor->m_Position);
						void* pVertices = mesh.LockVertexBuffer(D3DLOCK_READONLY);
						_ASSERT(!(mesh.GetOptions() & D3DXMESH_32BIT));
						unsigned short* pIndices = (unsigned short*)mesh.LockIndexBuffer(D3DLOCK_READONLY);
						for (int i = 0; i < (int)mesh.GetNumFaces(); i++)
						{
							unsigned char* pV0 = (unsigned char*)pVertices + pIndices[i * 3 + 0] * mesh.GetNumBytesPerVertex();
							unsigned char* pV1 = (unsigned char*)pVertices + pIndices[i * 3 + 1] * mesh.GetNumBytesPerVertex();
							unsigned char* pV2 = (unsigned char*)pVertices + pIndices[i * 3 + 2] * mesh.GetNumBytesPerVertex();
							my::Vector3 v0 = elems.GetPosition(pV0).transformCoord(World);
							my::Vector3 v1 = elems.GetPosition(pV1).transformCoord(World);
							my::Vector3 v2 = elems.GetPosition(pV2).transformCoord(World);
							my::Vector3 Normal = (v1 - v0).cross(v2 - v0).normalize();
							rcRasterizeTriangle(m_ctx, &v0.x, &v1.x, &v2.x, Normal.y > walkableThr ? RC_WALKABLE_AREA : 0, *ptask->m_solid, ptask->m_cfg.walkableClimb);
						}
						mesh.UnlockVertexBuffer();
						mesh.UnlockIndexBuffer();
						break;
					}
					case physx::PxGeometryType::eCONVEXMESH:
					{
						const MeshComponent* mesh_cmp = dynamic_cast<const MeshComponent*>(cmp);
						boost::shared_ptr<physx::PxConvexMesh> convexMesh;
						if (!mesh_cmp->m_PxMesh)
						{
							PhysxInputData readBuffer(my::ResourceMgr::getSingleton().OpenIStream(mesh_cmp->m_PxMeshPath.c_str()));
							convexMesh.reset(PhysxSdk::getSingleton().m_sdk->createConvexMesh(readBuffer), PhysxDeleter<physx::PxConvexMesh>());
						}
						else
						{
							convexMesh.reset(mesh_cmp->m_PxMesh->m_ptr->is<physx::PxConvexMesh>(), PhysxDeleter<physx::PxConvexMesh>());
							static_cast<physx::Gu::ConvexMesh*>(convexMesh.get())->acquireReference();
						}
						boost::const_multi_array_ref<physx::PxVec3, 1> verts(convexMesh->getVertices(), boost::extents[convexMesh->getNbVertices()]);
						const physx::PxU8* polys = convexMesh->getIndexBuffer();
						for (unsigned int i = 0; i < convexMesh->getNbPolygons(); i++)
						{
							physx::PxHullPolygon hullpoly;
							convexMesh->getPolygonData(i, hullpoly);
							if (hullpoly.mNbVerts < 3)
							{
								m_ctx->log(RC_LOG_ERROR, "buildNavigation: invalid polygon");
								continue;
							}
							for (int j = 2; j < hullpoly.mNbVerts; j++)
							{
								my::Vector3 v0 = ((my::Vector3&)verts[polys[hullpoly.mIndexBase + 0]]).transformCoord(actor->m_World);
								my::Vector3 v1 = ((my::Vector3&)verts[polys[hullpoly.mIndexBase + j - 1]]).transformCoord(actor->m_World);
								my::Vector3 v2 = ((my::Vector3&)verts[polys[hullpoly.mIndexBase + j - 0]]).transformCoord(actor->m_World);
								rcRasterizeTriangle(m_ctx, &v0.x, &v1.x, &v2.x, hullpoly.mPlane[1] > walkableThr ? RC_WALKABLE_AREA : 0, *ptask->m_solid, ptask->m_cfg.walkableClimb);
							}
						}
						break;
					}
					case physx::PxGeometryType::eTRIANGLEMESH:
					{
						const MeshComponent* mesh_cmp = dynamic_cast<const MeshComponent*>(cmp);
						boost::shared_ptr<physx::PxTriangleMesh> triangleMesh;
						if (!mesh_cmp->m_PxMesh)
						{
							PhysxInputData readBuffer(my::ResourceMgr::getSingleton().OpenIStream(mesh_cmp->m_PxMeshPath.c_str()));
							triangleMesh.reset(PhysxSdk::getSingleton().m_sdk->createTriangleMesh(readBuffer), PhysxDeleter<physx::PxTriangleMesh>());
						}
						else
						{
							triangleMesh.reset(mesh_cmp->m_PxMesh->m_ptr->is<physx::PxTriangleMesh>(), PhysxDeleter<physx::PxTriangleMesh>());
							static_cast<physx::Gu::TriangleMesh*>(triangleMesh.get())->acquireReference();
						}
						boost::const_multi_array_ref<physx::PxVec3, 1> verts(triangleMesh->getVertices(), boost::extents[triangleMesh->getNbVertices()]);
						for (unsigned int i = 0; i < triangleMesh->getNbTriangles(); i++)
						{
							my::Vector3 v0, v1, v2;
							if (triangleMesh->getTriangleMeshFlags().isSet(physx::PxTriangleMeshFlag::e16_BIT_INDICES))
							{
								boost::const_multi_array_ref<unsigned short, 1> tris((unsigned short*)triangleMesh->getTriangles(), boost::extents[triangleMesh->getNbTriangles() * 3]);
								v0 = ((my::Vector3&)verts[tris[i * 3 + 0]]).transformCoord(actor->m_World);
								v1 = ((my::Vector3&)verts[tris[i * 3 + 1]]).transformCoord(actor->m_World);
								v2 = ((my::Vector3&)verts[tris[i * 3 + 2]]).transformCoord(actor->m_World);
							}
							else
							{
								boost::const_multi_array_ref<int, 1> tris((int*)triangleMesh->getTriangles(), boost::extents[triangleMesh->getNbTriangles() * 3]);
								v0 = ((my::Vector3&)verts[tris[i * 3 + 0]]).transformCoord(actor->m_World);
								v1 = ((my::Vector3&)verts[tris[i * 3 + 1]]).transformCoord(actor->m_World);
								v2 = ((my::Vector3&)verts[tris[i * 3 + 2]]).transformCoord(actor->m_World);
							}
							my::Vector3 Normal = (v1 - v0).cross(v2 - v0).normalize();
							rcRasterizeTriangle(m_ctx, &v0.x, &v1.x, &v2.x, Normal.y > walkableThr ? RC_WALKABLE_AREA : 0, * ptask->m_solid, ptask->m_cfg.walkableClimb);
						}
						break;
					}
					case physx::PxGeometryType::eHEIGHTFIELD:
					{
						struct Callback : public my::OctNode::QueryCallback
						{
							rcContext* m_ctx;
							BuildTileMeshTask* ptask;
							const float walkableThr;
							TerrainStream tstr;
							Callback(rcContext* ctx, BuildTileMeshTask* _ptask, float _walkableThr, Terrain* terrain)
								: m_ctx(ctx)
								, ptask(_ptask)
								, walkableThr(_walkableThr)
								, tstr(terrain)
							{
							}
							virtual void OnQueryEntity(my::OctEntity* oct_entity, const my::AABB& aabb, my::IntersectionTests::IntersectionType)
							{
								TerrainChunk* chunk = dynamic_cast<TerrainChunk*>(oct_entity);
								for (int m = 0; m < (int)tstr.m_terrain->m_ChunkSize; m++)
								{
									for (int n = 0; n < (int)tstr.m_terrain->m_ChunkSize; n++)
									{
										my::Vector3 v0 = tstr.GetPos(chunk->m_Row * tstr.m_terrain->m_ChunkSize + m + 0, chunk->m_Col * tstr.m_terrain->m_ChunkSize + n + 0).transformCoord(tstr.m_terrain->m_Actor->m_World);
										my::Vector3 v1 = tstr.GetPos(chunk->m_Row * tstr.m_terrain->m_ChunkSize + m + 1, chunk->m_Col * tstr.m_terrain->m_ChunkSize + n + 0).transformCoord(tstr.m_terrain->m_Actor->m_World);
										my::Vector3 v2 = tstr.GetPos(chunk->m_Row * tstr.m_terrain->m_ChunkSize + m + 1, chunk->m_Col * tstr.m_terrain->m_ChunkSize + n + 1).transformCoord(tstr.m_terrain->m_Actor->m_World);
										my::Vector3 v3 = tstr.GetPos(chunk->m_Row * tstr.m_terrain->m_ChunkSize + m + 0, chunk->m_Col * tstr.m_terrain->m_ChunkSize + n + 1).transformCoord(tstr.m_terrain->m_Actor->m_World);
										my::Vector3 Normal[2] = {
											(v1 - v0).cross(v3 - v0).normalize(),
											(v1 - v3).cross(v2 - v3).normalize() };
										rcRasterizeTriangle(m_ctx, &v0.x, &v1.x, &v3.x, Normal[0].y > walkableThr ? RC_WALKABLE_AREA : 0, *ptask->m_solid, ptask->m_cfg.walkableClimb);
										rcRasterizeTriangle(m_ctx, &v3.x, &v1.x, &v2.x, Normal[1].y > walkableThr ? RC_WALKABLE_AREA : 0, *ptask->m_solid, ptask->m_cfg.walkableClimb);
									}
								}
							}
						};
						Terrain* terrain = dynamic_cast<Terrain*>(cmp_iter->get());
						if (!terrain)
						{
							m_ctx->log(RC_LOG_ERROR, "buildNavigation: invalid terrain component");
							continue;
						}
						terrain->QueryEntity(bindingBox.transform(terrain->m_Actor->m_World.inverse()), &Callback(m_ctx, ptask, walkableThr, terrain));
						break;
					}
					}
				}
			}
		};

		CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, theApp.GetMainWnd());
		ASSERT(pFrame);
		Callback cb(pdlg, m_ctx, this, my::AABB(m_cfg.bmin[0], m_cfg.bmin[1], m_cfg.bmin[2], m_cfg.bmax[0], m_cfg.bmax[1], m_cfg.bmax[2]), cosf(m_cfg.walkableSlopeAngle / 180.0f * RC_PI));
		pFrame->QueryEntity(cb.bindingBox, &cb);

		// Once all geometry is rasterized, we do initial pass of filtering to
		// remove unwanted overhangs caused by the conservative rasterization
		// as well as filter spans where the character cannot possibly stand.
		if (pdlg->m_filterLowHangingObstacles)
			rcFilterLowHangingWalkableObstacles(m_ctx, m_cfg.walkableClimb, *m_solid);
		if (pdlg->m_filterLedgeSpans)
			rcFilterLedgeSpans(m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid);
		if (pdlg->m_filterWalkableLowHeightSpans)
			rcFilterWalkableLowHeightSpans(m_ctx, m_cfg.walkableHeight, *m_solid);

		// Compact the heightfield so that it is faster to handle from now on.
		// This will result more cache coherent data as well as the neighbours
		// between walkable cells will be calculated.
		m_chf.reset(rcAllocCompactHeightfield(), rcFreeCompactHeightfield);
		if (!m_chf)
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'chf'.");
			return;
		}
		if (!rcBuildCompactHeightfield(m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid, *m_chf))
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build compact data.");
			return;
		}

		//if (!m_keepInterResults)
		//{
		//	rcFreeHeightField(m_solid);
		//	m_solid = 0;
		//}

		// Erode the walkable area by agent radius.
		if (!rcErodeWalkableArea(m_ctx, m_cfg.walkableRadius, *m_chf))
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not erode.");
			return;
		}

		//// (Optional) Mark areas.
		//const ConvexVolume* vols = m_geom->getConvexVolumes();
		//for (int i = 0; i < m_geom->getConvexVolumeCount(); ++i)
		//	rcMarkConvexPolyArea(m_ctx, vols[i].verts, vols[i].nverts, vols[i].hmin, vols[i].hmax, (unsigned char)vols[i].area, *m_chf);


		// Partition the heightfield so that we can use simple algorithm later to triangulate the walkable areas.
		// There are 3 martitioning methods, each with some pros and cons:
		// 1) Watershed partitioning
		//   - the classic Recast partitioning
		//   - creates the nicest tessellation
		//   - usually slowest
		//   - partitions the heightfield into nice regions without holes or overlaps
		//   - the are some corner cases where this method creates produces holes and overlaps
		//      - holes may appear when a small obstacles is close to large open area (triangulation can handle this)
		//      - overlaps may occur if you have narrow spiral corridors (i.e stairs), this make triangulation to fail
		//   * generally the best choice if you precompute the nacmesh, use this if you have large open areas
		// 2) Monotone partioning
		//   - fastest
		//   - partitions the heightfield into regions without holes and overlaps (guaranteed)
		//   - creates long thin polygons, which sometimes causes paths with detours
		//   * use this if you want fast navmesh generation
		// 3) Layer partitoining
		//   - quite fast
		//   - partitions the heighfield into non-overlapping regions
		//   - relies on the triangulation code to cope with holes (thus slower than monotone partitioning)
		//   - produces better triangles than monotone partitioning
		//   - does not have the corner cases of watershed partitioning
		//   - can be slow and create a bit ugly tessellation (still better than monotone)
		//     if you have large open areas with small obstacles (not a problem if you use tiles)
		//   * good choice to use for tiled navmesh with medium and small sized tiles

		if (pdlg->m_partitionType == SAMPLE_PARTITION_WATERSHED)
		{
			// Prepare for region partitioning, by calculating distance field along the walkable surface.
			if (!rcBuildDistanceField(m_ctx, *m_chf))
			{
				m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build distance field.");
				return;
			}

			// Partition the walkable surface into simple regions without holes.
			if (!rcBuildRegions(m_ctx, *m_chf, m_cfg.borderSize, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
			{
				m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build watershed regions.");
				return;
			}
		}
		else if (pdlg->m_partitionType == SAMPLE_PARTITION_MONOTONE)
		{
			// Partition the walkable surface into simple regions without holes.
			// Monotone partitioning does not need distancefield.
			if (!rcBuildRegionsMonotone(m_ctx, *m_chf, m_cfg.borderSize, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
			{
				m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build monotone regions.");
				return;
			}
		}
		else // SAMPLE_PARTITION_LAYERS
		{
			// Partition the walkable surface into simple regions without holes.
			if (!rcBuildLayerRegions(m_ctx, *m_chf, m_cfg.borderSize, m_cfg.minRegionArea))
			{
				m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build layer regions.");
				return;
			}
		}

		// Create contours.
		m_cset.reset(rcAllocContourSet(), rcFreeContourSet);
		if (!m_cset)
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'cset'.");
			return;
		}
		if (!rcBuildContours(m_ctx, *m_chf, m_cfg.maxSimplificationError, m_cfg.maxEdgeLen, *m_cset))
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not create contours.");
			return;
		}

		if (m_cset->nconts == 0)
		{
			return;
		}

		// Build polygon navmesh from the contours.
		m_pmesh.reset(rcAllocPolyMesh(), rcFreePolyMesh);
		if (!m_pmesh)
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'pmesh'.");
			return;
		}
		if (!rcBuildPolyMesh(m_ctx, *m_cset, m_cfg.maxVertsPerPoly, *m_pmesh))
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not triangulate contours.");
			return;
		}

		// Build detail mesh.
		m_dmesh.reset(rcAllocPolyMeshDetail(), rcFreePolyMeshDetail);
		if (!m_dmesh)
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'dmesh'.");
			return;
		}

		if (!rcBuildPolyMeshDetail(m_ctx, *m_pmesh, *m_chf,
			m_cfg.detailSampleDist, m_cfg.detailSampleMaxError,
			*m_dmesh))
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could build polymesh detail.");
			return;
		}

		//if (!m_keepInterResults)
		//{
		//	rcFreeCompactHeightfield(m_chf);
		//	m_chf = 0;
		//	rcFreeContourSet(m_cset);
		//	m_cset = 0;
		//}

		unsigned char* navData = 0;
		int navDataSize = 0;
		if (m_cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON)
		{
			if (m_pmesh->nverts >= 0xffff)
			{
				// The vertex indices are ushorts, and cannot point to more than 0xffff vertices.
				m_ctx->log(RC_LOG_ERROR, "Too many vertices per tile %d (max: %d).", m_pmesh->nverts, 0xffff);
				return;
			}

			// Update poly flags from areas.
			for (int i = 0; i < m_pmesh->npolys; ++i)
			{
				if (m_pmesh->areas[i] == RC_WALKABLE_AREA)
					m_pmesh->areas[i] = SAMPLE_POLYAREA_GROUND;

				if (m_pmesh->areas[i] == SAMPLE_POLYAREA_GROUND ||
					m_pmesh->areas[i] == SAMPLE_POLYAREA_GRASS ||
					m_pmesh->areas[i] == SAMPLE_POLYAREA_ROAD)
				{
					m_pmesh->flags[i] = SAMPLE_POLYFLAGS_WALK;
				}
				else if (m_pmesh->areas[i] == SAMPLE_POLYAREA_WATER)
				{
					m_pmesh->flags[i] = SAMPLE_POLYFLAGS_SWIM;
				}
				else if (m_pmesh->areas[i] == SAMPLE_POLYAREA_DOOR)
				{
					m_pmesh->flags[i] = SAMPLE_POLYFLAGS_WALK | SAMPLE_POLYFLAGS_DOOR;
				}
			}

			dtNavMeshCreateParams params;
			memset(&params, 0, sizeof(params));
			params.verts = m_pmesh->verts;
			params.vertCount = m_pmesh->nverts;
			params.polys = m_pmesh->polys;
			params.polyAreas = m_pmesh->areas;
			params.polyFlags = m_pmesh->flags;
			params.polyCount = m_pmesh->npolys;
			params.nvp = m_pmesh->nvp;
			params.detailMeshes = m_dmesh->meshes;
			params.detailVerts = m_dmesh->verts;
			params.detailVertsCount = m_dmesh->nverts;
			params.detailTris = m_dmesh->tris;
			params.detailTriCount = m_dmesh->ntris;
			//params.offMeshConVerts = m_geom->getOffMeshConnectionVerts();
			//params.offMeshConRad = m_geom->getOffMeshConnectionRads();
			//params.offMeshConDir = m_geom->getOffMeshConnectionDirs();
			//params.offMeshConAreas = m_geom->getOffMeshConnectionAreas();
			//params.offMeshConFlags = m_geom->getOffMeshConnectionFlags();
			//params.offMeshConUserID = m_geom->getOffMeshConnectionId();
			//params.offMeshConCount = m_geom->getOffMeshConnectionCount();
			params.walkableHeight = pdlg->m_agentHeight;
			params.walkableRadius = pdlg->m_agentRadius;
			params.walkableClimb = pdlg->m_agentMaxClimb;
			params.tileX = tx;
			params.tileY = ty;
			params.tileLayer = 0;
			rcVcopy(params.bmin, m_pmesh->bmin);
			rcVcopy(params.bmax, m_pmesh->bmax);
			params.cs = m_cfg.cs;
			params.ch = m_cfg.ch;
			params.buildBvTree = true;

			if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
			{
				m_ctx->log(RC_LOG_ERROR, "Could not build Detour navmesh.");
				return;
			}
		}
		m_tileMemUsage = navDataSize / 1024.0f;

		m_ctx->stopTimer(RC_TIMER_TOTAL);

		// Show performance stats.
		duLogBuildTimes(*m_ctx, m_ctx->getAccumulatedTime(RC_TIMER_TOTAL));
		m_ctx->log(RC_LOG_PROGRESS, ">> Polymesh: %d vertices  %d polygons", m_pmesh->nverts, m_pmesh->npolys);

		m_tileBuildTime = m_ctx->getAccumulatedTime(RC_TIMER_TOTAL) / 1000.0f;

		dataSize = navDataSize;
		data = navData;
		m_solid.reset();
		//m_triareas.reset();
		m_chf.reset();
		m_cset.reset();
		m_pmesh.reset();
		m_dmesh.reset();
	}
};

inline static unsigned int nextPow2(unsigned int v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

inline static unsigned int ilog2(unsigned int v)
{
	unsigned int r;
	unsigned int shift;
	r = (v > 0xffff) << 4; v >>= r;
	shift = (v > 0xff) << 3; v >>= shift; r |= shift;
	shift = (v > 0xf) << 2; v >>= shift; r |= shift;
	shift = (v > 0x3) << 1; v >>= shift; r |= shift;
	r |= (v >> 1);
	return r;
}

void CNavigationDlg::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class
	if (!UpdateData(TRUE))
	{
		TRACE(traceAppMsg, 0, "UpdateData failed during dialog termination.\n");
		// the UpdateData routine will set focus to correct item
		return;
	}

	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);

	int gw = 0, gh = 0;
	const float* bmin = &m_bindingBox.m_min.x;
	const float* bmax = &m_bindingBox.m_max.x;
	rcCalcGridSize(bmin, bmax, m_cellSize, &gw, &gh);
	const int ts = (int)m_tileSize;
	const int tw = (gw + ts - 1) / ts;
	const int th = (gh + ts - 1) / ts;
	const float tcs = m_tileSize * m_cellSize;

	// Max tiles and max polys affect how the tile IDs are caculated.
	// There are 22 bits available for identifying a tile and a polygon.
	int tileBits = rcMin((int)ilog2(nextPow2(tw * th)), 14);
	if (tileBits > 14) tileBits = 14;
	int polyBits = 22 - tileBits;
	m_maxTiles = 1 << tileBits;
	m_maxPolysPerTile = 1 << polyBits;

	m_navMesh.reset(dtAllocNavMesh(), dtFreeNavMesh);
	if (!m_navMesh)
	{
		this->log(RC_LOG_ERROR, "buildTiledNavigation: Could not allocate navmesh.");
		return;
	}

	dtNavMeshParams params;
	rcVcopy(params.orig, &m_bindingBox.m_min.x);
	params.tileWidth = m_tileSize * m_cellSize;
	params.tileHeight = m_tileSize * m_cellSize;
	params.maxTiles = m_maxTiles;
	params.maxPolys = m_maxPolysPerTile;

	dtStatus status;

	status = m_navMesh->init(&params);
	if (dtStatusFailed(status))
	{
		this->log(RC_LOG_ERROR, "buildTiledNavigation: Could not init navmesh.");
		return;
	}

	// Start the build process.
	this->startTimer(RC_TIMER_TEMP);

	theApp.LeaveDeviceSection();

	float m_lastBuiltTileBmin[3];
	float m_lastBuiltTileBmax[3];
	std::vector<my::ParallelTaskPtr> tasks;
	for (int y = 0; y < th; ++y)
	{
		for (int x = 0; x < tw; ++x)
		{
			m_lastBuiltTileBmin[0] = bmin[0] + x * tcs;
			m_lastBuiltTileBmin[1] = bmin[1];
			m_lastBuiltTileBmin[2] = bmin[2] + y * tcs;

			m_lastBuiltTileBmax[0] = bmin[0] + (x + 1) * tcs;
			m_lastBuiltTileBmax[1] = bmax[1];
			m_lastBuiltTileBmax[2] = bmin[2] + (y + 1) * tcs;

			//int dataSize = 0;
			//unsigned char* data = buildTileMesh(x, y, m_lastBuiltTileBmin, m_lastBuiltTileBmax, dataSize);
			//if (data)
			//{
			//	// Remove any previous data (navmesh owns and deletes the data).
			//	m_navMesh->removeTile(m_navMesh->getTileRefAt(x, y, 0), 0, 0);
			//	// Let the navmesh own the data.
			//	dtStatus status = m_navMesh->addTile(data, dataSize, DT_TILE_FREE_DATA, 0, 0);
			//	if (dtStatusFailed(status))
			//		dtFree(data);
			//}

			my::ParallelTaskPtr task(new BuildTileMeshTask(x, y, m_lastBuiltTileBmin, m_lastBuiltTileBmax, this, this));
			theApp.PushTask(task.get());
			tasks.push_back(task);
		}
	}

	theApp.DoAllParallelTasks();

	theApp.EnterDeviceSection();

	for (int i = 0; i < tasks.size(); i++)
	{
		boost::shared_ptr<BuildTileMeshTask> task = boost::static_pointer_cast<BuildTileMeshTask>(tasks[i]);
		if (task->data)
		{
			//// Remove any previous data (navmesh owns and deletes the data).
			//m_navMesh->removeTile(m_navMesh->getTileRefAt(x, y, 0), 0, 0);
			// Let the navmesh own the data.
			dtStatus status = m_navMesh->addTile(task->data, task->dataSize, DT_TILE_FREE_DATA, 0, 0);
			if (dtStatusFailed(status))
			{
				dtFree(task->data);
			}
		}
	}

	this->stopTimer(RC_TIMER_TOTAL);

	// Show performance stats.
	duLogBuildTimes(*this, this->getAccumulatedTime(RC_TIMER_TOTAL));

	CDialogEx::OnOK();
}
