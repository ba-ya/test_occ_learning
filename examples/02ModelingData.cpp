#include "00headers.h"
#include <gce_MakeCirc.hxx>
#include <gce_MakeLin2d.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS.hxx>
#include <TopExp_Explorer.hxx>
#include <TopExp.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <TopoDS_Face.hxx>
void ModelingData::Draw(vtkRenderer *renderer)
{
    // make circle
    {
        gp_Pnt p1(0, 0, 0);
        gp_Pnt p2(0, 10, 0);
        gp_Pnt p3(10, 0, 0);
        gce_MakeCirc mc(p1, p2, p3);
        if (mc.IsDone()) {
            const gp_Circ c = mc.Value();
        }
    }

    // gce还原gp
    {
        gp_Pnt2d p1(0, 0);
        gp_Pnt2d p2(2, 0);
        gce_MakeLin2d L = gce_MakeLin2d(p1, p2);
        if (L.Status() == gce_Done) {
            gp_Lin2d l = L.Value();
        }
    }

    // TopAbs_Orientation
    // TopAbs_State
    {
        TopAbs_Orientation ori;
        ori = TopAbs_FORWARD;
        ori = TopAbs_REVERSED;
        ori = TopAbs_INTERNAL;
        ori = TopAbs_EXTERNAL;

        TopAbs_State state;
        state = TopAbs_IN;
        state = TopAbs_ON;
        state = TopAbs_OUT;
        state = TopAbs_UNKNOWN;
    }

    // TopExp
    {
        TopoDS_Shape shape;
        for (TopExp_Explorer ex(shape, TopAbs_FACE); ex.More(); ex.Next()) {
            // ...
            ex.Current();
        }
        // find all the vertices which are not in an edge
        // shape, tofind, toavoid
        for (TopExp_Explorer ex(shape, TopAbs_VERTEX, TopAbs_EDGE); ex.More(); ex.Next()) {
            //...
            ex.Current();
        }

        // find all the faces in a shell,
        TopExp_Explorer ex1, ex2;
        for (ex1.Init(shape, TopAbs_SHELL); ex1.More(); ex1.Next()) {
            for (ex2.Init(ex1.Current(), TopAbs_FACE); ex2.More(); ex2.Next()) {
                ex2.Current();
            }
        }
        //all the faces not in a shell
        for (ex1.Init(shape, TopAbs_FACE, TopAbs_SHELL); ex1.More(); ex1.Next()) {
            ex1.Current();
        }

        // TopTools_IndexedMapOfShape
        const TopAbs_ShapeEnum type = TopAbs_EDGE;
        TopExp_Explorer ex3(shape, type);
        TopTools_IndexedMapOfShape map;
        while(ex3.More()) {
            map.Add(ex3.Current());
            ex3.Next();
        }

        // example, DrawShape
        // edges on the face or not
        {
            TopTools_IndexedMapOfShape edge_map;
            TopExp::MapShapes(shape, TopAbs_EDGE, edge_map);

            TColStd_Array1OfInteger face_count(1, edge_map.Extent());
            face_count.Init(0);
            TopExp_Explorer exp_face(shape, TopAbs_FACE);
            for (;exp_face.More(); exp_face.Next()) {
                TopoDS_Face face = TopoDS::Face(exp_face.Current());

                TopExp_Explorer exp_edge(face, TopAbs_EDGE);
                for (; exp_edge.More(); exp_edge.Next()) {
                    auto edge_id = edge_map.FindIndex(exp_edge.Current());
                    face_count[edge_id] = 1;
                }
            }
        }
    }
}
// TopoDS_Shape
void process_edge(const TopoDS_Edge &the_edge) {}
void process (const TopoDS_Shape &the_shape) {
    if (the_shape.ShapeType() == TopAbs_VERTEX) {
        TopoDS_Vertex v;
        v = TopoDS::Vertex(the_shape);
        // TopoDS_Vertex v2 = the_shape; // wrong
        TopoDS_Shape v3 = TopoDS::Vertex(the_shape);
    } else if (the_shape.ShapeType() == TopAbs_EDGE) {
        // process_edge(the_shape); // wrong
        process_edge(TopoDS::Edge(the_shape));
    } else {
        // process_edge(TopoDS::Edge(the_shape)); // 编译通过,实际运行会报错
    }
}
