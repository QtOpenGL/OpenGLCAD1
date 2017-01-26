#ifndef MAINVIEW_H
#define MAINVIEW_H

#include <QOpenGLFunctions_4_1_Core>
#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>

#include <QVector2D>
#include <QMouseEvent>

class MainView : public QOpenGLWidget, protected QOpenGLFunctions_4_1_Core {

  Q_OBJECT

public:
  MainView(QWidget *parent = 0);
  ~MainView();

  void clearArrays(); // Clears net

  void presetNet(unsigned short preset);
  void updateBuffers();

  bool showNet;
  short int selectedPt;
  short int findClosest(float x, float y);

  void refresh(); // updateBuffers + update

protected:
  void initializeGL();
  void paintGL();

  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void keyPressEvent(QKeyEvent *event);

private:
  QOpenGLShaderProgram* mainShaderProg;
  QVector<QVector2D> netCoords;
  GLuint netVAO, netCoordsBO;
  void createShaderPrograms();
  void createBuffers();

};

#endif // MAINVIEW_H
