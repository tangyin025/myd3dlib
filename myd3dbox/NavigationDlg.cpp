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
#include "Terrain.h"


// CNavigationDlg dialog

IMPLEMENT_DYNAMIC(CNavigationDlg, CDialogEx)

CNavigationDlg::CNavigationDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CNavigationDlg::IDD, pParent)
	, m_solid(NULL)
	//, m_triareas(NULL)
	, m_chf(NULL)
	, m_cset(NULL)
	, m_pmesh(NULL)
	, m_dmesh(NULL)
	, m_navMesh(NULL)
	, m_navQuery(NULL)
{
	m_navQuery = dtAllocNavMeshQuery();
}

CNavigationDlg::~CNavigationDlg()
{
}

void CNavigationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CNavigationDlg, CDialogEx)
END_MESSAGE_MAP()


// CNavigationDlg message handlers

void CNavigationDlg::doLog(const rcLogCategory category, const char* msg, const int len)
{
	theApp.m_EventLog(msg);
}

void CNavigationDlg::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);

	//const float* verts = 0;// m_geom->getMesh()->getVerts();
	//const int nverts = 100;// m_geom->getMesh()->getVertCount();
	//const int* tris = 0;// m_geom->getMesh()->getTris();
	//const int ntris = 10;// m_geom->getMesh()->getTriCount();

	//
	// Step 1. Initialize build config.
	//

	// Init build configuration from GUI
	memset(&m_cfg, 0, sizeof(m_cfg));
	m_cfg.cs = 0.3f;// m_cellSize;
	m_cfg.ch = 0.2f;// m_cellHeight;
	m_cfg.walkableSlopeAngle = 45.0f;// m_agentMaxSlope;
	m_cfg.walkableHeight = 10;// (int)ceilf(m_agentHeight / m_cfg.ch);
	m_cfg.walkableClimb = 4;// (int)floorf(m_agentMaxClimb / m_cfg.ch);
	m_cfg.walkableRadius = 2;// (int)ceilf(m_agentRadius / m_cfg.cs);
	m_cfg.maxEdgeLen = 40;// (int)(m_edgeMaxLen / m_cellSize);
	m_cfg.maxSimplificationError = 1.3f;// m_edgeMaxError;
	m_cfg.minRegionArea = 64;// (int)rcSqr(m_regionMinSize);		// Note: area = size*size
	m_cfg.mergeRegionArea = 400;// (int)rcSqr(m_regionMergeSize);	// Note: area = size*size
	m_cfg.maxVertsPerPoly = 6;// (int)m_vertsPerPoly;
	m_cfg.detailSampleDist = 1.8f;// m_detailSampleDist < 0.9f ? 0 : m_cellSize * m_detailSampleDist;
	m_cfg.detailSampleMaxError = 0.2f;// m_cellHeight * m_detailSampleMaxError;

	//rcVcopy(m_cfg.bmin, &pFrame->m_min.x);
	//rcVcopy(m_cfg.bmax, &pFrame->m_max.x);
	my::AABB box(-100, 100);
	rcVcopy(m_cfg.bmin, &box.m_min.x);
	rcVcopy(m_cfg.bmax, &box.m_max.x);
	rcCalcGridSize(m_cfg.bmin, m_cfg.bmax, m_cfg.cs, &m_cfg.width, &m_cfg.height);

	// Reset build times gathering.
	this->resetTimers();

	// Start the build process.	
	this->startTimer(RC_TIMER_TOTAL);

	this->log(RC_LOG_PROGRESS, "Building navigation:");
	this->log(RC_LOG_PROGRESS, " - %d x %d cells", m_cfg.width, m_cfg.height);
	//this->log(RC_LOG_PROGRESS, " - %.1fK verts, %.1fK tris", nverts / 1000.0f, ntris / 1000.0f);

	//
	// Step 2. Rasterize input polygon soup.
	//

	// Allocate voxel heightfield where we rasterize our input data to.
	m_solid = rcAllocHeightfield();
	if (!m_solid)
	{
		this->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'solid'.");
		return;
	}

	if (!rcCreateHeightfield(this, *m_solid, m_cfg.width, m_cfg.height, m_cfg.bmin, m_cfg.bmax, m_cfg.cs, m_cfg.ch))
	{
		this->log(RC_LOG_ERROR, "buildNavigation: Could not create solid heightfield.");
		return;
	}

	//// Allocate array that can hold triangle area types.
	//// If you have multiple meshes you need to process, allocate
	//// and array which can hold the max number of triangles you need to process.
	//m_triareas = new unsigned char[ntris];
	//if (!m_triareas)
	//{
	//	this->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'm_triareas' (%d).", ntris);
	//	return;
	//}

	//// Find triangles which are walkable based on their slope and rasterize them.
	//// If your input data is multiple meshes, you can transform them here, calculate
	//// the are type for each of the meshes and rasterize them.
	//memset(m_triareas, 0, ntris*sizeof(unsigned char));
	//rcMarkWalkableTriangles(this, m_cfg.walkableSlopeAngle, verts, nverts, tris, ntris, m_triareas);
	//{
	//	this->log(RC_LOG_ERROR, "buildNavigation: Could not rasterize triangles.");
	//	return;
	//if (!rcRasterizeTriangles(this, verts, nverts, tris, m_triareas, ntris, *m_solid, m_cfg.walkableClimb))
	//}

	struct Callback : public my::OctNode::QueryCallback
	{
		CNavigationDlg * pDlg;
		Callback(CNavigationDlg * _pDlg)
			: pDlg(_pDlg)
		{
		}
		virtual void OnQueryEntity(my::OctEntity * oct_entity, const my::AABB & aabb, my::IntersectionTests::IntersectionType)
		{
			Actor * actor = dynamic_cast<Actor *>(oct_entity);
			ASSERT(actor);
			if (!actor->m_PxActor || !actor->m_PxActor->is<physx::PxRigidStatic>())
			{
				return;
			}
			const float walkableThr = cosf(pDlg->m_cfg.walkableSlopeAngle / 180.0f*RC_PI);
			Actor::ComponentPtrList::iterator cmp_iter = actor->m_Cmps.begin();
			for (; cmp_iter != actor->m_Cmps.end(); cmp_iter++)
			{
				Component * cmp = cmp_iter->get();
				if (!cmp->m_PxShape)
				{
					continue;
				}
				switch (cmp->m_PxShape->getGeometryType())
				{
				case physx::PxGeometryType::eSPHERE:
				case physx::PxGeometryType::ePLANE:
				case physx::PxGeometryType::eCAPSULE:
				case physx::PxGeometryType::eBOX:
					TRACE("OnToolsBuildnavigation: unsupported collision shape");
					continue;
				case physx::PxGeometryType::eCONVEXMESH:
				{
					physx::PxConvexMeshGeometry geom;
					VERIFY(cmp->m_PxShape->getConvexMeshGeometry(geom));
					boost::const_multi_array_ref<physx::PxVec3, 1> verts(geom.convexMesh->getVertices(), boost::extents[geom.convexMesh->getNbVertices()]);
					const physx::PxU8 * polys = geom.convexMesh->getIndexBuffer();
					for (unsigned int i = 0; i < geom.convexMesh->getNbPolygons(); i++)
					{
						physx::PxHullPolygon hullpoly;
						geom.convexMesh->getPolygonData(i, hullpoly);
						if (hullpoly.mNbVerts < 3)
						{
							TRACE("OnToolsBuildnavigation: invalid polygon");
							continue;
						}
						for (int j = 2; j < hullpoly.mNbVerts; j++)
						{
							my::Vector3 v0 = ((my::Vector3 &)verts[polys[hullpoly.mIndexBase + 0]]).transformCoord(actor->m_World);
							my::Vector3 v1 = ((my::Vector3 &)verts[polys[hullpoly.mIndexBase + j - 1]]).transformCoord(actor->m_World);
							my::Vector3 v2 = ((my::Vector3 &)verts[polys[hullpoly.mIndexBase + j - 0]]).transformCoord(actor->m_World);
							rcRasterizeTriangle(pDlg, &v0.x, &v1.x, &v2.x, hullpoly.mPlane[1] > walkableThr ? RC_WALKABLE_AREA : 0, *pDlg->m_solid, pDlg->m_cfg.walkableClimb);
						}
					}
					break;
				}
				case physx::PxGeometryType::eTRIANGLEMESH:
				{
					physx::PxTriangleMeshGeometry geom;
					VERIFY(cmp->m_PxShape->getTriangleMeshGeometry(geom));
					boost::const_multi_array_ref<physx::PxVec3, 1> verts(geom.triangleMesh->getVertices(), boost::extents[geom.triangleMesh->getNbVertices()]);
					for (unsigned int i = 0; i < geom.triangleMesh->getNbTriangles(); i++)
					{
						my::Vector3 v0, v1, v2;
						if (geom.triangleMesh->getTriangleMeshFlags().isSet(physx::PxTriangleMeshFlag::e16_BIT_INDICES))
						{
							boost::const_multi_array_ref<unsigned short, 1> tris((unsigned short *)geom.triangleMesh->getTriangles(), boost::extents[geom.triangleMesh->getNbTriangles() * 3]);
							v0 = ((my::Vector3 &)verts[tris[i * 3 + 0]]).transformCoord(actor->m_World);
							v1 = ((my::Vector3 &)verts[tris[i * 3 + 1]]).transformCoord(actor->m_World);
							v2 = ((my::Vector3 &)verts[tris[i * 3 + 2]]).transformCoord(actor->m_World);
						}
						else
						{
							boost::const_multi_array_ref<int, 1> tris((int *)geom.triangleMesh->getTriangles(), boost::extents[geom.triangleMesh->getNbTriangles() * 3]);
							v0 = ((my::Vector3 &)verts[tris[i * 3 + 0]]).transformCoord(actor->m_World);
							v1 = ((my::Vector3 &)verts[tris[i * 3 + 1]]).transformCoord(actor->m_World);
							v2 = ((my::Vector3 &)verts[tris[i * 3 + 2]]).transformCoord(actor->m_World);
						}
						my::Vector3 Normal = (v1 - v0).cross(v2 - v0).normalize();
						rcRasterizeTriangle(pDlg, &v0.x, &v1.x, &v2.x, Normal.y > walkableThr ? RC_WALKABLE_AREA : 0, *pDlg->m_solid, pDlg->m_cfg.walkableClimb);
					}
					break;
				}
				case physx::PxGeometryType::eHEIGHTFIELD:
				{
					Terrain * terrain = dynamic_cast<Terrain*>(cmp_iter->get());
					if (!terrain)
					{
						TRACE("OnToolsBuildnavigation: invalid terrain component");
						continue;
					}
					TerrainStream tstr(terrain, false);
					for (int i = 0; i < terrain->m_RowChunks * terrain->m_ChunkSize; i++)
					{
						for (int j = 0; j < terrain->m_ColChunks * terrain->m_ChunkSize; j++)
						{
							my::Vector3 v0 = tstr.GetPos(i + 0, j + 0).transformCoord(actor->m_World);
							my::Vector3 v1 = tstr.GetPos(i + 1, j + 0).transformCoord(actor->m_World);
							my::Vector3 v2 = tstr.GetPos(i + 1, j + 1).transformCoord(actor->m_World);
							my::Vector3 v3 = tstr.GetPos(i + 0, j + 1).transformCoord(actor->m_World);
							my::Vector3 Normal[2] = {
								(v1 - v0).cross(v3 - v0).normalize(),
								(v1 - v3).cross(v2 - v3).normalize()
							};
							rcRasterizeTriangle(pDlg, &v0.x, &v1.x, &v3.x, Normal[0].y > walkableThr ? RC_WALKABLE_AREA : 0, *pDlg->m_solid, pDlg->m_cfg.walkableClimb);
							rcRasterizeTriangle(pDlg, &v3.x, &v1.x, &v2.x, Normal[1].y > walkableThr ? RC_WALKABLE_AREA : 0, *pDlg->m_solid, pDlg->m_cfg.walkableClimb);
						}
					}
					tstr.Release();
					break;
				}
				}
			}
		}
	};
	pFrame->QueryEntityAll(&Callback(this));

	//if (!m_keepInterResults)
	//{
	//	delete[] m_triareas;
	//	m_triareas = 0;
	//}

	bool m_filterLowHangingObstacles = true;
	bool m_filterLedgeSpans = true;
	bool m_filterWalkableLowHeightSpans = true;

	//
	// Step 3. Filter walkables surfaces.
	//

	// Once all geoemtry is rasterized, we do initial pass of filtering to
	// remove unwanted overhangs caused by the conservative rasterization
	// as well as filter spans where the character cannot possibly stand.
	if (m_filterLowHangingObstacles)
		rcFilterLowHangingWalkableObstacles(this, m_cfg.walkableClimb, *m_solid);
	if (m_filterLedgeSpans)
		rcFilterLedgeSpans(this, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid);
	if (m_filterWalkableLowHeightSpans)
		rcFilterWalkableLowHeightSpans(this, m_cfg.walkableHeight, *m_solid);

	//
	// Step 4. Partition walkable surface to simple regions.
	//

	// Compact the heightfield so that it is faster to handle from now on.
	// This will result more cache coherent data as well as the neighbours
	// between walkable cells will be calculated.
	m_chf = rcAllocCompactHeightfield();
	if (!m_chf)
	{
		this->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'chf'.");
		return;
	}
	if (!rcBuildCompactHeightfield(this, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid, *m_chf))
	{
		this->log(RC_LOG_ERROR, "buildNavigation: Could not build compact data.");
		return;
	}

	//if (!m_keepInterResults)
	//{
	//	rcFreeHeightField(m_solid);
	//	m_solid = 0;
	//}

	// Erode the walkable area by agent radius.
	if (!rcErodeWalkableArea(this, m_cfg.walkableRadius, *m_chf))
	{
		this->log(RC_LOG_ERROR, "buildNavigation: Could not erode.");
		return;
	}

	//// (Optional) Mark areas.
	//const ConvexVolume* vols = m_geom->getConvexVolumes();
	//for (int i = 0; i < m_geom->getConvexVolumeCount(); ++i)
	//	rcMarkConvexPolyArea(this, vols[i].verts, vols[i].nverts, vols[i].hmin, vols[i].hmax, (unsigned char)vols[i].area, *m_chf);

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

	enum SamplePartitionType
	{
		SAMPLE_PARTITION_WATERSHED,
		SAMPLE_PARTITION_MONOTONE,
		SAMPLE_PARTITION_LAYERS,
	};

	int m_partitionType = SAMPLE_PARTITION_WATERSHED;

	if (m_partitionType == SAMPLE_PARTITION_WATERSHED)
	{
		// Prepare for region partitioning, by calculating distance field along the walkable surface.
		if (!rcBuildDistanceField(this, *m_chf))
		{
			this->log(RC_LOG_ERROR, "buildNavigation: Could not build distance field.");
			return;
		}

		// Partition the walkable surface into simple regions without holes.
		if (!rcBuildRegions(this, *m_chf, 0, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
		{
			this->log(RC_LOG_ERROR, "buildNavigation: Could not build watershed regions.");
			return;
		}
	}
	else if (m_partitionType == SAMPLE_PARTITION_MONOTONE)
	{
		// Partition the walkable surface into simple regions without holes.
		// Monotone partitioning does not need distancefield.
		if (!rcBuildRegionsMonotone(this, *m_chf, 0, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
		{
			this->log(RC_LOG_ERROR, "buildNavigation: Could not build monotone regions.");
			return;
		}
	}
	else // SAMPLE_PARTITION_LAYERS
	{
		// Partition the walkable surface into simple regions without holes.
		if (!rcBuildLayerRegions(this, *m_chf, 0, m_cfg.minRegionArea))
		{
			this->log(RC_LOG_ERROR, "buildNavigation: Could not build layer regions.");
			return;
		}
	}

	//
	// Step 5. Trace and simplify region contours.
	//

	// Create contours.
	m_cset = rcAllocContourSet();
	if (!m_cset)
	{
		this->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'cset'.");
		return;
	}
	if (!rcBuildContours(this, *m_chf, m_cfg.maxSimplificationError, m_cfg.maxEdgeLen, *m_cset))
	{
		this->log(RC_LOG_ERROR, "buildNavigation: Could not create contours.");
		return;
	}

	//
	// Step 6. Build polygons mesh from contours.
	//

	// Build polygon navmesh from the contours.
	m_pmesh = rcAllocPolyMesh();
	if (!m_pmesh)
	{
		this->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'pmesh'.");
		return;
	}
	if (!rcBuildPolyMesh(this, *m_cset, m_cfg.maxVertsPerPoly, *m_pmesh))
	{
		this->log(RC_LOG_ERROR, "buildNavigation: Could not triangulate contours.");
		return;
	}

	//
	// Step 7. Create detail mesh which allows to access approximate height on each polygon.
	//

	m_dmesh = rcAllocPolyMeshDetail();
	if (!m_dmesh)
	{
		this->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'pmdtl'.");
		return;
	}

	if (!rcBuildPolyMeshDetail(this, *m_pmesh, *m_chf, m_cfg.detailSampleDist, m_cfg.detailSampleMaxError, *m_dmesh))
	{
		this->log(RC_LOG_ERROR, "buildNavigation: Could not build detail mesh.");
		return;
	}

	//if (!m_keepInterResults)
	//{
	//	rcFreeCompactHeightfield(m_chf);
	//	m_chf = 0;
	//	rcFreeContourSet(m_cset);
	//	m_cset = 0;
	//}

	// At this point the navigation mesh data is ready, you can access it from m_pmesh.
	// See duDebugDrawPolyMesh or dtCreateNavMeshData as examples how to access the data.

	//
	// (Optional) Step 8. Create Detour data from Recast poly mesh.
	//

	// The GUI may allow more max points per polygon than Detour can handle.
	// Only build the detour navmesh if we do not exceed the limit.
	if (m_cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON)
	{
		unsigned char* navData = 0;
		int navDataSize = 0;

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
		params.walkableHeight = 2.0f;// m_agentHeight;
		params.walkableRadius = 0.6f;// m_agentRadius;
		params.walkableClimb = 0.9f;// m_agentMaxClimb;
		rcVcopy(params.bmin, m_pmesh->bmin);
		rcVcopy(params.bmax, m_pmesh->bmax);
		params.cs = m_cfg.cs;
		params.ch = m_cfg.ch;
		params.buildBvTree = true;

		if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
		{
			this->log(RC_LOG_ERROR, "Could not build Detour navmesh.");
			return;
		}

		m_navMesh = dtAllocNavMesh();
		if (!m_navMesh)
		{
			dtFree(navData);
			this->log(RC_LOG_ERROR, "Could not create Detour navmesh");
			return;
		}

		dtStatus status;

		status = m_navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
		if (dtStatusFailed(status))
		{
			dtFree(navData);
			this->log(RC_LOG_ERROR, "Could not init Detour navmesh");
			return;
		}

		status = m_navQuery->init(m_navMesh, 2048);
		if (dtStatusFailed(status))
		{
			this->log(RC_LOG_ERROR, "Could not init Detour navmesh query");
			return;
		}
	}

	this->stopTimer(RC_TIMER_TOTAL);

	// Show performance stats.
	duLogBuildTimes(*this, this->getAccumulatedTime(RC_TIMER_TOTAL));
	this->log(RC_LOG_PROGRESS, ">> Polymesh: %d vertices  %d polygons", m_pmesh->nverts, m_pmesh->npolys);

	CDialogEx::OnOK();
}
