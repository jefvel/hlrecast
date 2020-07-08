#define HL_NAME(n) recast_##n
#include <hl.h>
#include <Recast.h>


struct Vec3 {
	float x;
	float y;
	float z;
};

class NavMesh {
public:
    NavMesh();
    ~NavMesh();
    void destroy();

	float test(float positions[], int len);

    bool build(const float *positions, const int positionCount, const int *indices, const int indexCount, const rcConfig &config);
    Vec3 getClosestPoint(const Vec3 &position);
    //Vec3 getRandomPointAround(const Vec3 &position, float maxRadius);
    //Vec3 moveAlong(const Vec3 &position, const Vec3 &destination);
    //dtNavMesh getNavMesh();
    //NavPath computePath(const Vec3 &start, const Vec3 &end);
    //void setDefaultQueryExtent(const Vec3 &extent);
    //Vec3 getDefaultQueryExtent();
};
