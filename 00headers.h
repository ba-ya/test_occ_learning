#ifndef _1_CELLS_H
#define _1_CELLS_H

#include "common.h"
#include <vtkRenderer.h>

namespace Bottle {
void Draw(vtkRenderer* renderer);
}

namespace ModelingData {
void Draw(vtkRenderer* renderer);
}
namespace ModelingAlgorithms {
void Draw(std::vector<vtkSmartPointer<vtkRenderer>> renders);
}

#endif // _1_2D_CELLS_H
