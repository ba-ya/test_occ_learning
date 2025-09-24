------

### **1. 几何 (Geom/Geom2d) 相关**

| 抽象基类         | 常用子类（具体类型）                                         |
| ---------------- | ------------------------------------------------------------ |
| **Geom_Curve**   | `Geom_Line`（直线）、`Geom_Circle`（圆）、`Geom_Ellipse`（椭圆）、`Geom_BSplineCurve`（B样条曲线）、`Geom_BezierCurve`（贝塞尔曲线）、`Geom_TrimmedCurve`（裁剪曲线） |
| **Geom_Surface** | `Geom_Plane`（平面）、`Geom_CylindricalSurface`（圆柱面）、`Geom_ConicalSurface`（圆锥面）、`Geom_SphericalSurface`（球面）、`Geom_BSplineSurface`（B样条曲面） |
| **Geom2d_Curve** | `Geom2d_Line`、`Geom2d_Circle`、`Geom2d_Ellipse`、`Geom2d_BSplineCurve`、`Geom2d_BezierCurve`、`Geom2d_TrimmedCurve` |

------

### **2. 拓扑 (TopoDS) 相关**

| 抽象基类         | 常用子类（具体类型）                                         |
| ---------------- | ------------------------------------------------------------ |
| **TopoDS_Shape** | `TopoDS_Vertex`（顶点）、`TopoDS_Edge`（边）、`TopoDS_Wire`（线框）、`TopoDS_Face`（面）、`TopoDS_Shell`、`TopoDS_Solid`、`TopoDS_Compound` |

> **注意**：TopoDS 里的“抽象基类”本质是一个通用容器（`TopoDS_Shape`），它通过 `ShapeType()` 来区分具体类型，而不像 Geom 那样靠纯虚函数。

------

### **3. 数据结构 (Adaptor 类)**

这些是适配器接口，不是直接的几何数据：

| 抽象基类            | 常用子类或适配对象                         |
| ------------------- | ------------------------------------------ |
| **Adaptor_Curve**   | 适配 `Geom_Curve` 系列（直线、圆、样条等） |
| **Adaptor_Surface** | 适配 `Geom_Surface` 系列（平面、圆柱面等） |
| **Adaptor_Curve2d** | 适配 `Geom2d_Curve` 系列                   |

------

### **总结**

- **抽象基类 (不能直接 new)**：定义接口，供算法统一调用。

- **常用子类 (可以 new)**：具体实现各种曲线/曲面/形状。

- 例如：

  ```cpp
  Handle(Geom_Curve) c = new Geom_Circle(...); // ✅ 正确
  Handle(Geom_Curve) c = new Geom_Curve();     // ❌ 错误，抽象类不能实例化
  ```
