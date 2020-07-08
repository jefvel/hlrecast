import h3d.scene.Mesh;
import recast.RecastConfig;
import recast.RecastMesh;
import recast.Recast;

class Test extends hxd.App {
    static function main() {
        new Test();
    }

    override function init() {
        super.init();

        var startTime = haxe.Timer.stamp();
        var mesh = new RecastMesh();
        var conf = new RecastConfig();

        /*
        mesh.addVertex(0, 0, 0);
        mesh.addVertex(0, 100, 0);
        mesh.addVertex(100, 100, 0);
        mesh.addVertex(100, 0, 0);

        mesh.addTriangle(0, 1, 2);
        mesh.addTriangle(3, 0, 2);

        // Add vertical square
        var s = mesh.vertCount;
        var size = 50;
        mesh.addVertex(20, 0, size);
        mesh.addVertex(20, size, 0);
        mesh.addVertex(20, size, size);
        mesh.addVertex(20, 0, 0);
        */

        //mesh.addTriangle(s + 0, s + 1, s + 2);
        //mesh.addTriangle(s + 0, s + 3, s + 1);


        mesh.bounds.addPos(0, 0, 10);
        var prim = new h3d.prim.Cube();

        // translate it so its center will be at the center of the cube
        prim.translate( -0.5, -0.5, -0.5);

        // unindex the faces to create hard edges normals
        prim.unindex();

        // add face normals
        prim.addNormals();

        // add texture coordinates
        prim.addUVs();

        var box = new Mesh(prim, s3d);
        box.x = 20;
        box.z = 1;
        box.y = 20;
        box.scaleY = 5;
        box.scaleZ = 5;
        box.scaleX = 5;
        box.rotate(0, 0, Math.PI * 0.25);
        mesh.addBoundingBox(box);

        /*
        var stairsUp = new Mesh(prim, s3d);
        stairsUp.x = 11.5;
        stairsUp.z = 0.0;
        stairsUp.y = 11.5;
        stairsUp.scaleY = 2.0;
        stairsUp.scaleX = 20;
        stairsUp.rotate(0, Math.PI * -0.1, Math.PI * 0.25);
        mesh.addBoundingBox(stairsUp);
        */

        var e = new h3d.prim.Cylinder(12);
        var cyl = new h3d.scene.Mesh(e, s3d);
        cyl.scaleZ = 20;
        cyl.scaleX = cyl.scaleY = 2.0;
        cyl.x = 30;
        cyl.y = 35;
        mesh.addBoundingBox(cyl);

        /*
        var b = new Mesh(prim, s3d);
        b.scaleX = 0.1;
        b.x = 20;
        b.scaleY = 50;
        b.scaleZ = 5;
        b.y = b.scaleY * 0.5;
        b.material.color.setColor(0x444444);
        */

        var ground = new Mesh(prim, s3d);
        ground.material.color.setColor(0xdedede);
        ground.scaleX = ground.scaleY = 100;
        ground.x = ground.y = 50;
        mesh.addBoundingBox(ground);
        
        conf.setBounds(mesh.bounds);

        var navMesh = Recast.buildMesh(mesh, conf);

        navMesh.primitive.addNormals();
        var obj = new h3d.scene.Mesh(navMesh.primitive, s3d);
        obj.material.shadows = false;
        obj.material.color.setColor(0xFF0000);
        obj.material.mainPass.wireframe = true;


        // adds a directional light to the scene
        var light = new h3d.scene.fwd.DirLight(new h3d.Vector(0.5, 0.5, -0.5), s3d);
        light.enableSpecular = true;

        // set the ambient light to 30%
        s3d.lightSystem.ambientLight.set(0.3, 0.3, 0.3);

        new h3d.scene.CameraController(s3d).loadFromCamera();

        trace('total generation ${haxe.Timer.stamp() - startTime} seconds.');
    }
}