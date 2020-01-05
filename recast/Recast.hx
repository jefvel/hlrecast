package recast;

class Recast {
    public static function buildMesh(mesh : RecastMesh, conf : RecastConfig) : Bool {
        return _BuildMesh(
            mesh.getVertBytes(), 
            mesh.vertCount,
            mesh.getTriangleBytes(),
            mesh.triangleCount,
            conf
        );
    }

    @:hlNative("recast", "build_mesh")
    private static function _BuildMesh(verts : hl.Bytes, vertCount : Int, tris : hl.Bytes, triCount : Int, conf : Dynamic) : Bool { return false; }
}