#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_Slider_sphere_size_valueChanged(int value)
{
    float radius = value / 10.0f;
    ui->widget->getScene()->updateGeometry( radius );
    ui->widget->updateGL();
}

void MainWindow::on_Slider_ior_valueChanged(int value)
{
    float ior = value / 10.0f;
    ui->widget->getScene()->updateMaterial( ior );
    ui->widget->updateGL();
}
