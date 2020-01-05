import recast.RecastConfig;
import recast.RecastMesh;
import recast.Recast;

class Test {
    static function main() {
        var mesh = new RecastMesh();
        var conf = new RecastConfig();

        mesh.addVertex(0, 0, 0);
        mesh.addVertex(0, 10, 0);
        mesh.addVertex(10, 10, 0);
        mesh.addVertex(10, 0, 0);

        mesh.addTriangle(0, 1, 2);
        mesh.addTriangle(3, 0, 2);

        mesh.bounds.addPos(0, 0, 10);

        conf.setBounds(mesh.bounds);

        Recast.buildMesh(mesh, conf);
    }
}