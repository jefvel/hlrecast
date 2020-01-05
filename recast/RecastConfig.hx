package recast;

import h3d.scene.fwd.PointLight;
import hl.F32;


typedef PartitionType = Int;

class PartitionTypes {
    public static final SamplePartitionWatershed : PartitionType = 0;
    public static final SamplePartitionMonotone : PartitionType = 1;
    public static final SamplePartitionLayers : PartitionType = 2;
}

class RecastConfig {
    // Navmesh generation params
    public var width : Int = 100;
    public var height : Int = 40;
    public var tileSize : Int = 1;
    public var borderSize : Int = 0;

    public var cs : F32 = 0.6;
    public var ch : F32 = 0.6;

    public var bminX : F32 = 0.0;
    public var bminY : F32 = 0.0;
    public var bminZ : F32 = 0.0;

    public var bmaxX : F32 = 100.0;
    public var bmaxY : F32 = 100.0;
    public var bmaxZ : F32 = 100.0;

    public var walkableSlopeAngle : F32 = 25;

    public var walkableHeight : Int = 5;
    public var walkableClimb : Int = 1;
    public var walkableRadius : F32 = 0.;

    public var maxEdgeLen : Int;
    public var maxSimplificationError : F32;
    public var minRegionArea : Int = 1;
    public var mergeRegionArea : Int = 0;
    public var maxVertsPerPoly : Int = 8;
    public var detailSampleDist : F32 = 0.98;
    public var detailSampleMaxError : F32 = 0.1;

    // More config stuff
    public var partitionType : PartitionType = PartitionTypes.SamplePartitionWatershed;


    // Detour Config


    public function new() {}

    public function setBounds( bounds : h3d.col.Bounds ) {
        bminX = bounds.xMin;
        bminY = bounds.zMin;
        bminZ = bounds.yMin;

        bmaxX = bounds.xMax;
        bmaxY = bounds.zMax;
        bmaxZ = bounds.yMax;
    }
}