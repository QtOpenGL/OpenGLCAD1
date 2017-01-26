#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :  QMainWindow(parent), ui(new Ui::MainWindow) {
  qDebug() << "✓✓ MainWindow constructor";
  ui->setupUi(this);

  ui->controlNet->setChecked(true);

  ui->netPresets->addItem("Pentagon");
  ui->netPresets->addItem("Basis");

}

MainWindow::~MainWindow() {
  qDebug() << "✗✗ MainWindow destructor";
  delete ui;
}

void MainWindow::on_controlNet_toggled(bool checked) {
  ui->mainView->showNet = checked;
  ui->mainView->update();
}

void MainWindow::on_netPresets_currentIndexChanged(int index) {
  if (ui->mainView->isValid()) {
    ui->mainView->presetNet(index);
  }
}
