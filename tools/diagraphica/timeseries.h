// Author(s): A.J. (Hannes) Pretorius
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file ./timeseries.h

#ifndef TIMESERIES_H
#define TIMESERIES_H

#include <QtCore>
#include <QtGui>

#include <cstddef>
#include <cstdlib>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <QTimer>
#include "attribute.h"
#include "diagram.h"
#include "graph.h"
#include "settings.h"
#include "visualizer.h"

class TimeSeries : public Visualizer
{
  Q_OBJECT
  public:
    // -- constructors and destructor -------------------------------
    TimeSeries(
      QWidget *parent,
      Settings* s,
      Graph* g);
    virtual ~TimeSeries();

    // -- get functions ---------------------------------------------
    void getIdcsClstMarked(std::set< size_t > &idcs);
    void getIdcsClstMarked(
      std::set< size_t > &idcs ,
      QColor& col);
    void getIdxMseOver(
      size_t& idxLeaf,
      std::set< size_t > &idcsBndl,
      QColor& colLeaf);
    void getCurrIdxDgrm(
      size_t& idxLeaf,
      std::set< size_t > &idcsBndl,
      QColor& colLeaf);
    void getAnimIdxDgrm(
      size_t& idxLeaf,
      std::set< size_t > &idcsBndl,
      QColor& colLeaf);
    void getAttrIdcs(std::vector< size_t > &idcs);

    void setDiagram(Diagram* dgrm);
    void initAttributes(const std::vector< size_t > attrIdcs);

  public slots:
    void clearData();
    void markItems(Cluster* frame);
    void markItems(QList<Cluster*> frames);

  public:
    // -- visualization functions  ----------------------------------
    void visualize(const bool& inSelectMode);

    // -- event handlers --------------------------------------------
    void handleMouseEvent(QMouseEvent* e);
    void handleWheelEvent(QWheelEvent* e);
    void handleMouseLeaveEvent();
    void handleKeyEvent(QKeyEvent* e);

    QSize sizeHint() const { return QSize(600,200); }

  signals:
    void routingCluster(Cluster *cluster, QList<Cluster *> clusterSet, QList<Attribute *> attributes);
    void hoverCluster(Cluster *cluster, QList<Attribute *> attributes = QList<Attribute *>());
    void marksChanged();
    void animationChanged();

  protected:
    // -- utility functions -----------------------------------------
    void calcSettingsGeomBased();
    void calcSettingsDataBased();
    void calcPositions();

    void clearDiagram();
    void clearAttributes();

    // -- utility event handlers ------------------------------------
  protected slots:
    void animate();
  protected:
    void handleRwndDiagram(const int& dgrmIdx);
    void handlePrevDiagram(const int& dgrmIdx);
    void handlePlayDiagram(const size_t& dgrmIdx);
    void handleNextDiagram(const int& dgrmIdx);

    // -- hit detection ---------------------------------------------
    void route();
    void handleHits(const std::vector< int > &ids);
    void processHits(
      GLint hits,
      GLuint buffer[]);

    // -- utility drawing functions ---------------------------------
    void clear();
    void drawSlider(const bool& inSelectMode);
    void drawScale(const bool& inSelectMode);
    void drawMarkedItems(const bool& inSelectMode);
    void drawAxes(const bool& inSelectMode);
    void drawAttrVals(const bool& inSelectMode);
    void drawDiagrams(const bool& inSelectMode);
    void drawMouseOver(const bool& inSelectMode);
    void drawLabels(const bool& inSelectMode);

    void initDragSlider();
    void clickSliderBar();
    void dragSlider();

    void handleHitItems(const int& idx);
    void handleDragItems(const int& idx);

    void handleShowDiagram(const int& dgrmIdx);
    void handleDragDiagram(const int& dgrmIdx);

    enum
    {
      ID_TIMER,
      ID_CANVAS,
      ID_SLIDER,
      ID_SLIDER_HDL,
      ID_SLIDER_HDL_LFT,
      ID_SLIDER_HDL_RGT,
      ID_ITEMS,
      ID_DIAGRAM,
      ID_DIAGRAM_CLSE,
      ID_DIAGRAM_MORE,
      ID_DIAGRAM_RWND,
      ID_DIAGRAM_PREV,
      ID_DIAGRAM_PLAY,
      ID_DIAGRAM_NEXT,
      DRAG_DIR_NULL,
      DRAG_DIR_LFT,
      DRAG_DIR_RGT,
      DRAG_STATUS_NONE,
      DRAG_STATUS_SLDR,
      DRAG_STATUS_SLDR_LFT,
      DRAG_STATUS_SLDR_RGT,
      DRAG_STATUS_ITMS,
      DRAG_STATUS_DGRM
    };

    // -- data members ----------------------------------------------
    QPoint m_lastMousePos;
    Settings* settings;

    Diagram* diagram;                // association
    std::vector< Attribute* > attributes; // association

    Position2D posSliderTopLft;
    Position2D posSliderBotRgt;
    double itvSlider;
    int    nodesItvSlider;
    double itvSliderPerNode;
    double sliderDragPosition;

    Position2D posScaleTopLft;
    Position2D posScaleBotRgt;
    double minPixPerNode;      // min number of pixels per item
    double actPixPerNode;      // actual number of pixels per item

    size_t    nodesWdwScale;
    double itvWdwPerNode;
    size_t    wdwStartIdx;
    int    nodesItvScale;      // number of nodes per marked interval

    std::vector< Position2D > posAxesTopLft;
    std::vector< Position2D > posAxesBotRgt;

    std::vector< std::vector< Position2D> > posValues;

    // interaction
    int dragStatus;
    double dragDistNodes;

    // selection
    std::set< size_t > itemsMarked;
    int  shiftStartIdx;
    int  dragStartIdx;
    int  dragDir;

    // mouse overs
    size_t mouseOverIdx;

    // diagrams
    double scaleDgrm;
    std::map< size_t, Position2D > showDgrm;
    int dragIdxDgrm; // diagram currently being dragged
    size_t currIdxDgrm; // currently selected diagram
    size_t animIdxDgrm; // diagram being animated

    double ySpacePxl;

    // animation
    QTimer m_animationTimer;
    std::set< size_t >::iterator animFrame;
};

#endif

// -- end -----------------------------------------------------------
