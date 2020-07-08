#include "hlrecast.h"
#include <hl.h>

#include <iostream>
#include <DetourNavMesh.h>
#include <Recast.h>

template <typename T> class recast_struct {
	hl_type* t;
public:
	T value;
};

enum SamplePartitionType
{
	SAMPLE_PARTITION_WATERSHED,
	SAMPLE_PARTITION_MONOTONE,
	SAMPLE_PARTITION_LAYERS,
};

struct hlRecastConfig
{
	/// The width of the field along the x-axis. [Limit: >= 0] [Units: vx]
	int width;

	/// The height of the field along the z-axis. [Limit: >= 0] [Units: vx]
	int height;

	/// The width/height size of tile's on the xz-plane. [Limit: >= 0] [Units: vx]
	int tileSize;

	/// The size of the non-navigable border around the heightfield. [Limit: >=0] [Units: vx]
	int borderSize;

	/// The xz-plane cell size to use for fields. [Limit: > 0] [Units: wu] 
	float cs;

	/// The y-axis cell size to use for fields. [Limit: > 0] [Units: wu]
	float ch;

	/// The minimum bounds of the field's AABB. [(x, y, z)] [Units: wu]
	float bmin[3];

	/// The maximum bounds of the field's AABB. [(x, y, z)] [Units: wu]
	float bmax[3];

	/// The maximum slope that is considered walkable. [Limits: 0 <= value < 90] [Units: Degrees] 
	float walkableSlopeAngle;

	/// Minimum floor to 'ceiling' height that will still allow the floor area to 
	/// be considered walkable. [Limit: >= 3] [Units: vx] 
	int walkableHeight;

	/// Maximum ledge height that is considered to still be traversable. [Limit: >=0] [Units: vx] 
	int walkableClimb;

	/// The distance to erode/shrink the walkable area of the heightfield away from 
	/// obstructions.  [Limit: >=0] [Units: vx] 
	int walkableRadius;

	/// The maximum allowed length for contour edges along the border of the mesh. [Limit: >=0] [Units: vx] 
	int maxEdgeLen;

	/// The maximum distance a simplfied contour's border edges should deviate 
	/// the original raw contour. [Limit: >=0] [Units: vx]
	float maxSimplificationError;

	/// The minimum number of cells allowed to form isolated island areas. [Limit: >=0] [Units: vx] 
	int minRegionArea;

	/// Any regions with a span count smaller than this value will, if possible, 
	/// be merged with larger regions. [Limit: >=0] [Units: vx] 
	int mergeRegionArea;

	/// The maximum number of vertices allowed for polygons generated during the 
	/// contour to polygon conversion process. [Limit: >= 3] 
	int maxVertsPerPoly;

	/// Sets the sampling distance to use when generating the detail mesh.
	/// (For height detail only.) [Limits: 0 or >= 0.9] [Units: wu] 
	float detailSampleDist;

	/// The maximum distance the detail mesh surface should deviate from heightfield
	/// data. (For height detail only.) [Limit: >=0] [Units: wu] 
	float detailSampleMaxError;

	int partitionType;
};

rcConfig m_cfg;
rcHeightfield* m_solid;
unsigned char* m_triareas;
rcCompactHeightfield* m_chf;
rcContourSet* m_cset;
rcPolyMesh* m_pmesh;
rcPolyMeshDetail* m_dmesh;

bool m_keepInterResults = false;
bool m_filterLowHangingObstacles = true;
bool m_filterLedgeSpans = true;
bool m_filterWalkableLowHeightSpans = true;
int m_partitionType = SAMPLE_PARTITION_WATERSHED;

void cleanup() {
	m_cfg.walkableSlopeAngle = 0.0;
	m_cfg.walkableClimb = false;
}

void setConfig(const hlRecastConfig conf) {

	m_partitionType = conf.partitionType;
	m_cfg.cs = conf.cs;
	m_cfg.ch = conf.ch;
	m_cfg.walkableSlopeAngle = conf.walkableSlopeAngle;
	m_cfg.walkableHeight = conf.walkableHeight; // (int)ceilf(m_agentHeight / m_cfg.ch);
	m_cfg.walkableClimb = conf.walkableClimb; // (int)floorf(m_agentMaxClimb / m_cfg.ch);
	m_cfg.walkableRadius = conf.walkableRadius; // (int)ceilf(m_agentRadius / m_cfg.cs);
	m_cfg.maxEdgeLen = conf.maxEdgeLen; // (int)(m_edgeMaxLen / m_cellSize);
	m_cfg.maxSimplificationError = conf.maxSimplificationError; // m_edgeMaxError;
	m_cfg.minRegionArea = conf.minRegionArea; // (int)rcSqr(m_regionMinSize);		// Note: area = size*size
	m_cfg.mergeRegionArea = conf.mergeRegionArea; // (int)rcSqr(m_regionMergeSize);	// Note: area = size*size
	m_cfg.maxVertsPerPoly = conf.maxVertsPerPoly;
	m_cfg.detailSampleDist = conf.detailSampleDist; // m_detailSampleDist < 0.9f ? 0 : m_cellSize * m_detailSampleDist;
	m_cfg.detailSampleMaxError = conf.detailSampleMaxError; // m_cellHeight* m_detailSampleMaxError;


	// Set the area where the navigation will be build.
	// Here the bounds of the input mesh are used, but the
	// area could be specified by an user defined box, etc.
	rcVcopy(m_cfg.bmin, conf.bmin);
	rcVcopy(m_cfg.bmax, conf.bmax);

	rcCalcGridSize(m_cfg.bmin, m_cfg.bmax, m_cfg.cs, &m_cfg.width, &m_cfg.height);
}


HL_PRIM vbyte *HL_NAME(build_mesh)(vbyte *vertbytes, int vertCount, vbyte *triBytes, int triCount, recast_struct<hlRecastConfig> *conf, int &blobSize) {
	hlRecastConfig c = conf->value;

	float *verts = (float*)vertbytes;
	int   *tris  = (int*)triBytes;

	cleanup();
	setConfig(c);
	rcContext ctx;

	for (int i = 0; i < vertCount; i++) {
		std::cout << " x: " << verts[i * 3];
		std::cout << " y: " << verts[i * 3 + 1];
		std::cout << " z: " << verts[i * 3 + 2];
		std::cout << "\n";
	}

	m_solid = rcAllocHeightfield();
	if (!m_solid) {
		ctx.log(RC_LOG_ERROR, "buildNavigation: Out of memory 'solid'.");
		return NULL;
	}

	if (!rcCreateHeightfield(&ctx, *m_solid, m_cfg.width, m_cfg.height, m_cfg.bmin, m_cfg.bmax, m_cfg.cs, m_cfg.ch)) {
		ctx.log(RC_LOG_ERROR, "buildNavigation: Could not create solid heightfield.");
		return NULL;
	}

    // Allocate array that can hold triangle area types.
	// If you have multiple meshes you need to process, allocate
	// and array which can hold the max number of triangles you need to process.
	m_triareas = new unsigned char[triCount];
	if (!m_triareas)
	{
		ctx.log(RC_LOG_ERROR, "buildNavigation: Out of memory 'm_triareas' (%d).", triCount);
		return NULL;
	}
	
	// Find triangles which are walkable based on their slope and rasterize them.
	// If your input data is multiple meshes, you can transform them here, calculate
	// the are type for each of the meshes and rasterize them.
	memset(m_triareas, 0, triCount*sizeof(unsigned char));
	rcMarkWalkableTriangles(&ctx, m_cfg.walkableSlopeAngle, verts, vertCount, tris, triCount, m_triareas);
	if (!rcRasterizeTriangles(&ctx, verts, vertCount, tris, m_triareas, triCount, *m_solid, m_cfg.walkableClimb))
	{
		ctx.log(RC_LOG_ERROR, "buildNavigation: Could not rasterize triangles.");
		return NULL;
	}

	if (!m_keepInterResults)
	{
		delete [] m_triareas;
		m_triareas = 0;
	}
	
	//
	// Step 3. Filter walkables surfaces.
	//
	
	// Once all geoemtry is rasterized, we do initial pass of filtering to
	// remove unwanted overhangs caused by the conservative rasterization
	// as well as filter spans where the character cannot possibly stand.
	if (m_filterLowHangingObstacles)
		rcFilterLowHangingWalkableObstacles(&ctx, m_cfg.walkableClimb, *m_solid);
	if (m_filterLedgeSpans)
		rcFilterLedgeSpans(&ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid);
	if (m_filterWalkableLowHeightSpans)
		rcFilterWalkableLowHeightSpans(&ctx, m_cfg.walkableHeight, *m_solid);


	//
	// Step 4. Partition walkable surface to simple regions.
	//

	// Compact the heightfield so that it is faster to handle from now on.
	// This will result more cache coherent data as well as the neighbours
	// between walkable cells will be calculated.
	m_chf = rcAllocCompactHeightfield();
	if (!m_chf)
	{
		ctx.log(RC_LOG_ERROR, "buildNavigation: Out of memory 'chf'.");
		return NULL;
	}
	if (!rcBuildCompactHeightfield(&ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid, *m_chf))
	{
		ctx.log(RC_LOG_ERROR, "buildNavigation: Could not build compact data.");
		return NULL;
	}
	
	if (!m_keepInterResults)
	{
		rcFreeHeightField(m_solid);
		m_solid = 0;
	}
		
	// Erode the walkable area by agent radius.
	if (!rcErodeWalkableArea(&ctx, m_cfg.walkableRadius, *m_chf))
	{
		ctx.log(RC_LOG_ERROR, "buildNavigation: Could not erode.");
		return NULL;
	}

	/*
	// (Optional) Mark areas.
	const ConvexVolume* vols = m_geom->getConvexVolumes();
	for (int i  = 0; i < m_geom->getConvexVolumeCount(); ++i)
		rcMarkConvexPolyArea(&ctx, vols[i].verts, vols[i].nverts, vols[i].hmin, vols[i].hmax, (unsigned char)vols[i].area, *m_chf);
	*/
	
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
	
	if (m_partitionType == SAMPLE_PARTITION_WATERSHED)
	{
		// Prepare for region partitioning, by calculating distance field along the walkable surface.
		if (!rcBuildDistanceField(&ctx, *m_chf))
		{
			ctx.log(RC_LOG_ERROR, "buildNavigation: Could not build distance field.");
			return NULL;
		}
		
		// Partition the walkable surface into simple regions without holes.
		if (!rcBuildRegions(&ctx, *m_chf, 0, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
		{
			ctx.log(RC_LOG_ERROR, "buildNavigation: Could not build watershed regions.");
			return NULL;
		}
	}
	else if (m_partitionType == SAMPLE_PARTITION_MONOTONE)
	{
		// Partition the walkable surface into simple regions without holes.
		// Monotone partitioning does not need distancefield.
		if (!rcBuildRegionsMonotone(&ctx, *m_chf, 0, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
		{
			ctx.log(RC_LOG_ERROR, "buildNavigation: Could not build monotone regions.");
			return NULL;
		}
	}
	else // SAMPLE_PARTITION_LAYERS
	{
		// Partition the walkable surface into simple regions without holes.
		if (!rcBuildLayerRegions(&ctx, *m_chf, 0, m_cfg.minRegionArea))
		{
			ctx.log(RC_LOG_ERROR, "buildNavigation: Could not build layer regions.");
			return NULL;
		}
	}

	
	//
	// Step 5. Trace and simplify region contours.
	//
	
	// Create contours.
	m_cset = rcAllocContourSet();
	if (!m_cset)
	{
		ctx.log(RC_LOG_ERROR, "buildNavigation: Out of memory 'cset'.");
		return NULL;
	}
	if (!rcBuildContours(&ctx, *m_chf, m_cfg.maxSimplificationError, m_cfg.maxEdgeLen, *m_cset))
	{
		ctx.log(RC_LOG_ERROR, "buildNavigation: Could not create contours.");
		return NULL;
	}

	
	//
	// Step 6. Build polygons mesh from contours.
	//
	
	// Build polygon navmesh from the contours.
	m_pmesh = rcAllocPolyMesh();
	if (!m_pmesh)
	{
		ctx.log(RC_LOG_ERROR, "buildNavigation: Out of memory 'pmesh'.");
		return NULL;
	}
	if (!rcBuildPolyMesh(&ctx, *m_cset, m_cfg.maxVertsPerPoly, *m_pmesh))
	{
		ctx.log(RC_LOG_ERROR, "buildNavigation: Could not triangulate contours.");
		return NULL;
	}

	
	//
	// Step 7. Create detail mesh which allows to access approximate height on each polygon.
	//
	
	m_dmesh = rcAllocPolyMeshDetail();
	if (!m_dmesh)
	{
		ctx.log(RC_LOG_ERROR, "buildNavigation: Out of memory 'pmdtl'.");
		return NULL;
	}

	if (!rcBuildPolyMeshDetail(&ctx, *m_pmesh, *m_chf, m_cfg.detailSampleDist, m_cfg.detailSampleMaxError, *m_dmesh))
	{
		ctx.log(RC_LOG_ERROR, "buildNavigation: Could not build detail mesh.");
		return NULL;
	}


	if (!m_keepInterResults)
	{
		rcFreeCompactHeightfield(m_chf);
		m_chf = 0;
		rcFreeContourSet(m_cset);
		m_cset = 0;
	}

	//std::cout << "Generated mesh with " << m_pmesh->nverts << " verts. \n";
	for (int i = 0; i < m_pmesh->npolys; i++) {
		m_pmesh->polys[i];
	}


	int total_size = sizeof(*m_pmesh);

	size_t verts_size = sizeof(short) * 3 * m_pmesh->nverts;
	size_t polys_size = sizeof(short) * 2 * m_pmesh->nvp * m_pmesh->maxpolys;
	size_t regs_size  = sizeof(short) * 1 * m_pmesh->maxpolys;
	size_t flags_size = sizeof(short) * 1 * m_pmesh->maxpolys;
	size_t areas_size = sizeof(char)  * 1 * m_pmesh->maxpolys;

	size_t pointers_size = sizeof(m_pmesh->verts) + sizeof(m_pmesh->polys) + sizeof(m_pmesh->regs) + sizeof(m_pmesh->flags) + sizeof(m_pmesh->areas);

	// Remove pointers from this
	total_size -= pointers_size;

	size_t struct_size = total_size;

	total_size += verts_size + polys_size + regs_size + flags_size + areas_size;

	vbyte* result = hl_alloc_bytes(total_size);
	size_t p = 0;

	void* s = m_pmesh;

	// Copy static data (non pointer values, starting from nverts)
	memcpy(result, &m_pmesh->nverts, struct_size);
	p += struct_size;


	// Copy dynamic values into buffer
	memcpy(result + p, m_pmesh->verts, verts_size);
	p += verts_size;

	memcpy(result + p, m_pmesh->polys, polys_size);
	p += polys_size;

	memcpy(result + p, m_pmesh->regs, regs_size);
	p += regs_size;

	memcpy(result + p, m_pmesh->flags, flags_size);
	p += flags_size;

	memcpy(result + p, m_pmesh->areas, areas_size);
	p += areas_size;

	bool e = p == total_size;


	//std::cout << e << "\n";
	blobSize = total_size;

	rcFreePolyMesh(m_pmesh);
	m_pmesh = 0;
	rcFreePolyMeshDetail(m_dmesh);
	m_dmesh = 0;

	return result;
}

DEFINE_PRIM(_BYTES, build_mesh, _BYTES _I32 _BYTES _I32 _DYN _REF(_I32));



NavMesh::NavMesh() {

}

NavMesh::~NavMesh() {
}

void NavMesh::destroy() {

}

float NavMesh::test(float positions[], int len)
{
	float res = 0.f;
	for (int i = 0; i < len; i++) {
		res += positions[i];
	}

	return res;
}


bool NavMesh::build(const float* positions, const int positionCount, const int* indices, const int indexCount, const rcConfig& config)
{
	return false;
}

Vec3 NavMesh::getClosestPoint(const Vec3& point) {
	Vec3 v;
	v.x = 100;
	v.y = 312;
	v.z = 34.f;
	return v;
}


