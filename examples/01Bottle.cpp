#include "00headers.h"
#include <IVtkTools_ShapeDataSource.hxx>
#include <IVtkOCC_Shape.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GC_MakeSegment.hxx>
#include <TopoDS_Edge.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <TopoDS_Wire.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <TopoDS.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Edge.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <Geom_Plane.hxx>
#include <BRepOffsetAPI_MakeThickSolid.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom2d_Ellipse.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <GCE2d_MakeSegment.hxx>
#include <BRepLib.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include <vtkNamedColors.h>

void Bottle::Draw(vtkRenderer *renderer)
{
    Standard_Real height = 70.0, width = 50, thickness = 30;
    gp_Pnt pnt1(-width / 2, 0, 0);
    gp_Pnt pnt2(-width / 2, -thickness / 4, 0);
    gp_Pnt pnt3(0, -thickness / 2, 0);
    gp_Pnt pnt4(width / 2, -thickness / 4, 0);
    gp_Pnt pnt5(width / 2, 0, 0);

    Handle(Geom_TrimmedCurve) segment1 = GC_MakeSegment(pnt1, pnt2);
    Handle(Geom_TrimmedCurve) segment2 = GC_MakeSegment(pnt4, pnt5);
    Handle(Geom_TrimmedCurve) arc_of_circle = GC_MakeArcOfCircle(pnt2, pnt3, pnt4);;

    TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(segment1);
    TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(arc_of_circle);
    TopoDS_Edge edge3 = BRepBuilderAPI_MakeEdge(segment2);
    // or, TopoDS_Edge edge3 = BRepBuilderAPI_MakeEdge(pnt4, pnt5);

    TopoDS_Wire wire = BRepBuilderAPI_MakeWire(edge1, edge2, edge3);

    // 沿x轴镜像生成闭合曲线
    gp_Ax1 xaxis = gp::OX();
    gp_Trsf trsf;
    trsf.SetMirror(xaxis);
    BRepBuilderAPI_Transform BRep_trsf(wire, trsf);
    TopoDS_Wire mirror_wire = TopoDS::Wire(BRep_trsf.Shape());

    BRepBuilderAPI_MakeWire mk_wires;
    mk_wires.Add(wire);
    mk_wires.Add(mirror_wire);
    TopoDS_Wire wires = mk_wires.Wire();

    // wires->faces
    TopoDS_Face face = BRepBuilderAPI_MakeFace(wires);
    // faces->prism
    gp_Vec prism_vec(0, 0, height);
    TopoDS_Shape body = BRepPrimAPI_MakePrism(face, prism_vec);

    // 边缘 sharp->round
    BRepFilletAPI_MakeFillet round_fillet(body);
    TopExp_Explorer edge_explorer(body, TopAbs_EDGE);
    while(edge_explorer.More()) {
        TopoDS_Edge edge = TopoDS::Edge(edge_explorer.Current());
        round_fillet.Add(thickness / 12, edge);
        edge_explorer.Next();
    }
    body = round_fillet.Shape();

    // neck
    Standard_Real neck_radius = thickness / 4;
    Standard_Real neck_height = height / 10;
    gp_Pnt neck_location(0, 0, height);
    gp_Dir neck_axis = gp::DZ();
    gp_Ax2 neck_ax2(neck_location, neck_axis);
    BRepPrimAPI_MakeCylinder cylinder(neck_ax2, neck_radius, neck_height);
    TopoDS_Shape neck = cylinder.Shape();
    body = BRepAlgoAPI_Fuse(body, neck);

    // neck 镂空
    TopoDS_Face face_to_remove;
    Standard_Real z_max = -1;
    for (TopExp_Explorer face_explorer(body, TopAbs_FACE); face_explorer.More(); face_explorer.Next()) {
        TopoDS_Face face = TopoDS::Face(face_explorer.Current());
        Handle(Geom_Surface) surface = BRep_Tool::Surface(face);
        if (surface->DynamicType() == STANDARD_TYPE(Geom_Plane)) {
            Handle(Geom_Plane) plane = Handle(Geom_Plane)::DownCast(surface);
            Standard_Real pos_z = plane->Location().Z();
            if (pos_z > z_max) {
                z_max = pos_z;
                face_to_remove = face;
            }
        }
    }
    TopTools_ListOfShape faces_to_remove;
    faces_to_remove.Append(face_to_remove);

    BRepOffsetAPI_MakeThickSolid mk_solid;
    mk_solid.MakeThickSolidByJoin(body, faces_to_remove, -thickness/50, 1e-3);
    body = mk_solid.Shape();

    // 螺纹
    Handle(Geom_CylindricalSurface) cyl1 = new Geom_CylindricalSurface(neck_ax2, neck_radius * 0.99);
    Handle(Geom_CylindricalSurface) cyl2 = new Geom_CylindricalSurface(neck_ax2, neck_radius * 1.05);

    // 右手定则确定y方向
    gp_Pnt2d pnt(2 * M_PI, neck_height / 2);
    gp_Dir2d dir(2 * M_PI, neck_height / 4);
    gp_Ax2d ax2d(pnt, dir);

    // 长半径2pi,
    Standard_Real major = 2 * M_PI;
    Standard_Real minor = neck_height / 10;
    Handle(Geom2d_Ellipse) ellipse1 = new Geom2d_Ellipse(ax2d, major, minor);
    Handle(Geom2d_Ellipse) ellipse2 = new Geom2d_Ellipse(ax2d, major, minor / 4);

    // X(t) = Xc + major * cos(t) * dirX.x + minor * sin(t) * dirY.x
    // Y(t) = Yc + major * cos(t) * dirX.y + minor * sin(t) * dirY.y
    // sin(t)在0和PI的时候 = 0,
    // 所以两个曲线的端点是一致的
    Handle(Geom2d_TrimmedCurve) arc1 = new Geom2d_TrimmedCurve(ellipse1, 0, M_PI);
    Handle(Geom2d_TrimmedCurve) arc2 = new Geom2d_TrimmedCurve(ellipse2, 0, M_PI);
    gp_Pnt2d ellipse_pnt1;
    gp_Pnt2d ellipse_pnt2;
    ellipse1->D0(0, ellipse_pnt1);
    ellipse1->D0(M_PI, ellipse_pnt2);

    Handle(Geom2d_TrimmedCurve) segment = GCE2d_MakeSegment(ellipse_pnt1, ellipse_pnt2);

    // 2D 曲线映射到 3D 圆柱面上，生成一条 3D 边。
    TopoDS_Edge edge1_on_surf1 = BRepBuilderAPI_MakeEdge(arc1, cyl1);
    TopoDS_Edge edge2_on_surf1 = BRepBuilderAPI_MakeEdge(segment, cyl1);
    TopoDS_Edge edge1_on_surf2 = BRepBuilderAPI_MakeEdge(arc2, cyl2);
    TopoDS_Edge edge2_on_surf2 = BRepBuilderAPI_MakeEdge(segment, cyl2);

    TopoDS_Wire thread_wire1 = BRepBuilderAPI_MakeWire(edge1_on_surf1, edge2_on_surf1);
    TopoDS_Wire thread_wire2 = BRepBuilderAPI_MakeWire(edge1_on_surf2, edge2_on_surf2);

    // 曲线添加3d信息
    BRepLib::BuildCurves3d(thread_wire1);
    BRepLib::BuildCurves3d(thread_wire2);

    BRepOffsetAPI_ThruSections tool(Standard_True);
    tool.AddWire(thread_wire1);
    tool.AddWire(thread_wire2);
    tool.CheckCompatibility(Standard_False);
    TopoDS_Shape thread = tool.Shape();

    TopoDS_Compound res;
    BRep_Builder builder;
    builder.MakeCompound(res);
    builder.Add(res, body);
    builder.Add(res, thread);

    auto actor = Helper::shape_to_actor(res);
    renderer->AddActor(actor);

    auto colors = vtkSmartPointer<vtkNamedColors>::New();
    actor->GetProperty()->SetColor(166 / 255., 166 / 255., 255/255.);
}
