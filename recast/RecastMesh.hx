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

    inline function addPoint(p) {
        addVertex(p.x, p.y, p.z);
    }

    // Adds simplified object mesh to navmesh
    public function addMesh(mesh : h3d.scene.Mesh) {
        var pos = new h3d.col.Point(mesh.x, mesh.y, mesh.z);
        var sx = mesh.scaleX;
        var sy = mesh.scaleY;
        var sz = mesh.scaleZ;

        var q = mesh.getRotationQuat().toMatrix();
        var b = mesh.primitive.getBounds();

        var s = vertCount;

        var p0 = new h3d.col.Point(b.xMin * sx, b.yMax * sy, b.zMin * sz);
        var p1 = new h3d.col.Point(b.xMin * sx, b.yMin * sy, b.zMin * sz);
        var p2 = new h3d.col.Point(b.xMin * sx, b.yMax * sy, b.zMax * sz);
        var p3 = new h3d.col.Point(b.xMin * sx, b.yMin * sy, b.zMax * sz);

        var p4 = new h3d.col.Point(b.xMax * sx, b.yMax * sy, b.zMin * sz);
        var p5 = new h3d.col.Point(b.xMax * sx, b.yMax * sy, b.zMax * sz);

        var p6 = new h3d.col.Point(b.xMax * sx, b.yMin * sy, b.zMax * sz);
        var p7 = new h3d.col.Point(b.xMax * sx, b.yMin * sy, b.zMin * sz);

        p0.transform3x3(q); p0 = p0.add(pos);
        p1.transform3x3(q); p1 = p1.add(pos);
        p2.transform3x3(q); p2 = p2.add(pos);
        p3.transform3x3(q); p3 = p3.add(pos);

        p4.transform3x3(q); p4 = p4.add(pos);
        p5.transform3x3(q); p5 = p5.add(pos);

        p6.transform3x3(q); p6 = p6.add(pos);
        p7.transform3x3(q); p7 = p7.add(pos);

        addPoint(p0);
        addPoint(p1);
        addPoint(p2);
        addPoint(p3);

        addPoint(p4);
        addPoint(p5);

        addPoint(p6);
        addPoint(p7);
        
        // Left side
        addTriangle(s + 0, s + 1, s + 2);
        addTriangle(s + 2, s + 3, s + 1);

        // Front
        addTriangle(s + 0, s + 2, s + 5);
        addTriangle(s + 4, s + 2, s + 5);

        // Right
        addTriangle(s + 7, s + 6, s + 5);
        addTriangle(s + 7, s + 5, s + 4);

        // Back
        addTriangle(s + 6, s + 1, s + 3);
        addTriangle(s + 6, s + 7, s + 1);

        // Top
        addTriangle(s + 3, s + 5, s + 6);
        addTriangle(s + 3, s + 2, s + 5);
    }

    public function getVertBytes() : haxe.io.Bytes {
        return Float32Array.fromArray(verts).getData().bytes;
    }

    public function getTriangleBytes() : haxe.io.Bytes {
        return Int32Array.fromArray(tris).getData().bytes;
    }
}