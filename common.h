#ifndef COMMON_H
#define COMMON_H

#include <QDir>
#include <QRegularExpression>
#include <QString.h>
#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkRenderer.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>.h>
#include <vtkSmartPointer.h>
#include <TopoDS_Shape.hxx>
#include <IVtkTools_ShapeDataSource.hxx>
#include <IVtkOCC_Shape.hxx>

namespace Helper {
static vtkSmartPointer<vtkActor> shape_to_actor(TopoDS_Shape &shape) {
    auto aDS = vtkSmartPointer<IVtkTools_ShapeDataSource>::New();
    Handle(IVtkOCC_Shape) vtkShape = new IVtkOCC_Shape(shape);
    aDS->SetShape(vtkShape);
    aDS->Update();

    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(aDS->GetOutputPort());
    mapper->Update();

    auto actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetRepresentationToSurface();
    actor->GetProperty()->SetEdgeVisibility(false);
    return actor;
}
}

#endif // COMMON_H
