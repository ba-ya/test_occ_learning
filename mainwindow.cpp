#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkNamedColors.h>
#include <vtkCamera.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

#include <QDir>
#include "00headers.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{
    auto renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();

    vtk_widget = new QVTKOpenGLNativeWidget(this);
    vtk_widget->setRenderWindow(renderWindow);
    ui->layout->addWidget(vtk_widget);

    renderer = vtkSmartPointer<vtkRenderer>::New();
    auto colors = vtkSmartPointer<vtkNamedColors>::New();
    renderer->SetBackground(colors->GetColor3d("SlateGray").GetData());
    vtk_widget->renderWindow()->AddRenderer(renderer);

    init_examples();
}

void MainWindow::init_examples()
{
    auto &&get_class_name = [this]() {
        QStringList names;
        auto dir = QDir(QDir::currentPath());
        dir.cdUp(); dir.cdUp();
        auto file_path = QDir(dir).absoluteFilePath("00headers.h");
        QFile file(file_path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << file_path << "open fail";
            return names;
        }
        QTextStream in(&file);
        QString content = in.readAll();
        file.close();

        // 使用正则匹配 namespace 名称
        static QRegularExpression regex(R"(namespace\s*(\w+)\s*\{)");
        QRegularExpressionMatchIterator i = regex.globalMatch(content);
        while (i.hasNext()) {
            names << i.next().captured(1);
        }
        return names;
    };
    auto &&init_table = [](QTableWidget *table, QStringList files) {
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        auto cnt_row = files.size();
        table->setRowCount(cnt_row);
        table->setColumnCount(1);
        for (int i = 0; i < cnt_row; ++i) {
            table->setItem(i, 0, new QTableWidgetItem(files.at(i)));
        }
    };
    init_table(ui->table, get_class_name());

    connect(ui->table, &QTableWidget::itemDoubleClicked, this, [this](QTableWidgetItem *item) {
        ui->lineEdit_name->setText(QString("%1, %2").arg(item->row()+ 1).arg(item->text()));
        do_something(item->text());
    });
}

void MainWindow::do_something(const QString &name)
{
    clear();
    if (name == "Bottle") {
        Bottle::Draw(renderer);
    }
    reset_camera();
    do_render();
}

void MainWindow::clear()
{
    renderer->RemoveAllViewProps();
}

void MainWindow::reset_camera()
{
    auto camera = renderer->GetActiveCamera();
    camera->SetPosition(0, 0, 600);
    camera->SetFocalPoint(0, 0, 0);
    camera->SetViewUp(0, 1, 0);
    renderer->ResetCamera();
    renderer->ResetCameraClippingRange();
}

void MainWindow::do_render()
{
    vtk_widget->renderWindow()->Render();
}

void MainWindow::Bottle()
{

}


void MainWindow::on_btn_reset_camera_released()
{
    reset_camera();
    do_render();
}

