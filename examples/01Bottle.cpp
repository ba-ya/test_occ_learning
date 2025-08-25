#include "00headers.h"
#include <IVtkTools_ShapeDataSource.hxx>
#include <IVtkOCC_Shape.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GC_MakeSegment.hxx>
#include <TopoDS_Edge.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>

void Bottle::Draw(vtkRenderer *renderer)
{
    Standard_Real height = 70.0, width = 50, thickness = 30;
    gp_Pnt pnt1(-width / 2, 0, 0);
    gp_Pnt pnt2(-width / 2, thickness / 4, 0);

    Handle(Geom_TrimmedCurve) segment1 = GC_MakeSegment(pnt1, pnt2);

    TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(segment1);
    auto actor = Helper::shape_to_actor(edge1);
    renderer->AddActor(actor);
}
