// Author(s): Rimco Boudewijns
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "mainwindow.h"

#include "mcrl2/lts/lts_lts.h"
#include "mcrl2/utilities/exception.h"

#include <QSettings>

#include "springlayout.h"
#include "information.h"
#include "glwidget.h"
#include "dimensionsdialog.h"

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  m_fileDialog("", this)
{

  m_ui.setupUi(this);
  m_ui.dockOutput->setVisible(false);

  // Add graph area
  m_glwidget = new GLWidget(m_graph, m_ui.frame);
  m_glwidget->setDepth(0.0, 0);
  m_ui.widgetLayout->addWidget(m_glwidget);

  // Create springlayout algorithm + UI
  m_layout = new Graph::SpringLayout(m_graph);
  Graph::SpringLayoutUi* springlayoutui = m_layout->ui(this);
  addDockWidget(Qt::RightDockWidgetArea, springlayoutui);
  springlayoutui->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

  // Create GLWidget UI
  GLWidgetUi* glwidgetui = m_glwidget->ui(this);
  addDockWidget(Qt::RightDockWidgetArea, glwidgetui);
  glwidgetui->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

  // Add information UI
  m_information = new Graph::Information(m_graph);
  Graph::InformationUi* informationui = m_information->ui(this);
  addDockWidget(Qt::RightDockWidgetArea, informationui);
  informationui->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

  // Create timer for rendering (at 25fps)
  m_timer = new QTimer(this);

  // Connect signals & slots
  connect(m_glwidget, SIGNAL(widgetResized(const Graph::Coord3D&)), this, SLOT(onWidgetResized(const Graph::Coord3D&)));
  connect(m_ui.actLayoutControl, SIGNAL(toggled(bool)), springlayoutui, SLOT(setVisible(bool)));
  connect(m_ui.actVisualization, SIGNAL(toggled(bool)), glwidgetui, SLOT(setVisible(bool)));
  connect(m_ui.actInformation, SIGNAL(toggled(bool)), informationui, SLOT(setVisible(bool)));
  connect(m_ui.actOutput, SIGNAL(toggled(bool)), m_ui.dockOutput, SLOT(setVisible(bool)));
  connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimer()));
  connect(m_ui.act3D, SIGNAL(toggled(bool)), this, SLOT(on3DChanged(bool)));
  connect(m_ui.actLayout, SIGNAL(toggled(bool)), springlayoutui, SLOT(setActive(bool)));
  connect(m_ui.actReset, SIGNAL(triggered()), m_glwidget, SLOT(resetViewpoint()));
  connect(m_ui.actOpenFile, SIGNAL(triggered()), this, SLOT(onOpenFile()));
  connect(m_ui.actExportImage, SIGNAL(triggered()), this, SLOT(onExportImage()));
  connect(m_ui.actImport_XML, SIGNAL(triggered()), this, SLOT(onImportXML()));
  connect(m_ui.actExport_XML, SIGNAL(triggered()), this, SLOT(onExportXML()));
  connect(m_ui.dockWidgetOutput, SIGNAL(logMessage(QString, QString, QDateTime, QString, QString)), this, SLOT(onLogOutput(QString, QString, QDateTime, QString, QString)));

  m_timer->start(40);

  QSettings settings("mCRL2", "LTSGraph");
  restoreGeometry(settings.value("geometry").toByteArray());
  restoreState(settings.value("windowState").toByteArray());
  springlayoutui->setSettings(settings.value("settings").toByteArray());

  m_ui.actLayoutControl->setChecked(!springlayoutui->isHidden());
  m_ui.actVisualization->setChecked(!glwidgetui->isHidden());
  m_ui.actInformation->setChecked(!informationui->isHidden());
  m_ui.actOutput->setChecked(!m_ui.dockOutput->isHidden());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
  QSettings settings("mCRL2", "LTSGraph");
  settings.setValue("geometry", saveGeometry());
  settings.setValue("windowState", saveState());
  settings.setValue("settings", m_layout->ui()->settings());
  QMainWindow::closeEvent(event);
}

void MainWindow::showEvent(QShowEvent*)
{
  if (!m_delayedOpen.isEmpty())
  {
    openFile(m_delayedOpen);
    m_delayedOpen = QString();
  }
}

MainWindow::~MainWindow()
{
  delete m_timer;
  delete m_layout;
  delete m_information;
  delete m_glwidget;
}

void MainWindow::onWidgetResized(const Graph::Coord3D& newsize)
{
  m_graph.clip(-newsize / 2.0, newsize / 2.0);
  m_layout->setClipRegion(-newsize / 2.0, newsize / 2.0);
}

void MainWindow::on3DChanged(bool enabled)
{
  if (enabled)
    m_glwidget->setDepth(1000, 25);
  else
    m_glwidget->setDepth(0, 80);
}

void MainWindow::onTimer()
{
  m_glwidget->updateGL();
}

void MainWindow::onLogOutput(QString /*level*/, QString /*hint*/, QDateTime /*timestamp*/, QString /*message*/, QString formattedMessage)
{
  m_ui.statusBar->showMessage(formattedMessage, 5000);
}

void MainWindow::delayedOpenFile(QString fileName)
{
  m_delayedOpen = fileName;
}

void MainWindow::openFile(QString fileName)
{
  if (!fileName.isNull())
  {
    try
    {
      m_ui.actLayout->setChecked(false);
      m_glwidget->pause();
      m_glwidget->resetViewpoint(0);
      m_graph.load(fileName, -m_glwidget->size3() / 2.0, m_glwidget->size3() / 2.0);
      m_glwidget->rebuild();
      m_glwidget->resume();
      m_information->update();
      setWindowTitle(QString("LTSGraph - ") + fileName);
    }
    catch (mcrl2::runtime_error e)
    {
      QMessageBox::critical(this, "Error opening file", e.what());
      mCRL2log(mcrl2::log::error) << "Error opening file: " << e.what() << std::endl;
      setWindowTitle(QString("LTSGraph"));
    }
  }
}

void MainWindow::onOpenFile()
{

  QString fileName(m_fileDialog.getOpenFileName(tr("Open file"),
                                                tr("Labelled transition systems (*.lts *.aut *.fsm *.dot)")));

  openFile(fileName);
}

void MainWindow::onExportImage()
{
  QString bitmap = tr("Bitmap images (*.png *.jpg *.jpeg *.gif *.bmp *.pbm *.pgm *.ppm *.xbm *.xpm)");
  QString vector = tr("Vector format (*.pdf *.ps *.eps *.svg *.pgf)");
  QString tikz = tr("LaTeX TikZ Image (*.tex)");
  
  QString filter = bitmap + ";;" + vector + ";;" + tikz;
  QString selectedFilter = bitmap;
  QString fileName(m_fileDialog.getSaveFileName(tr("Save file"),
                                                filter,
                                                &selectedFilter));

  if (!fileName.isNull())
  {
    if (selectedFilter == bitmap)
    {
      DimensionsDialog dDialog(this);
      if (dDialog.exec())
      {
        m_glwidget->savePixmap(fileName, dDialog.resultWidth(), dDialog.resultHeight());
      }
    }
    else
	if (selectedFilter == vector)
    {
      m_glwidget->saveVector(fileName);
    }
	else
	{
      m_glwidget->saveTikz(fileName, m_glwidget->width() / m_glwidget->height());
	}
  }

}

void MainWindow::onImportXML()
{
  QString fileName(m_fileDialog.getOpenFileName(tr("Open file"),
                                                tr("XML Graph (*.xml)")));

  if (!fileName.isNull())
  {
    m_layout->ui()->setActive(false);
    m_glwidget->resetViewpoint(0);
    m_graph.loadXML(fileName);
    m_glwidget->rebuild();
    m_information->update();
  }

}

void MainWindow::onExportXML()
{
  QString fileName(m_fileDialog.getSaveFileName(tr("Save file"),
                                                tr("XML Graph (*.xml)")));

  if (!fileName.isNull())
  {
    m_graph.saveXML(fileName);
  }
}
