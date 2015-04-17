#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void keyPressEvent( QKeyEvent* event );


private slots:
    void on_Slider_sphere_size_valueChanged(int value);

    void on_Slider_ior_valueChanged(int value);

    void on_Slider_light_position_valueChanged(int value);

    void on_checkBox_memory_clicked(bool checked);

    void on_checkBox_fps_clicked(bool checked);

    void on_checkBox_acceleration_clicked(bool checked);

    void on_checkBox_antialiasing_clicked(bool checked);

    void on_Slider_paint_camera_valueChanged(int value);

    void on_comboBox_activated(int index);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
