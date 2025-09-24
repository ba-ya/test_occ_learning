#ifndef COMMON_H
#define COMMON_H

#include <QDir>
#include <QRegularExpression>
#include <QString.h>
#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkRenderer.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
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

static QString str(const gp_Pnt2d &pnt) {
    return QString("(%1, %2)").arg(pnt.X()).arg(pnt.Y());
}
static QString str(const gp_Pnt &pnt) {
    return QString("(%1, %2, %3)").arg(pnt.X()).arg(pnt.Y()).arg(pnt.Z());
}


static std::array<double, 4> get_viewport(int row, int col, int rows, int cols) {
    std::array<double, 4> viewport;
    // xmin, ymin, xmax, ymax
    viewport[0] = static_cast<double>(col) / cols;
    viewport[1] = static_cast<double>(rows - row - 1) / rows;
    viewport[2] = static_cast<double>(col + 1) / cols;
    viewport[3] = static_cast<double>(rows - row) / rows;
    return viewport;
}
static void layout_renders_in_grid(std::vector<vtkSmartPointer<vtkRenderer>> renders,
                                   int grid_rows, int grid_cols) {
    for (int row = 0; row < grid_rows; ++row) {
        for (int col = 0; col < grid_cols; ++col) {
            auto index = row * grid_cols + col;
            auto viewport = get_viewport(row, col, grid_rows, grid_cols);
            if (index >= renders.size()) {
                break;
            }
            renders[index]->SetViewport(viewport.data());
        }
    }
}
}

#endif // COMMON_H
