- [2D曲线映射到3D曲面](#2d曲线映射到3d曲面)
  - [1. **BRepBuilderAPI\_MakeEdge 的工作原理**](#1-brepbuilderapi_makeedge-的工作原理)
  - [2. **圆柱面参数化**](#2-圆柱面参数化)
  - [3. **arc1 如何映射到 3D？**](#3-arc1-如何映射到-3d)
  - [4. **结果**](#4-结果)
  - [5. **为什么要这样做？**](#5-为什么要这样做)
  - [**总结**](#总结)
- [曲线方程X(t) = Xc + major \* cos(t) \* dirX.x + minor \* sin(t) \* dirY.x](#曲线方程xt--xc--major--cost--dirxx--minor--sint--diryx)

# 2D曲线映射到3D曲面

`arc1` 这一段本身是 **2D 曲线**，并不在三维空间里，它是在圆柱面的 **参数平面 (u,v)** 上。真正让它“变成 3D 曲线”的，是这一句：

```cpp
TopoDS_Edge edge1_on_surf1 = BRepBuilderAPI_MakeEdge(arc1, cyl1);
```

---

## 1. **BRepBuilderAPI\_MakeEdge 的工作原理**

* 它接收两个主要参数：

  1. **一个 2D 曲线** (`arc1`)，在圆柱面的 (u,v) 空间。
  2. **一个 3D 圆柱面** (`cyl1`)。

* 这表示：**把 arc1 当作 cyl1 的参数曲线**，按照它的 u,v 值去圆柱表面找对应的 3D 点。

---

## 2. **圆柱面参数化**

圆柱面在 OpenCASCADE 中常用的参数化是：

* **u 方向** → 周向角（通常是弧度制：0 → 2π）
* **v 方向** → 圆柱高度（线性）

公式：

```
X(u,v) = R * cos(u)  
Y(u,v) = R * sin(u)  
Z(u,v) = v
```

（这里 R 是圆柱半径）

---

## 3. **arc1 如何映射到 3D？**

* `arc1` 是一条 2D 曲线 (u,v)，比如它的起点是 (u0, v0)，终点是 (u1, v1)。
* 每一个点 (u,v) 通过圆柱面的映射变为 3D 点 (x,y,z)。
* 于是 `arc1` 整条曲线就“贴”到了 `cyl1` 上。

---

## 4. **结果**

生成的 `edge1_on_surf1` 就是：

* 一个 **TopoDS\_Edge**，它有 3D 几何形态（空间坐标曲线），
* 还保留了它的 2D 曲线（参数空间曲线）作为参考。

这种双重表示在 OpenCASCADE 里非常重要，因为后续做布尔运算、放样、扫掠时都会用到 2D–3D 的一一对应关系。

---

## 5. **为什么要这样做？**

因为螺纹通常是在圆柱上生成的波形曲线，而不是一个独立的 3D 曲线。如果你用 2D 曲线 + 曲面，就能精确控制它的参数化位置和几何关系。

直接用 3D 曲线很难保证它完全贴合圆柱，尤其是做后续布尔运算时会出现“缝隙”或“失配”。

---

## **总结**

* `arc1` 在 2D 空间是半个椭圆波形。
* 通过 `BRepBuilderAPI_MakeEdge(arc1, cyl1)`，它被**映射到圆柱表面**，得到一条 3D 曲线。
* 这条曲线就是后续 **螺纹牙形轮廓的一部分**。

```
展开平面:  ——— 波浪曲线 ———
卷回圆柱:  ↘︎ 贴到圆柱表面 ↘︎
```

# 曲线方程X(t) = Xc + major * cos(t) * dirX.x + minor * sin(t) * dirY.x

```c++
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
```



