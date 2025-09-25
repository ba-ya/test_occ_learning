#include "00headers.h"
#include <GeomAPI_IntCS.hxx>
#include <GeomAPI_IntSS.hxx>
#include <Geom_Line.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_Circle.hxx>
#include <TopoDS_Edge.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <GccAna_Lin2d2Tan.hxx>
#include <GccEnt.hxx>
#include <gp_Circ2d.hxx>
#include <gp_Ax2d.hxx>
#include <GccEnt_QualifiedCirc.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <GeomPlate_BuildPlateSurface.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <TopoDS.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepFill_CurveConstraint.hxx>
#include <GeomPlate_MakeApprox.hxx>
#include <Geom_BSplineSurface.hxx>
#include <GeomPlate_Surface.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <vtkNamedColors.h>
#include <TopTools_Array1OfShape.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <TopoDS_Solid.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeTorus.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <TopExp_Explorer.hxx>
#include <BRepFilletAPI_MakeFillet2d.hxx>
#include <TopoDS_Face.hxx>
#include <BRepTools.hxx>
#include <Geom_Surface.hxx>
#include <Geom_Plane.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <Geom2d_Circle.hxx>
#include <BRepLib.hxx>
#include <BRepFeat_MakeDPrism.hxx>
#include <vtkProperty.h>
#include <GCE2d_MakeLine.hxx>
#include <Geom2d_Line.hxx>
#include <Geom_BezierCurve.hxx>
#include <BRepFeat_MakePipe.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepFeat_MakeLinearForm.hxx>
void ModelingAlgorithms::Draw(std::vector<vtkSmartPointer<vtkRenderer>> renders)
{
    int r_id = 0;
    // Geom_Curve和Geom_Surface是抽象基类, 不能直接使用
    Handle(Geom_Curve) curve = new Geom_Line(gp_Pnt(-10, 0, 0), gp_Dir(1, 0, 0));
    Handle(Geom_Surface) surface = new Geom_CylindricalSurface(gp_Ax3(gp_Pnt(0, 0, 0), gp::DZ()), 5.0);

    Standard_Real tolerance = 1e-6;// 1um
    // 曲线和面的交点
    {
        GeomAPI_IntCS intersector(curve, surface);
        if (intersector.IsDone()) {
            intersector.NbPoints();
        }
    }

    // 面和面的交线
    {
        Handle(Geom_Surface) surface_2 = new Geom_CylindricalSurface(gp_Ax3(gp_Pnt(0, 0, 0), gp::DX()), 5.0);
        GeomAPI_IntSS intersector(surface, surface_2, tolerance);
        if (intersector.IsDone()) {
            intersector.NbLines();
            // Handle(Geom_Curve) c1 = intersector.Line(1);// 1-nbLines
        }
    }

    //Exterior/Interior
    {
        // gp_Ax2(p_orig, z, x)
        gp_Circ2d c1(gp_Ax2d(gp_Pnt2d(0, 0), gp_Dir2d(1, 0)), 4);
        gp_Circ2d c2(gp_Ax2d(gp_Pnt2d(10, 0), gp_Dir2d(1, 0)), 4);
        GccAna_Lin2d2Tan solver(GccEnt::Outside(c1), GccEnt::Outside(c2), tolerance);
    }

    // r_id = 0
    //创建一个板曲面，并将其从多段线近似为曲线约束和点约束
    {
        Standard_Integer cnt_cur_front = 4;
        Standard_Integer cnt_point_constraint = 1;
        gp_Pnt p1(0, 0, 0);
        gp_Pnt p2(0, 10, 0);
        gp_Pnt p3(0, 10, 10);
        gp_Pnt p4(0, 0, 10);
        gp_Pnt p5(5, 5, 5);
        BRepBuilderAPI_MakePolygon polygon;
        polygon.Add(p1);
        polygon.Add(p2);
        polygon.Add(p3);
        polygon.Add(p4);
        polygon.Add(p5);
        polygon.Close();

        GeomPlate_BuildPlateSurface bp_surf(3, 15, 2);
        BRepTools_WireExplorer wire_exp;
        for (wire_exp.Init(polygon.Wire()); wire_exp.More(); wire_exp.Next()) {
            TopoDS_Edge edge = wire_exp.Current();
            Handle(BRepAdaptor_Curve) curve = new BRepAdaptor_Curve(edge);
            Handle(BRepFill_CurveConstraint) cont = new BRepFill_CurveConstraint(curve, 0);
            bp_surf.Add(cont);
        }
        Handle(GeomPlate_PointConstraint) p_cont = new GeomPlate_PointConstraint(p5, 0);
        bp_surf.Add(p_cont);
        bp_surf.Perform();

        Standard_Integer max_seg = 9;
        Standard_Integer max_degree = 8;
        Standard_Integer crit_order = 0;
        Standard_Real dmax, tol = 1e-4;
        Handle(GeomPlate_Surface) p_surf = bp_surf.Surface();
        dmax = Max(tol, 10 * bp_surf.G0Error());
        GeomPlate_MakeApprox make_approx_plate(p_surf, tol, max_seg, max_degree, dmax, crit_order);
        Handle(Geom_Surface) surf(make_approx_plate.Surface());

        Standard_Real umin, umax, vmin, vmax;
        p_surf->Bounds(umin, umax, vmin, vmax);
        BRepBuilderAPI_MakeFace mf(surf, umin, umax, vmin, vmax, tol);

        auto render = renders[r_id++];
        auto shape = mf.Shape();
        render->AddActor(Helper::shape_to_actor(shape));
        auto shape2 = polygon.Shape();
        auto actor2 = Helper::shape_to_actor(shape2);
        auto colors = vtkSmartPointer<vtkNamedColors>::New();
        actor2->GetProperty()->SetColor(colors->GetColor3d("Red").GetData());
        render->AddActor(actor2);
    }

    Standard_Real PI = 3.1415926535;
    // r_id = 1
    //create a wire, 圆角矩形框,
    {
        auto &&make_arc = [PI](Standard_Real x,
                                Standard_Real y,
                                Standard_Real r,
                                Standard_Real angle,
                                TopoDS_Shape &edge,
                                TopoDS_Shape &v1,
                                TopoDS_Shape &v2) {
            gp_Ax2 origin = gp::XOY();
            gp_Vec offset(x, y, 0);
            origin.Translate(offset);

            // x轴是0度方向
            // +angle是逆时针旋转angle角度
            BRepBuilderAPI_MakeEdge make_edge(gp_Circ(origin, r), angle, angle + PI / 2);
            edge = make_edge;
            v1 = make_edge.Vertex1();
            v2 = make_edge.Vertex2();
        };
        auto render = renders[r_id++];
        TopTools_Array1OfShape edges(1, 8);
        TopTools_Array1OfShape vertexs(1, 8);

        // round edge
        Standard_Real length = 10, height = 6, radius = .5;
        Standard_Real x = length / 2 - radius, y = height / 2 - radius;
        make_arc( x, -y, radius, 3 * PI / 2, edges[2], vertexs[2], vertexs[3]);
        make_arc( x,  y, radius, 0         , edges[4], vertexs[4], vertexs[5]);
        make_arc(-x,  y, radius, PI / 2    , edges[6], vertexs[6], vertexs[7]);
        make_arc(-x, -y, radius, PI        , edges[8], vertexs[8], vertexs[1]);
        // linear edge
        for (Standard_Integer i = 1; i <= 7; i += 2) {
            edges[i] = BRepBuilderAPI_MakeEdge(TopoDS::Vertex(vertexs[i]), TopoDS::Vertex(vertexs[i+1]));
        }
        BRepBuilderAPI_MakeWire make_wire;
        for (Standard_Integer i = 1; i <= 8; ++i) {
            make_wire.Add(TopoDS::Edge(edges[i]));
        }
        auto wire = make_wire.Shape();
        render->AddActor(Helper::shape_to_actor(wire));
    }

    // r_id = 2
    // cylinder
    {
        Standard_Real x = 20, y = 10, z = 15, r = 10, dy = 30;
        gp_Ax2 axes = gp::XOY();
        axes.Translate(gp_Vec(x, y, z));
        TopoDS_Face face = BRepPrimAPI_MakeCylinder(axes, r, dy, PI / 2);
        TopoDS_Solid soild = BRepPrimAPI_MakeCylinder(axes, r, dy, PI / 2);

        auto render = renders[r_id++];
        auto actor = Helper::shape_to_actor(soild);
        render->AddActor(actor);
    }

    // r_id = 3
    // cone
    {
        Standard_Real r1 = 30, r2 = 20, h = 15;
        TopoDS_Face face = BRepPrimAPI_MakeCone(r1, r2, h);
        TopoDS_Solid soild = BRepPrimAPI_MakeCone(r1, r2, h);

        auto render = renders[r_id++];
        auto actor = Helper::shape_to_actor(face);
        render->AddActor(actor);
    }

    // r_id = 4, 5, 6, 7
    // sphere
    {
        Standard_Real r = 30, angle = 3 * PI / 2, a1 = - PI / 6, a2 = PI / 3;
        TopoDS_Solid s1 = BRepPrimAPI_MakeSphere(r);
        TopoDS_Face s2 = BRepPrimAPI_MakeSphere(r, angle);
        TopoDS_Solid s3 = BRepPrimAPI_MakeSphere(r, a1, a2);
        TopoDS_Face s4 = BRepPrimAPI_MakeSphere(r, a1, a2, angle);
        TopTools_Array1OfShape shapes(1, 4);
        shapes[1] = s1;
        shapes[2] = s2;
        shapes[3] = s3;
        shapes[4] = s4;
        for (int i = 1; i <= shapes.Size(); ++i) {
            auto render = renders[r_id++];
            auto actor = Helper::shape_to_actor(shapes[i]);
            render->AddActor(actor);
        }
    }

    // r_id = 8, 9, 10, 11
    // torus
    {
        Standard_Real r1 = 40, r2 = 10, angle = PI, a1 =  0, a2 = PI ;
        TopoDS_Shell s1 = BRepPrimAPI_MakeTorus(r1, r2);
        TopoDS_Shell s2 = BRepPrimAPI_MakeTorus(r1, r2, angle);
        TopoDS_Shell s3 = BRepPrimAPI_MakeTorus(r1, r2, a1, a2);
        TopoDS_Solid s4 = BRepPrimAPI_MakeTorus(r1, r2, a1, a2, angle);
        TopTools_Array1OfShape shapes(1, 4);
        shapes[1] = s1;
        shapes[2] = s2;
        shapes[3] = s3;
        shapes[4] = s4;
        for (int i = 1 ; i <= shapes.Size(); ++i) {
            auto render = renders[r_id++];
            auto actor = Helper::shape_to_actor(shapes[i]);
            render->AddActor(actor);
        }
    }

    // r_id = 12
    // fillet
    {
        Standard_Real a = 10, b = 10, c = 10, r = .5;
        TopoDS_Solid box = BRepPrimAPI_MakeBox(a, b, c);
        BRepFilletAPI_MakeFillet make_fillet(box);
        TopExp_Explorer exp(box, TopAbs_EDGE);
        for (; exp.More(); exp.Next()) {
            make_fillet.Add(r, TopoDS::Edge(exp.Current()));
        }
        auto render = renders[r_id++];
        auto shape = make_fillet.Shape();
        auto actor = Helper::shape_to_actor(shape);
        render->AddActor(actor);
    }

#if 0
    // r_id = 13
    // plane fillet
    // Fail,
    {
        Standard_Real a = 10, b = 10, c = 10, r = .05;
        TopoDS_Solid box = BRepPrimAPI_MakeBox(a, b, c);
        // pick face
        TopExp_Explorer ex1(box, TopAbs_FACE);
        const TopoDS_Face &face = TopoDS::Face(ex1.Current());
        // outwire
        TopoDS_Wire outer_wire = BRepTools::OuterWire(face);
        // mk
        BRepFilletAPI_MakeFillet2d make_fillet(face);
        TopExp_Explorer ex2(outer_wire, TopAbs_VERTEX);
        for (; ex2.More(); ex2.Next()) {
            TopoDS_Vertex v = TopoDS::Vertex(ex2.Current());
            make_fillet.AddFillet(v, r);
        }
        qDebug() << make_fillet.Status();
        // 7, ChFi2d_ConnexionError,
        // the vertex given to locate the fillet or the chamfer is not connected !< to 2 edges
        auto render = renders[r_id++];
        // auto shape = make_fillet.Shape();
        auto actor = Helper::shape_to_actor(outer_wire);
        render->AddActor(actor);
    }
#endif

    // r_id = 13
    // sew a box
    {
        Standard_Real a = 10, b = 10, c = 10, r = .05;
        TopoDS_Solid box = BRepPrimAPI_MakeBox(a, b, c);
        TopExp_Explorer ex1(box, TopAbs_FACE);
        BRepBuilderAPI_Sewing sew;
        for (; ex1.More(); ex1.Next()) {
            const TopoDS_Face &face = TopoDS::Face(ex1.Current());
            sew.Add(face);
        }
        sew.Perform();
        auto render = renders[r_id++];
        auto shape = sew.SewedShape();
        auto actor = Helper::shape_to_actor(shape);
        render->AddActor(actor);
    }

    // r_id = 14
    // Draft Prism
    {
        TopoDS_Shape box = BRepPrimAPI_MakeBox(400, 250, 300);
        TopExp_Explorer exp;
        exp.Init(box, TopAbs_FACE);
        exp.Next();
        exp.Next();
        exp.Next();
        exp.Next();
        exp.Next();
        TopoDS_Face face = TopoDS::Face(exp.Current());
        Handle(Geom_Surface) surf = BRep_Tool::Surface(face);
        gp_Circ2d circ(gp_Ax2d(gp_Pnt2d(200, 125), gp_Dir2d(1, 0)), 50.);

        BRepBuilderAPI_MakeWire make_wire;
        Handle(Geom2d_Curve) aline = new Geom2d_Circle(circ);
        TopoDS_Edge e1 = BRepBuilderAPI_MakeEdge(aline, surf, 0, PI);
        TopoDS_Edge e2 = BRepBuilderAPI_MakeEdge(aline, surf, PI, 2 * PI);
        make_wire.Add(e1);
        make_wire.Add(e2);

        BRepBuilderAPI_MakeFace make_face;
        make_face.Init(surf, Standard_False, 1e-6);
        make_face.Add(make_wire.Wire());
        make_face.Build();
        TopoDS_Face result_face = make_face.Face();
        BRepLib::BuildCurves3d(result_face);
        BRepFeat_MakeDPrism make_prism(box,// 基体
                                       result_face,// 基准面, 开始拔模的面, 决定形状
                                       face,// 草图面, 区域
                                       PI / 180 * 7,// 拔模角度, 单位是弧度
                                       Standard_True,// 融合方式
                                       Standard_False); // 是否修改原始形状
        make_prism.Perform(200);
        auto render = renders[r_id++];
#if 1
        TopoDS_Shape shape = make_prism.Shape();
        render->AddActor(Helper::shape_to_actor(shape));
#else
        // result_face(圆形)在box面上
        render->AddActor(Helper::shape_to_actor(box));

        auto actor_res_face = Helper::shape_to_actor(result_face);
        auto colors = vtkSmartPointer<vtkNamedColors>::New();
        actor_res_face->GetProperty()->SetColor(colors->GetColor3d("Pink").GetData());
        render->AddActor(actor_res_face);
#endif
    }

    // r_id = 15
    // pipe
    auto colors = vtkSmartPointer<vtkNamedColors>::New();
    {
        auto render = renders[r_id++];
        // 以xoy面为上下底,
        // exp顺序为:左右前后下上, 与bound顺序一致,xmin, xmax, ymin, ymax, zmin, zmax
        TopoDS_Shape shape = BRepPrimAPI_MakeBox(400, 250, 300);
        TopExp_Explorer exp;
        exp.Init(shape, TopAbs_FACE);
#if 0
        int target_id = 4;
        int i = 0;
        for (;exp.More(); exp.Next()) {
            TopoDS_Face face = TopoDS::Face(exp.Current());
            auto actor = Helper::shape_to_actor(face);
            if (i == target_id) {
                QString color_name = "DeepPink";
                actor->GetProperty()->SetColor(colors->GetColor3d(color_name.toUtf8().constData()).GetData());
            }
            render->AddActor(actor);
            i++;
        }
#endif
        exp.Next();// ex.current = 右
        exp.Next();// ex.current = 前
        TopoDS_Face box_front_face = TopoDS::Face(exp.Current());
        Handle(Geom_Surface) surf = BRep_Tool::Surface(box_front_face);

        BRepBuilderAPI_MakeWire make_wire;
        gp_Pnt2d p1, p2;
        p1 = gp_Pnt2d(100, 100);
        p2 = gp_Pnt2d(200, 100);
        Handle(Geom2d_Line) line = GCE2d_MakeLine(p1, p2).Value();
        make_wire.Add(BRepBuilderAPI_MakeEdge(line, surf, 0, p1.Distance(p2)));

        p1 = p2;
        p2 = gp_Pnt2d(150, 200);
        line = GCE2d_MakeLine(p1, p2).Value();
        make_wire.Add(BRepBuilderAPI_MakeEdge(line, surf, 0, p1.Distance(p2)));

        p1 = p2;
        p2 = gp_Pnt2d(100, 100);
        line = GCE2d_MakeLine(p1, p2).Value();
        make_wire.Add(BRepBuilderAPI_MakeEdge(line, surf, 0, p1.Distance(p2)));

        BRepBuilderAPI_MakeFace make_face;
        make_face.Init(surf, Standard_False, 1e-6);
        make_face.Add(make_wire.Wire());

        TopoDS_Face triangle_face = make_face.Face();
        BRepLib::BuildCurves3d(triangle_face);

        TColgp_Array1OfPnt curve_poles(1, 3);
        curve_poles(1) = gp_Pnt(150,   0, 150);
        curve_poles(2) = gp_Pnt(200, 125, 150);
        curve_poles(3) = gp_Pnt(150, 200, 150);
        Handle(Geom_BezierCurve) curve = new Geom_BezierCurve(curve_poles);
        TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(curve);
        TopoDS_Wire wire = BRepBuilderAPI_MakeWire(edge);

        BRepFeat_MakePipe make_pipe(shape, triangle_face, box_front_face, wire, Standard_False, Standard_True);
        make_pipe.Perform();
        TopoDS_Shape pipe = make_pipe.Shape();

        auto actor = Helper::shape_to_actor(pipe);
        actor->GetProperty()->SetOpacity(.2);
        render->AddActor(actor);
    }

    // r_id = 16
    // rib,肋条
    {
        auto render = renders[r_id++];
        BRepBuilderAPI_MakeWire make_wire;
        std::vector<gp_Pnt> pnts = {
            gp_Pnt(  0, 0, 0  ),
            gp_Pnt(200, 0, 0  ),
            gp_Pnt(200, 0, 50 ),
            gp_Pnt( 50, 0, 50 ),
            gp_Pnt( 50, 0, 200),
            gp_Pnt(  0, 0, 200),
            gp_Pnt(  0, 0, 0  ),
        };
        for (int i = 0; i < pnts.size() - 1; ++i) {
            make_wire.Add(BRepBuilderAPI_MakeEdge(pnts.at(i), pnts.at(i + 1)));
        }

        TopoDS_Shape prism = BRepPrimAPI_MakePrism(BRepBuilderAPI_MakeFace(make_wire.Wire()),
                                                   gp_Vec(gp_Pnt(0, 0, 0), gp_Pnt(0, 100, 0)));
        TopoDS_Wire wire = BRepBuilderAPI_MakeWire(BRepBuilderAPI_MakeEdge(gp_Pnt(50, 50, 100), gp_Pnt(100, 50, 50)));
        Handle(Geom_Plane) plane = new Geom_Plane(gp_Pnt(0, 50, 0), gp_Vec(0, 1, 0));
        BRepFeat_MakeLinearForm form(prism, wire, plane, gp_Dir(0, 50, 0), gp_Dir(0, -25, 0), 1, Standard_True);

        form.Perform();

        TopoDS_Shape rst = form.Shape();
        auto actor = Helper::shape_to_actor(rst);
        render->AddActor(actor);
    }

}
