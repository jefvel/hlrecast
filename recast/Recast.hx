package recast;

import hl.Ref;

class Recast {
    public static function buildMesh(mesh : RecastMesh, conf : RecastConfig) : PolyMesh {
        var b = 0;

        var bytes = _BuildMesh(
            mesh.getVertBytes(), 
            mesh.vertCount,
            mesh.getTriangleBytes(),
            mesh.triangleCount,
            conf,
            b
        );

        if (bytes == null) {
            return null;
        }

        var mesh = PolyMesh.fromBytes(bytes.toBytes(b));

        return mesh;
    }

    @:hlNative("recast", "build_mesh")
    private static function _BuildMesh(verts : hl.Bytes, vertCount : Int, tris : hl.Bytes, triCount : Int, conf : Dynamic, blobSize : Ref<Int>) : hl.Bytes { return null; }
}