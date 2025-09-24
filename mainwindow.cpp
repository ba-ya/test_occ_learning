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

    cow = vtkSmartPointer<vtkCameraOrientationWidget>::New();

    init_examples();

    connect(ui->btn_xoy, &QPushButton::released, this, [this]() {
        reset_camera(XOY);
        do_render();
    });
    connect(ui->btn_yoz, &QPushButton::released, this, [this]() {
        reset_camera(YOZ);
        do_render();
    });
    connect(ui->btn_zox, &QPushButton::released, this, [this]() {
        reset_camera(ZOX);
        do_render();
    });
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
        ui->lineEdit_name->setText(QString("%1-%2").arg(item->row()+ 1).arg(item->text()));
        do_something(item->text());
    });
}

void MainWindow::do_something(const QString &name)
{
    clear();
    int size = 1;
    if (renders.size() != size) {
        resize_render(size, 1 , 1);
    }
    auto m_render = renders[0];
    if (name == "Bottle") {
        Bottle::Draw(m_render);
    } else if (name == "ModelingData") {
        ModelingData::Draw(m_render);
    } else if (name == "ModelingAlgorithms") {
        resize_render(20, 5, 4);
        ModelingAlgorithms::Draw(renders);
    }
    if (renders.size() == 1) {
        cow->SetParentRenderer(m_render);
        cow->On();
    } else {
        cow->Off();
    }
    reset_camera();
    do_render();
}

void MainWindow::resize_render(int count, int grid_rows, int grid_cols)
{
    for (auto &render : renders) {
        vtk_widget->renderWindow()->RemoveRenderer(render);
    }
    renders.clear();
    auto colors = vtkSmartPointer<vtkNamedColors>::New();
    std::vector<double *> color_list = {
        colors->GetColor3d("SlateGray").GetData(),
        colors->GetColor3d("DarkSlateGray").GetData(),
    };
    for (int i = 0; i < count; ++i) {
        auto render = vtkSmartPointer<vtkRenderer>::New();
        render->SetBackground(color_list.at(i % color_list.size()));

        vtk_widget->renderWindow()->AddRenderer(render);
        renders.push_back(render);
    }
    Helper::layout_renders_in_grid(renders, grid_rows, grid_cols);
}

void MainWindow::clear()
{
    for (auto &render : renders) {
        render->RemoveAllViewProps();
    }
}

void MainWindow::reset_camera(VIEWPLANE plane)
{
    double pos[3]{0, 0, 0}, focal[3]{0, 0, 0}, up[3]{0, 0, 0};
    if (plane == XOY) {
        pos[2] = 600;// z
        up[1] = 1; // y
    } else if (plane == YOZ){
        pos[0] = 600;// x
        up[2] = 1;// z
    } else {
        pos[1] = 600;// y
        up[0] = 1; // x
    }
    for (auto &render : renders) {
        auto camera = render->GetActiveCamera();
        camera->SetPosition(pos);
        camera->SetFocalPoint(focal);
        camera->SetViewUp(up);
        render->ResetCamera();
        render->ResetCameraClippingRange();
    }
}

void MainWindow::do_render()
{
    vtk_widget->renderWindow()->Render();
}


