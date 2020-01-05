package recast;


import haxe.io.Int32Array;
import haxe.io.Float32Array;
import hxd.FloatBuffer;

// Recastmesh. Z is up.
class RecastMesh {
    var verts : Array<Float>;
    var tris : Array<Int>;

    public var vertCount (default, null): Int;
    public var triangleCount (default, null) : Int;

    public var bounds : h3d.col.Bounds;

    public function new() {
        verts = [];
        tris = [];
        vertCount = triangleCount = 0;
        bounds = new h3d.col.Bounds();
    }

    public inline function addVertex(x : Float, y : Float, z : Float) {
        vertCount ++;
        verts.push(x);
        verts.push(z);
        verts.push(y);

        bounds.addPos(x, y, z);
    }

    public inline function addTriangle(a : Int, b : Int, c : Int) {
        triangleCount ++;
        tris.push(a);
        tris.push(b);
        tris.push(c);
    }

    public function getVertBytes() : haxe.io.Bytes {
        return Float32Array.fromArray(verts).getData().bytes;
    }

    public function getTriangleBytes() : haxe.io.Bytes {
        return Int32Array.fromArray(tris).getData().bytes;
    }
}