#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVTKOpenGLNativeWidget.h>
#include <vtkRenderer.h>
#include <TopoDS_Shape.hxx>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btn_reset_camera_released();

private:
    void init();
    void init_examples();
    void do_something(const QString &name);

    void clear();
    void reset_camera();
    void do_render();

    void Bottle();

private:
    Ui::MainWindow *ui;
    QVTKOpenGLNativeWidget *vtk_widget;
    vtkSmartPointer<vtkRenderer> renderer;
};
#endif // MAINWINDOW_H
