package recast;

import haxe.io.UInt16Array;

class PolyMesh {
    static final RC_MESH_NULL_IDX : hl.UI16 = 0xffff;

    public var polyCount : Int;
    public var vertCount : Int;
    public var maxPolys : Int;
    public var vertsPerPoly : Int;

    public var bounds : h3d.col.Bounds;

    public var cs : Float;
    public var ch : Float;

    public var borderSize : Int;

    public var maxEdgeError : Float;


    function new() {
        bounds = new h3d.col.Bounds();
    }

    public var primitive : h3d.prim.Polygon;
    public function toWireframe() {

    }

    public static function fromBytes(bytes : haxe.io.Bytes) {
        var bs = new haxe.io.BytesInput(bytes);
        var res = new PolyMesh();

        res.vertCount = bs.readInt32();
        res.polyCount = bs.readInt32();
        res.maxPolys = bs.readInt32();
        res.vertsPerPoly = bs.readInt32();
        
        // Flip z and y in bounds
        res.bounds.xMin = bs.readFloat();
        res.bounds.zMin = bs.readFloat();
        res.bounds.yMin = bs.readFloat();

        res.bounds.xMax = bs.readFloat();
        res.bounds.zMax = bs.readFloat();
        res.bounds.yMax = bs.readFloat();

        res.cs = bs.readFloat();
        res.ch = bs.readFloat();
        res.borderSize = bs.readInt32();
        res.maxEdgeError = bs.readFloat();

        var vertCount = 3 * res.vertCount;


        var vs = new Array<Float>();

        var points = new Array<h3d.col.Point>();

        // Flip y and z
        for (i in 0...res.vertCount) {
            var x = res.cs * bs.readUInt16() + res.bounds.xMin;
            var z = res.ch * bs.readUInt16() + res.bounds.zMin;
            var y = res.cs * bs.readUInt16() + res.bounds.yMin;

            vs.push(x);
            vs.push(y);
            vs.push(z);
            points.push(new h3d.col.Point(x, y, z + res.ch * 0.1));
        }

        var sizeOfShort = 2;
        var polys = UInt16Array.fromBytes(bs.read(sizeOfShort * 2 * res.maxPolys * res.vertsPerPoly));
        for (i in 0...polys.length) {
            //Sys.print(polys[i]+ ", ");
        }
        var vi = [];
        var indexes = [];
        var indexBuffer = new hxd.IndexBuffer();
        for (i in 0...res.polyCount) {
            var p = res.vertsPerPoly * i * 2;

            for (j in 2...res.vertsPerPoly) {
                if (polys[p + j] == RC_MESH_NULL_IDX) break;
                vi[0] = polys[p];
                vi[1] = polys[p + j - 1];
                vi[2] = polys[p + j];

                for (k in 0...3) {
                    var index = vi[k];
                    indexBuffer.push(index);
                    //trace(index);
                }
                //trace("---");
            }
        }

        res.primitive = new h3d.prim.Polygon(points, indexBuffer);

        return res;
    }
}