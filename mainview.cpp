#include "mainview.h"
#include "math.h"
#include <QDebug>

MainView::MainView(QWidget *parent) : QOpenGLWidget(parent) {
  qDebug() << "✓✓ MainView constructor";

  selectedPt = -1;
}

MainView::~MainView() {
  qDebug() << "✗✗ MainView destructor";

  clearArrays();

  // Delete vertex arrays
  glDeleteBuffers(1, &netCoordsBO);
  glDeleteVertexArrays(1, &netVAO);

  // delete shader programs
  delete mainShaderProg;
}

// ---

void MainView::createShaderPrograms() {

    // Main shader program
    mainShaderProg = new QOpenGLShaderProgram();
    mainShaderProg->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertshader.glsl");
    mainShaderProg->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshader.glsl");
    mainShaderProg->link();
}

void MainView::createBuffers() {

    // Original points
    glGenVertexArrays(1, &netVAO);
    glBindVertexArray(netVAO); // VAO1, It can be defined after BO1

    glGenBuffers(1, &netCoordsBO);
    glBindBuffer(GL_ARRAY_BUFFER, netCoordsBO); //BO1, It can be defined before VAO1

    glEnableVertexAttribArray(0); 
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
}

void MainView::refresh(){
    update();
    updateBuffers();
    selectedPt = -1; // Deselect point when options are (de)selected etc. Just personal preference
}

float sq(float r){ // Self explanatory
    return r * r;
}

void MainView::updateBuffers() {
    // Load data in buffers
    glBindBuffer(GL_ARRAY_BUFFER, netCoordsBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QVector2D)*netCoords.size(), netCoords.data(), GL_DYNAMIC_DRAW); // Loads data to the currently bound BO.

    update();
}



void MainView::clearArrays() {

  // As of Qt 5.6, clear() does not release the memory anymore. Use e.g. squeeze()
    netCoords.clear();
    netCoords.squeeze();
}



void MainView::initializeGL() {

    initializeOpenGLFunctions();
    qDebug() << ":: OpenGL initialized";

    // If the application crashes here, try setting "MESA_GL_VERSION_OVERRIDE = 4.1"
    // and "MESA_GLSL_VERSION_OVERRIDE = 410" in Projects (left panel) -> Build Environment

    QString glVersion;
    glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    qDebug() << ":: Using OpenGL" << qPrintable(glVersion);

    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);
    // Default is GL_LESS
    glDepthFunc(GL_LEQUAL);

    // ---

    createShaderPrograms();
    createBuffers();

    presetNet(0);
  }

  void MainView::paintGL() {

    glClearColor(0.0,0.0,0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (showNet)
    { // Show the net
        mainShaderProg->bind();
        glBindVertexArray(netVAO);
        glDrawArrays(GL_LINE_STRIP, 0, netCoords.size());

        glPointSize(8.0);
        glDrawArrays(GL_POINTS, 0, netCoords.size());

        // Highlight selected control point
        if (selectedPt > -1) {
          glPointSize(12.0);
          glDrawArrays(GL_POINTS, selectedPt, 1);
        }

      glBindVertexArray(0);
      mainShaderProg->release();

    }
}



void MainView::presetNet(unsigned short preset) {

  selectedPt = -1;
  clearArrays();

  switch (preset) {
  case 0:
    // 'Pentagon'
    netCoords.reserve(5);
    netCoords.append(QVector2D(-0.25, -0.5));
    netCoords.append(QVector2D(-0.72, 0.0));
    netCoords.append(QVector2D(-0.25, 0.73));
    netCoords.append(QVector2D(0.79, 0.5));
    netCoords.append(QVector2D(0.5, -0.73));
    break;
  case 1:
    // 'Basis'
    netCoords.reserve(9);
    netCoords.append(QVector2D(-1.0, -0.25));
    netCoords.append(QVector2D(-0.75, -0.25));
    netCoords.append(QVector2D(-0.5, -0.25));
    netCoords.append(QVector2D(-0.25, -0.25));
    netCoords.append(QVector2D(0.0, 0.50));
    netCoords.append(QVector2D(0.25, -0.25));
    netCoords.append(QVector2D(0.5, -0.25));
    netCoords.append(QVector2D(0.75, -0.25));
    netCoords.append(QVector2D(1.0, -0.25));
    break;
  }

  updateBuffers();
}

void MainView::mousePressEvent(QMouseEvent *event) {

  // In order to allow keyPressEvents:
  setFocus();

  float xRatio, yRatio, xScene, yScene;

  xRatio = (float)event->x() / width();
  yRatio = (float)event->y() / height();

  qDebug()<<"event->x(): "<< event->x();
  qDebug()<<"event->y(): "<< event->y();

  qDebug()<<"xRatio: "<< xRatio;
  qDebug()<<"yRatio: "<< yRatio;

  // By default, the drawing canvas is the square [-1,1]^2:
  xScene = (1-xRatio)*-1 + xRatio*1;
  qDebug()<<"xScene: "<< xScene;
  // Note that the origin of the canvas is in the top left corner (not the lower left).
  yScene = yRatio*-1 + (1-yRatio)*1;
  qDebug()<<"yScene: "<< yScene;

  switch (event->buttons()) {
  case Qt::LeftButton:
    if (selectedPt > -1) {
      // De-select control point
      selectedPt = -1;
      setMouseTracking(false);
      update();
    }
    else {
      // Add new control point
      netCoords.append(QVector2D(xScene, yScene));
      updateBuffers();
    }
    break;
  case Qt::RightButton:
    // Select control point
    selectedPt = findClosest(xScene, yScene);
    update();
    break;
  }

}

void MainView::mouseMoveEvent(QMouseEvent *event) {

  if (selectedPt > -1) {
    float xRatio, yRatio, xScene, yScene;

    xRatio = (float)event->x() / width();
    yRatio = (float)event->y() / height();

    xScene = (1-xRatio)*-1 + xRatio*1;
    yScene = yRatio*-1 + (1-yRatio)*1;

    // Update position of the control point
    netCoords[selectedPt] = QVector2D(xScene, yScene);
    updateBuffers();
  }

}

void MainView::keyPressEvent(QKeyEvent *event) {

  // Only works when the widget has focus!

  switch(event->key()) {
  case 'G':
    if (selectedPt > -1) {
      // Grab selected control point
      setMouseTracking(true);
    }
    break;
  case 'X':
    if (selectedPt > -1) {
      // Remove selected control point
      netCoords.remove(selectedPt);
      selectedPt = -1;
      updateBuffers();
    }
    break;
  }

}

short int MainView::findClosest(float x, float y) {

  short int ptIndex = 0;
  float currentDist, minDist = 4;

  for (int k=0; k<netCoords.size(); k++) {
    currentDist = pow((netCoords[k].x()-x),2) + pow((netCoords[k].y()-y),2);
    if (currentDist < minDist) {
      minDist = currentDist;
      ptIndex = k;
    }
  }

  return ptIndex;
}
