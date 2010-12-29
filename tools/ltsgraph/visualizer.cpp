// Author(s): Carst Tankink and Ali Deniz Aladagli
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file visualizer.cpp
/// \brief Visualization functions.

#include "wx.hpp" // precompiled headers

#include "visualizer.h"
#include <sstream>
#include <cmath>
#include "ids.h"

Visualizer::Visualizer(LTSGraph* app)
{
  owner = app;
  width = 0;
  height = 0;
  pixelSize = 1;
  radius = 10;
  showHandles = false;
  showTransLabels = true;
  showStateLabels = true;
  showStateVector = false;
  fr = NULL;
}

Visualizer::~Visualizer()
{
}

void Visualizer::visualize(double _width, double _height, double _pixelSize,
                           bool inSelectMode, bool enabled3d)
{
  width = _width;
  height = _height;
  depth = (width + height) / 2;
  draw3d = enabled3d;
  pixelSize = _pixelSize;

  if (!fr)
  {
    fr = new mcrl2::utilities::wx::font_renderer();
  }

  drawStates(inSelectMode);
}

void Visualizer::drawStates(bool inSelectMode)
{

  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  Graph* graph = owner->getGraph();


  if (graph)
  {
      for(size_t i = 0; i < graph->getNumberOfStates(); ++i)
	  {
		State* s = graph->getState(i);
		if(draw3d)
			drawState(s);

		for(size_t j = 0; j < s->getNumberOfTransitions(); ++j)
		{
		  Transition* t = s->getTransition(j);
		  drawTransition(t, j, inSelectMode);
		}
		
		for(size_t j = 0; j < s->getNumberOfSelfLoops(); ++j)
		{
  		  Transition* t = s->getSelfLoop(j);
		  drawSelfLoop(t, j, inSelectMode);
		}
	  }
	  if(draw3d)
	  {
		  if (showTransLabels)
		  {
			for(size_t i = 0; i < graph->getNumberOfStates(); ++i) 
			{
			  State* s = graph->getState(i);

			  for(size_t j = 0; j < s->getNumberOfTransitions(); ++j)
			  {
  				Transition* t = s->getTransition(j);
				drawTransLabel(t, j, inSelectMode);
			  }
			  for(size_t j = 0; j < s->getNumberOfSelfLoops(); ++j)
			  {
  				Transition* t = s->getSelfLoop(j);
				drawTransLabel(t, j, inSelectMode);
			  }
			}	  
		  }	
		  for(size_t i = 0; i < graph->getNumberOfStates(); ++i) 
		  {
			State* s = graph->getState(i);
			if(s->getShowStateVector() || showStateLabels)
			  drawStateText(s);
		  }	
	  }
	  else
	  {
	    for(size_t i = 0; i < graph->getNumberOfStates(); ++i)
		{
		  State* s = graph->getState(i);


		  glPushMatrix();

		  drawState(s);

		  glPopMatrix();
		}
	  }
  }
}
void Visualizer::setWidth(double _width)
{
  width = _width;
  depth = (width + height) / 2;
}

void Visualizer::setHeight(double _height)
{
  height = _height;
  depth = (width + height) / 2;
}


void Visualizer::drawState(State* s)
{
  if(draw3d)
  {
	  quadratic=gluNewQuadric();							
	  gluQuadricNormals(quadratic, GLU_SMOOTH);		

	  double x = s->getX();
	  double y = s->getY();
	  double z = s->getZ();
	  double rad =  radius * pixelSize;

	  x = (x / 2000.0) * (width - rad * 2);
	  y = (y / 2000.0) * (height - rad * 2);
	  z = (z / 2000.0) * (depth - rad * 2);


	  // Draw border of states
	  if(s->isSelected())
	  {
		glColor4ub(255, 0, 0, 255);  // TODO: Param
	  }
	  else if(s->isInitialState())
	  {
		glColor4ub(0, 255, 0, 255);
	  }
	  else
	  {
		glColor4ub(0, 0, 0, 255);
	  }

	  glPushMatrix();
	  double cmvm[16];
	  double dumatrix[16] = {1,0,0,0,0,1,0,0,0,0,1,0,x,y,z,1};
	  owner->getCanvasMdlvwMtrx(cmvm);
	  double dumatrix2[16];
	  Utils::MultGLMatrices(cmvm, dumatrix, dumatrix2);
	  for (int i = 12; i < 16; i++)
		  dumatrix[i] = dumatrix2[i];
	  glLoadMatrixd(dumatrix);
	  
	  gluPartialDisk(quadratic,rad,rad*1.1,16,16,0,360);	
	  glPopMatrix();


	  if(s->isLocked())
	  {
		// Grey out a locked state
		glColor4ub(50, 50, 50, 255);
	  }

	  else
	  {
		wxColour c = s->getColour();
		glColor4ub(c.Red(), c.Green(), c.Blue(), 255);
	  }

	  glEnable(GL_LIGHT0);
	  glEnable(GL_LIGHTING);
	  glPushName(IDS::STATE);
	  glPushName( (GLuint) s->getValue());
	  
	  glPushMatrix();
	  glTranslatef(x, y, z);
	  gluSphere(quadratic,rad,16,16);
	  glPopMatrix();
	  gluDeleteQuadric(quadratic);
	  glDisable(GL_LIGHTING);
	  glDisable(GL_LIGHT0);

	  glPopName();
	  glPopName();
  }
  else
  {
	  double x = s->getX();
	  double y = s->getY();
	  double rad =  radius * pixelSize;

	  x = (x / 2000.0) * (width - rad * 2);
	  y = (y / 2000.0) * (height - rad * 2);
	  float t = 0.0;
	  int prec = 50;  // TODO: Make parameterisable
	  float step = 1.0 / prec;


	  if(s->isLocked())
	  {
		// Grey out a locked state
		glColor3ub(170, 170, 170);
	  }

	  else
	  {
		wxColour c = s->getColour();

		glColor3ub(c.Red(), c.Green(), c.Blue());
	  }


	  glPushName(IDS::STATE);
	  glPushName( (GLuint) s->getValue());


	  glBegin(GL_TRIANGLE_FAN);
		glVertex2d(x, y);
		while ( t <= 1.0 )
		{
		  double phi = 2 * M_PI * t;
		  double nx = x + rad * cos(phi);
		  double ny = y + rad * sin(phi);

		  glVertex2d(nx, ny);

		  t += step;
		}
	  glEnd();

	  t = 0.0f;
	  // Draw border of states
	  if(s->isSelected())
	  {
		glColor3ub(255, 0, 0);  // TODO: Param
	  }
	  else if(s->isInitialState())
	  {
		glColor3ub(0, 255, 0);
	  }
	  else
	  {
		glColor3ub(0, 0, 0);
	  }

	  glBegin(GL_LINE_STRIP);
		while (t <= 1.0)
		{
		  double phi = 2 * M_PI * t;
		  double nx = x + rad * cos(phi);
		  double ny = y + rad * sin(phi);

		  glVertex2d(nx, ny);

		  t += step;
		}
	  glEnd();


	  // Draw label
	  std::stringstream labelstr;
	  labelstr << s->getValue();

	  if(showStateLabels) {
		fr->draw_text(labelstr.str(), x, y, (rad - 2 * pixelSize) / 24.0f,
					mcrl2::utilities::wx::al_center, mcrl2::utilities::wx::al_top);
	  }

	  glPopName();
	  glPopName();

	  // Draw state vector
	  bool showStateVector = s->getShowStateVector();
	  if(showStateVector) {
		std::stringstream vectorstr;
	   std::map<std::string, std::string>::iterator it;
	   std::map<std::string, std::string> stateParameters = s->getParameters();

		for(it = stateParameters.begin(); it != stateParameters.end(); ++it) {
		 vectorstr << it->first <<": " << it->second << "\n";
		} 

		fr->draw_wrapped_text(vectorstr.str(), x + rad * 2, width,  
			  height - y, y, (rad - 2 * pixelSize) / 24.0f,
			  mcrl2::utilities::wx::al_left, mcrl2::utilities::wx::al_bottom);

	  }
  }
}

void Visualizer::drawTransition(Transition* tr, size_t trid, bool selecting)
{
  State* from = tr->getFrom();
  State* to = tr->getTo();

  double rad = radius * pixelSize;

  double xFrom = from->getX();
  double yFrom = from->getY();
  double zFrom = from->getZ();
  double xTo = to->getX();
  double yTo = to->getY();
  double zTo = to->getZ();

  double xVirtual, yVirtual, zVirtual;
  double xControl, yControl, zControl;

  tr->getControl(xVirtual, yVirtual, zVirtual);


  // (xVirtual, yVirtual, zVirtual) is a "virtual" control point, which should lie on the
  // curve (For non-selfloops).
  // We need to calculate a spline control point such that the 2nd order
  // Bezier curve has its apex on (xControl, yControl, zControl) = V.
  //
  // A (2nd order) Bezier spline is defined by:
  // P(t) =  t    *    t  * S
  //      + 2t    * (1-t) * C
  //      + (1-t) * (1-t) * E
  // Where S is the start point of the curve, E the endpoint and C the actual
  // control point.
  // For V, we would like P(0.5) = .25 * S + .5 * C + .25 * E
  // Now, since V, S and E are known, we can compute C as:
  // C = 2 * V - .5 (S + E)
  // As these operations are scalar multiplications and vector additions, these
  // can be done per component (x, y, z).
  xControl = 2.0 * xVirtual - .5 * (xFrom + xTo);
  yControl = 2.0 * yVirtual - .5 * (yFrom + yTo);
  zControl = 2.0 * zVirtual - .5 * (zFrom + zTo);

  xFrom = (xFrom / 2000.0) * (width - rad * 2);
  yFrom = (yFrom / 2000.0) * (height - rad * 2);
  zFrom = (zFrom / 2000.0) * (depth - rad * 2);

  xTo =   (xTo / 2000.0) * (width - rad * 2);
  yTo =   (yTo / 2000.0) * (height - rad * 2);
  zTo =   (zTo / 2000.0) * (depth - rad * 2);

  xControl = (xControl / 2000.0) * (width - rad * 2);
  yControl = (yControl / 2000.0) * (height - rad * 2);
  zControl = (zControl / 2000.0) * (depth - rad * 2);

  xVirtual = (xVirtual / 2000.0) * (width - rad * 2);
  yVirtual = (yVirtual / 2000.0) * (height - rad * 2);
  zVirtual = (zVirtual / 2000.0) * (depth - rad * 2);

  // Draw a Bezier curve through the control points
  GLdouble ctrlPts[3][3] = {{xFrom, yFrom, zFrom},
                            {xControl, yControl, zControl},
                            {xTo, yTo, zTo}};

  double t, it, b0, b1, b2, x, y, z;
  int N = 50; // TODO: Parameterisable
  glColor3ub(0, 0, 0); // TODO: Parameterisable


  glBegin(GL_LINE_STRIP);
    for(int k  = 0; k < N; ++k)
    {
      t = static_cast<float>(k) / (N - 1);
      it = 1.0f - t;
      b0 =      t *  t;
      b1 = 2 *  t * it;
      b2 =     it * it;

      x = b0 * ctrlPts[0][0] +
          b1 * ctrlPts[1][0] +
          b2 * ctrlPts[2][0];
      y = b0 * ctrlPts[0][1] +
          b1 * ctrlPts[1][1] +
          b2 * ctrlPts[2][1];
      z = b0 * ctrlPts[0][2] +
          b1 * ctrlPts[1][2] +
          b2 * ctrlPts[2][2];
	  if(draw3d)
		  glVertex3d(x, y, z);
	  else
		  glVertex2d(x, y);
    }
  glEnd();


  // Transitions are uniquely identified by their from state and the identifier
  // within this state

  if(showHandles)
  {
    glPushName(IDS::TRANSITION);
    glPushName( (GLuint) from->getValue());
    glPushName( (GLuint) trid);

	glColor3ub(255,255,255);
	if(draw3d)
	{
		glBegin(GL_QUADS);

		  glVertex3d(xVirtual - 0.00725, yVirtual, zVirtual + 0.00725);
		  glVertex3d(xVirtual, yVirtual + 0.00725, zVirtual + 0.00725);
		  glVertex3d(xVirtual + 0.00725, yVirtual, zVirtual + 0.00725);
		  glVertex3d(xVirtual, yVirtual - 0.00725, zVirtual + 0.00725);

		  glVertex3d(xVirtual - 0.00725, yVirtual, zVirtual - 0.00725);
		  glVertex3d(xVirtual, yVirtual + 0.00725, zVirtual - 0.00725);
		  glVertex3d(xVirtual, yVirtual + 0.00725, zVirtual + 0.00725);
		  glVertex3d(xVirtual - 0.00725, yVirtual, zVirtual + 0.00725);

		  glVertex3d(xVirtual, yVirtual + 0.00725, zVirtual - 0.00725);
		  glVertex3d(xVirtual + 0.00725, yVirtual, zVirtual - 0.00725);
		  glVertex3d(xVirtual + 0.00725, yVirtual, zVirtual + 0.00725);
		  glVertex3d(xVirtual, yVirtual + 0.00725, zVirtual + 0.00725);

		  glVertex3d(xVirtual + 0.00725, yVirtual, zVirtual - 0.00725);
		  glVertex3d(xVirtual, yVirtual - 0.00725, zVirtual - 0.00725);
		  glVertex3d(xVirtual, yVirtual - 0.00725, zVirtual + 0.00725);
		  glVertex3d(xVirtual + 0.00725, yVirtual, zVirtual + 0.00725);

		  glVertex3d(xVirtual, yVirtual - 0.00725, zVirtual - 0.00725);
		  glVertex3d(xVirtual - 0.00725, yVirtual, zVirtual - 0.00725);
		  glVertex3d(xVirtual - 0.00725, yVirtual, zVirtual + 0.00725);
		  glVertex3d(xVirtual, yVirtual - 0.00725, zVirtual + 0.00725);

		  glVertex3d(xVirtual - 0.00725, yVirtual, zVirtual - 0.00725);
		  glVertex3d(xVirtual, yVirtual + 0.00725, zVirtual - 0.00725);
		  glVertex3d(xVirtual + 0.00725, yVirtual, zVirtual - 0.00725);
		  glVertex3d(xVirtual, yVirtual - 0.00725, zVirtual - 0.00725);

		glEnd();
	}
	else
	{
		glColor3ub(255, 255, 255);
		glBegin(GL_QUADS);
			glVertex2d(xVirtual , yVirtual - .015);
			glVertex2d(xVirtual - .015, yVirtual);
			glVertex2d(xVirtual, yVirtual + .015);
			glVertex2d(xVirtual + .015, yVirtual);
		glEnd();
	}

    if(tr->isSelected())
    {
      glColor3ub(255, 0, 0);
    }
    else
    {
      glColor3ub(0, 0, 0);
    }
	if(draw3d)
		{
		glBegin(GL_LINE_STRIP);

		  glVertex3d(xVirtual - 0.00725, yVirtual, zVirtual + 0.00725);
		  glVertex3d(xVirtual, yVirtual + 0.00725, zVirtual + 0.00725);
		  glVertex3d(xVirtual + 0.00725, yVirtual, zVirtual + 0.00725);
		  glVertex3d(xVirtual, yVirtual - 0.00725, zVirtual + 0.00725);

		glEnd();
		glBegin(GL_LINE_STRIP);

		  glVertex3d(xVirtual - 0.00725, yVirtual, zVirtual - 0.00725);
		  glVertex3d(xVirtual, yVirtual + 0.00725, zVirtual - 0.00725);
		  glVertex3d(xVirtual, yVirtual + 0.00725, zVirtual + 0.00725);
		  glVertex3d(xVirtual - 0.00725, yVirtual, zVirtual + 0.00725);

		glEnd();
		glBegin(GL_LINE_STRIP);

		  glVertex3d(xVirtual, yVirtual + 0.00725, zVirtual - 0.00725);
		  glVertex3d(xVirtual + 0.00725, yVirtual, zVirtual - 0.00725);
		  glVertex3d(xVirtual + 0.00725, yVirtual, zVirtual + 0.00725);
		  glVertex3d(xVirtual, yVirtual + 0.00725, zVirtual + 0.00725);

		glEnd();
		glBegin(GL_LINE_STRIP);

		  glVertex3d(xVirtual + 0.00725, yVirtual, zVirtual - 0.00725);
		  glVertex3d(xVirtual, yVirtual - 0.00725, zVirtual - 0.00725);
		  glVertex3d(xVirtual, yVirtual - 0.00725, zVirtual + 0.00725);
		  glVertex3d(xVirtual + 0.00725, yVirtual, zVirtual + 0.00725);

		glEnd();
		glBegin(GL_LINE_STRIP);

		  glVertex3d(xVirtual, yVirtual - 0.00725, zVirtual - 0.00725);
		  glVertex3d(xVirtual - 0.00725, yVirtual, zVirtual - 0.00725);
		  glVertex3d(xVirtual - 0.00725, yVirtual, zVirtual + 0.00725);
		  glVertex3d(xVirtual, yVirtual - 0.00725, zVirtual + 0.00725);

		glEnd();
		glBegin(GL_LINE_STRIP);

		  glVertex3d(xVirtual - 0.00725, yVirtual, zVirtual - 0.00725);
		  glVertex3d(xVirtual, yVirtual + 0.00725, zVirtual - 0.00725);
		  glVertex3d(xVirtual + 0.00725, yVirtual, zVirtual - 0.00725);
		  glVertex3d(xVirtual, yVirtual - 0.00725, zVirtual - 0.00725);

		glEnd();
	}
	else
	{
		glBegin(GL_LINE_LOOP);
			glVertex2d(xVirtual , yVirtual - .015);
			glVertex2d(xVirtual - .015, yVirtual);
			glVertex2d(xVirtual, yVirtual + .015);
			glVertex2d(xVirtual + .015, yVirtual);
		glEnd();
	}

    glPopName();
    glPopName();
    glPopName();
  }

  if(draw3d)
  {
	  glColor3ub(0, 0, 0);

	  double xDif = xTo - xControl;
	  double yDif = yTo - yControl;
	  double zDif = zTo - zControl;

	  double tanXZ = atan2(xDif, zDif) * 180.0f / M_PI;
	  double angYxz = atan2(yDif, sqrt(xDif * xDif + zDif * zDif)) * 180.0f / M_PI;

	  glPushMatrix();
	  glTranslatef(xTo, yTo, zTo);
	  glRotatef(tanXZ, 0.0f, 1.0f, 0.0f);
	  glRotatef(angYxz, -1.0f, 0.0f, 0.0f);
	  glTranslatef(0.0f, 0.0f, -2 * rad);
	  drawArrowHead3d(rad);
	  glPopMatrix();
  }
  else
  {
	  if(showTransLabels) {
		  // Draw label near the control point
		  // point
		  double labelX, labelY, labelZ;
		  tr->getLabelPos(labelX, labelY, labelZ);
		  labelX = (labelX / 2000.0) * (width - rad * 2);
		  labelY = (labelY / 2000.0) * (height - rad * 2);


		  if(tr->isSelected())
		  {
			  glColor3ub(255, 0, 0);
		  }
		  else
		  {
			  glColor3ub(0, 0, 0);
		  }

		  if(selecting) {
			  glPushName(IDS::LABEL);
			  glPushName( (GLuint) from->getValue());
			  glPushName( (GLuint) trid);
			  fr->draw_bounding_box(tr->getLabel(), labelX, labelY + .025,
				  8 * pixelSize / 20.0f,
				  mcrl2::utilities::wx::al_center, mcrl2::utilities::wx::al_top, false);
			  glPopName();
			  glPopName();
			  glPopName();
		  }
		  else {
			  fr->draw_text(tr->getLabel(), labelX, labelY + .025,
				  8 * pixelSize / 20.0f,
				  mcrl2::utilities::wx::al_center, mcrl2::utilities::wx::al_top);
		  }
	  }

	  glColor3ub(0, 0, 0);
	  
	  float ang = atan2(yControl - yTo, xControl - xTo)
		  * 180.0f / M_PI;

	  glTranslatef(xTo, yTo, 0.0f);
	  glRotatef(90 + ang, 0.0, 0.0, 1.0f);
	  glTranslatef(0.0f, -rad * 2, 0.0f);
	  drawArrowHead(rad);
	  glTranslatef(0.0f,  rad * 2, 0.0f);
	  glRotatef(-90 - ang, 0.0f, 0.0f, 1.0f);
	  glTranslatef(-xTo, -yTo, 0.0f);
  }
}

void Visualizer::drawArrowHead3d(double baseLength)
{
  quadratic = gluNewQuadric();							
  gluQuadricNormals(quadratic, GLU_SMOOTH);
  gluCylinder(quadratic,baseLength/5,0.0f,baseLength,64,64);
  gluDeleteQuadric(quadratic);
}

void Visualizer::drawArrowHead(double baseLength)
{
  glBegin(GL_TRIANGLES);
    glVertex2d(-baseLength / 2, 0);
    glVertex2d( baseLength / 2, 0);
    glVertex2d( 0,  baseLength);
  glEnd();
}

void Visualizer::drawSelfLoop(Transition* tr, size_t j, bool selecting)
{
  // We are drawing a self loop, so t.to == t.from
  State* s = tr->getFrom();

  double rad = radius * pixelSize;
  double piover4 = .25 * M_PI;

  double xState, yState, zState;
  double xVirtual, yVirtual, zVirtual;

  double alpha = tr->getControlAlpha();
  double beta = tr->getControlBeta();
  double gamma = tr->getControlGamma();

  xState = s->getX();
  yState = s->getY();
  zState = s->getZ();

  tr->getControl(xVirtual, yVirtual, zVirtual);

  double theAngle;
  if(draw3d)
	  theAngle = beta;
  else
	  theAngle = alpha;
  // Calculate control points of the curve
  // TODO: Explain
  double zeta = beta + piover4;
  double delta = beta - piover4;

  double xFactor = 1;
  double cosZeta = cos(zeta);
  double cosDelta = cos(delta);
  double sinZeta = sin(zeta);
  double sinDelta = sin(delta);

  if(fabs(cosZeta + cosDelta) > 0.01)
  {
    xFactor = (8 *(xVirtual - xState)) / (3 * (cosZeta + cosDelta));
  }
  double xControl1 = xState + xFactor * cosZeta;
  double xControl2 = xState + xFactor * cosDelta;
  double yControl1;
  double yControl2;

  if(fabs(sinZeta + sinDelta) <= 0.01)
  {
    float additive = tan(piover4) * (xControl1 - xState);
    yControl1 = yState + additive;
    yControl2 = yState - additive;
  }
  else
  {
    double yFactor = (8 * (yVirtual - yState)) / (3 * (sinZeta + sinDelta));
    yControl1 = yState + yFactor * sinZeta;
    yControl2 = yState + yFactor * sinDelta;

    if(fabs(cosZeta + cosDelta) <= .01)
    {
      float additive = tan(piover4) * (yControl1 - yState);
      xControl1 = xState - additive;
      xControl2 = xState + additive;
    }
  }

  double zControl = zState;
  if (fabs(sin(gamma)) > 0.01)
  {
	  double zFactor = (4 * (zVirtual - zState)) / (3 * (sin(gamma)));
	  zControl = zState + zFactor * sin(gamma);
  }



  // Normalize points for drawing on glContext
  xState = (xState / 2000.0f) * (width - rad * 2.0);
  yState = (yState / 2000.0f) * (height - rad * 2.0);
  zState = (zState / 2000.0f) * (depth - rad * 2.0);


  xVirtual = (xVirtual / 2000.0f) * (width - rad * 2.0);
  yVirtual = (yVirtual / 2000.0f) * (height - rad * 2.0);
  zVirtual = (zVirtual / 2000.0f) * (depth - rad * 2.0);

  xControl1 = (xControl1 / 2000.0f) * (width - rad * 2.0);
  yControl1 = (yControl1 / 2000.0f) * (height - rad * 2.0);
  zControl = (zControl / 2000.0f) * (depth - rad * 2.0);


  xControl2 = (xControl2 / 2000.0f) * (width - rad * 2.0);
  yControl2 = (yControl2 / 2000.0f) * (height - rad * 2.0);




  // Draw cubic Bezier curve through the control points
  GLdouble ctrlPts[3][3] = {{xState, yState, zState},
                            {xControl1, yControl1, zControl},
                            {xControl2, yControl2, zControl}};


  double t, it, b0, b1, b2, b3, x, y, z;
  int N = 50; // TODO: Parameterisable
  glColor3ub(0, 0, 0); // TODO: Parameterisable


  glBegin(GL_LINE_STRIP);
    for(int k  = 0; k < N; ++k)
    {
      t = static_cast<float>(k) / (N - 1);
      it = 1.0f - t;

      b0 =      t *  t *  t;
      b1 = 3 *  t *  t * it;
      b2 = 3 *  t * it * it;
      b3 =     it * it * it;


      x = b0 * ctrlPts[0][0] +
          b1 * ctrlPts[1][0] +
          b2 * ctrlPts[2][0] +
          b3 * ctrlPts[0][0];


      y = b0 * ctrlPts[0][1] +
          b1 * ctrlPts[1][1] +
          b2 * ctrlPts[2][1] +
          b3 * ctrlPts[0][1];

	  
	  z = b0 * ctrlPts[0][2] +
          b1 * ctrlPts[1][2] +
          b2 * ctrlPts[2][2] +
          b3 * ctrlPts[0][2];

	  if(draw3d)
		  glVertex3d(x, y, z);
	  else
		  glVertex2d(x, y);
    }
  glEnd();

  if(showHandles)
  {
	glPushName(IDS::SELF_LOOP);
    glPushName( (GLuint) s->getValue());
    glPushName( (GLuint) j);

	glColor3ub(255,255,255);
	if(draw3d)
		{
		glBegin(GL_QUADS);

		  glVertex3d(xVirtual - 0.00725, yVirtual, zVirtual + 0.00725);
		  glVertex3d(xVirtual, yVirtual + 0.00725, zVirtual + 0.00725);
		  glVertex3d(xVirtual + 0.00725, yVirtual, zVirtual + 0.00725);
		  glVertex3d(xVirtual, yVirtual - 0.00725, zVirtual + 0.00725);

		  glVertex3d(xVirtual - 0.00725, yVirtual, zVirtual - 0.00725);
		  glVertex3d(xVirtual, yVirtual + 0.00725, zVirtual - 0.00725);
		  glVertex3d(xVirtual, yVirtual + 0.00725, zVirtual + 0.00725);
		  glVertex3d(xVirtual - 0.00725, yVirtual, zVirtual + 0.00725);

		  glVertex3d(xVirtual, yVirtual + 0.00725, zVirtual - 0.00725);
		  glVertex3d(xVirtual + 0.00725, yVirtual, zVirtual - 0.00725);
		  glVertex3d(xVirtual + 0.00725, yVirtual, zVirtual + 0.00725);
		  glVertex3d(xVirtual, yVirtual + 0.00725, zVirtual + 0.00725);

		  glVertex3d(xVirtual + 0.00725, yVirtual, zVirtual - 0.00725);
		  glVertex3d(xVirtual, yVirtual - 0.00725, zVirtual - 0.00725);
		  glVertex3d(xVirtual, yVirtual - 0.00725, zVirtual + 0.00725);
		  glVertex3d(xVirtual + 0.00725, yVirtual, zVirtual + 0.00725);

		  glVertex3d(xVirtual, yVirtual - 0.00725, zVirtual - 0.00725);
		  glVertex3d(xVirtual - 0.00725, yVirtual, zVirtual - 0.00725);
		  glVertex3d(xVirtual - 0.00725, yVirtual, zVirtual + 0.00725);
		  glVertex3d(xVirtual, yVirtual - 0.00725, zVirtual + 0.00725);

		  glVertex3d(xVirtual - 0.00725, yVirtual, zVirtual - 0.00725);
		  glVertex3d(xVirtual, yVirtual + 0.00725, zVirtual - 0.00725);
		  glVertex3d(xVirtual + 0.00725, yVirtual, zVirtual - 0.00725);
		  glVertex3d(xVirtual, yVirtual - 0.00725, zVirtual - 0.00725);

		glEnd();
	}
	else
	{
		glBegin(GL_QUADS);
			glVertex2d(xVirtual , yVirtual - .015);
			glVertex2d(xVirtual - .015, yVirtual);
			glVertex2d(xVirtual, yVirtual + .015);
			glVertex2d(xVirtual + .015, yVirtual);
		glEnd();
	}

    if(tr->isSelected())
    {
      glColor3ub(255, 0, 0);
    }
    else
    {
      glColor3ub(0, 0, 0);
    }

	if(draw3d)
	{
		glBegin(GL_LINE_STRIP);

		  glVertex3d(xVirtual - 0.00725, yVirtual, zVirtual + 0.00725);
		  glVertex3d(xVirtual, yVirtual + 0.00725, zVirtual + 0.00725);
		  glVertex3d(xVirtual + 0.00725, yVirtual, zVirtual + 0.00725);
		  glVertex3d(xVirtual, yVirtual - 0.00725, zVirtual + 0.00725);

		glEnd();
		glBegin(GL_LINE_STRIP);

		  glVertex3d(xVirtual - 0.00725, yVirtual, zVirtual - 0.00725);
		  glVertex3d(xVirtual, yVirtual + 0.00725, zVirtual - 0.00725);
		  glVertex3d(xVirtual, yVirtual + 0.00725, zVirtual + 0.00725);
		  glVertex3d(xVirtual - 0.00725, yVirtual, zVirtual + 0.00725);

		glEnd();
		glBegin(GL_LINE_STRIP);

		  glVertex3d(xVirtual, yVirtual + 0.00725, zVirtual - 0.00725);
		  glVertex3d(xVirtual + 0.00725, yVirtual, zVirtual - 0.00725);
		  glVertex3d(xVirtual + 0.00725, yVirtual, zVirtual + 0.00725);
		  glVertex3d(xVirtual, yVirtual + 0.00725, zVirtual + 0.00725);

		glEnd();
		glBegin(GL_LINE_STRIP);

		  glVertex3d(xVirtual + 0.00725, yVirtual, zVirtual - 0.00725);
		  glVertex3d(xVirtual, yVirtual - 0.00725, zVirtual - 0.00725);
		  glVertex3d(xVirtual, yVirtual - 0.00725, zVirtual + 0.00725);
		  glVertex3d(xVirtual + 0.00725, yVirtual, zVirtual + 0.00725);

		glEnd();
		glBegin(GL_LINE_STRIP);

		  glVertex3d(xVirtual, yVirtual - 0.00725, zVirtual - 0.00725);
		  glVertex3d(xVirtual - 0.00725, yVirtual, zVirtual - 0.00725);
		  glVertex3d(xVirtual - 0.00725, yVirtual, zVirtual + 0.00725);
		  glVertex3d(xVirtual, yVirtual - 0.00725, zVirtual + 0.00725);

		glEnd();
		glBegin(GL_LINE_STRIP);

		  glVertex3d(xVirtual - 0.00725, yVirtual, zVirtual - 0.00725);
		  glVertex3d(xVirtual, yVirtual + 0.00725, zVirtual - 0.00725);
		  glVertex3d(xVirtual + 0.00725, yVirtual, zVirtual - 0.00725);
		  glVertex3d(xVirtual, yVirtual - 0.00725, zVirtual - 0.00725);

		glEnd();
	}
	else
	{
		glBegin(GL_LINE_LOOP);
			glVertex2d(xVirtual , yVirtual - .015);
			glVertex2d(xVirtual - .015, yVirtual);
			glVertex2d(xVirtual, yVirtual + .015);
			glVertex2d(xVirtual + .015, yVirtual);
		glEnd();
	}

    glPopName();
    glPopName();
    glPopName();
  }


  if(draw3d)
  {
	  double xDif = xState - xControl2;
	  double yDif = yState - yControl2;
	  double zDif = zState - zControl;

	  double tanXZ = atan2(xDif, zDif) * 180.0f / M_PI;
	  double angYxz = atan2(yDif, sqrt(xDif * xDif + zDif * zDif)) * 180.0f / M_PI;
	  glColor3ub(0, 0, 0);
	  glPushMatrix();
	  glTranslatef(xState, yState, zState);
	  glRotatef(tanXZ, 0.0f, 1.0f, 0.0f);
	  glRotatef(angYxz, -1.0f, 0.0f, 0.0f);
	  glTranslatef(0.0f, 0.0f, -2 * rad);
	  drawArrowHead3d(rad);
	  glPopMatrix();
  }
  else
  {
	  // Draw label.
	  if(showTransLabels) {
		double labelX, labelY, labelZ;
		tr->getLabelPos(labelX, labelY, labelZ);
		labelX = (labelX / 2000.0) * (width - rad * 2);
		labelY = (labelY / 2000.0) * (height - rad * 2);

		if(selecting) {
		  glPushName(IDS::SELF_LABEL);
		  glPushName( (GLuint) s->getValue());
		  glPushName( (GLuint) j);

		  fr->draw_bounding_box(tr->getLabel(), labelX, labelY + .025,
					  8 * pixelSize / 20.0f,
					  mcrl2::utilities::wx::al_center, mcrl2::utilities::wx::al_top, false);
		  glPopName();
		  glPopName();
		  glPopName();
		}
		else {
		  fr->draw_text(tr->getLabel(), labelX, labelY + .025,
					8 * pixelSize / 20.0f,
					mcrl2::utilities::wx::al_center, mcrl2::utilities::wx::al_top);
		}
	  }


	  glColor3ub(0, 0, 0);

	  float ang = atan2(yControl2 - yState, xControl2 - xState)
				  * 180.0f / M_PI;

	  glTranslatef(xState, yState, 0.0f);
	  glRotatef(90 + ang, 0.0, 0.0, 1.0f);
	  glTranslatef(0.0f, -rad * 2, 0.0f);
	  drawArrowHead(rad);
	  glTranslatef(0.0f,  rad * 2, 0.0f);
	  glRotatef(-90 - ang, 0.0f, 0.0f, 1.0f);
	  glTranslatef(-xState, -yState, 0.0f);
  }
}
	
void Visualizer::drawTransLabel(Transition* tr, size_t trid, bool selecting)
{
	// Draw label near the control point
  double labelX, labelY, labelZ;
  tr->getLabelPos(labelX, labelY, labelZ);
	double rad = radius * pixelSize;
  labelX = (labelX / 2000.0) * (width - rad * 2);
  labelY = (labelY / 2000.0) * (height - rad * 2);
	labelZ = (labelZ / 2000.0) * (depth - rad * 2);
	State* from = tr->getFrom();

    if(tr->isSelected())
    {
      glColor4ub(255, 0, 0, 255);
    }
    else
    {
      glColor4ub(0, 0, 0, 255);
    }
	glDepthMask(GL_FALSE);					
	glPushMatrix();
	double cmvm[16];
	double dumatrix[16] = {1,0,0,0,0,1,0,0,0,0,1,0,labelX,labelY,labelZ,1};
	owner->getCanvasMdlvwMtrx(cmvm);
	double dumatrix2[16];
	Utils::MultGLMatrices(cmvm, dumatrix, dumatrix2);
	for (int i = 12; i < 16; i++)
	dumatrix[i] = dumatrix2[i];
	glLoadMatrixd(dumatrix);
	

    if(selecting) {
	  if(from == tr->getTo())
		  glPushName(IDS::SELF_LABEL);
	  else
		  glPushName(IDS::LABEL);
      glPushName( (GLuint) from->getValue());
      glPushName( (GLuint) trid);
      fr->draw_bounding_box(tr->getLabel(), 0, .025, 0,
				  8 * pixelSize / 20.0f,
                  mcrl2::utilities::wx::al_center, mcrl2::utilities::wx::al_top, false);
      glPopName();
      glPopName();
      glPopName();
    }
    else {
      fr->draw_text(tr->getLabel(), 0, .025, 0,
                8 * pixelSize / 20.0f,
                mcrl2::utilities::wx::al_center, mcrl2::utilities::wx::al_top);
    }

	glPopMatrix();
	glDepthMask(GL_TRUE);
}

void Visualizer::drawStateText(State* s)
{
  glDepthMask(GL_FALSE);
  double x, y, z;
  x = s->getX();
  y = s->getY();
  z = s->getZ();
  double rad = radius * pixelSize;
  x = (x / 2000.0) * (width - rad * 2);
  y = (y / 2000.0) * (height - rad * 2);
  z = (z / 2000.0) * (depth - rad * 2);

  glPushMatrix();
  double cmvm[16];
  double dumatrix[16] = {1,0,0,0,0,1,0,0,0,0,1,0,x,y,z,1};
  owner->getCanvasMdlvwMtrx(cmvm);
  double dumatrix2[16];
  Utils::MultGLMatrices(cmvm, dumatrix, dumatrix2);
  for (int i = 12; i < 16; i++)
	  dumatrix[i] = dumatrix2[i];
  glLoadMatrixd(dumatrix);
  

  double xDif = -dumatrix[12];
  double yDif = -dumatrix[13];
  double zDif = -dumatrix[14];
  double tanXZ = atan2(xDif, zDif) * 180.0f / M_PI;
  double angYxz = atan2(yDif, sqrt(xDif * xDif + zDif * zDif)) * 180.0f / M_PI;

  glRotatef(tanXZ, 0.0f, 1.0f, 0.0f);
  glRotatef(-angYxz, 1.0f, 0.0f, 0.0f);

  // Draw label
  std::stringstream labelstr;
  labelstr << s->getValue();

  if(showStateLabels) 
  {
	glColor3ub(0,0,0);
	fr->draw_text(labelstr.str(), 0, 0, 1.1 * rad, (rad - 2 * pixelSize) / 24.0f,
		mcrl2::utilities::wx::al_center, mcrl2::utilities::wx::al_top);
  }

  // Draw state vector
  bool showStateVector = s->getShowStateVector();
  
  if(showStateVector) 
  {
	std::stringstream vectorstr;
    std::map<std::string, std::string>::iterator it;
    std::map<std::string, std::string> stateParameters = s->getParameters();

    for(it = stateParameters.begin(); it != stateParameters.end(); ++it)
	{
      vectorstr << it->first <<": " << it->second << "\n";
    } 

    fr->draw_wrapped_text(vectorstr.str(), rad, rad * 2, width - x,  
    height - 2 * y, 0, (rad - 2 * pixelSize) / 24.0f,
    mcrl2::utilities::wx::al_left, mcrl2::utilities::wx::al_bottom);

  }
  glPopMatrix();
  glDepthMask(GL_TRUE);

}

void Visualizer::setPixelSize(double ps)
{
  pixelSize = ps;
}

void Visualizer::initFontRenderer()
{
  if (!fr)
  {
    fr = new mcrl2::utilities::wx::font_renderer();
  }
}

void Visualizer::setRadius(int radius)
{
  this->radius = radius;
}

int Visualizer::getRadius() const 
{
  return radius;
}

void Visualizer::setCurves(bool value)
{
  showHandles = value;
}

void Visualizer::setTransLabels(bool value) 
{
  showTransLabels = value;
}

void Visualizer::setStateLabels(bool value) 
{
  showStateLabels = value;
}

void Visualizer::drawCoorSystem()
{
	double length;
	double cmvm[16];
	length = 0.3;
	owner->getCanvasMdlvwMtrx(cmvm);
	glDisable(GL_DEPTH_TEST);
	cmvm[12] = 0;
	cmvm[13] = 0;
	cmvm[14] = -1;
	cmvm[15] = 1;
	glLoadMatrixd(cmvm); 
	
	glBegin(GL_LINES);
	  glColor3ub(255, 0, 0);
	  glVertex3f(0.0f, 0.0f, 0.0f);
	  glVertex3f(length, 0.0f, 0.0f);
	  glColor3ub(0, 255, 0);
	  glVertex3f(0.0f, 0.0f, 0.0f);
	  glVertex3f(0.0f, length, 0.0f);
	  glColor3ub(0, 0, 255);
	  glVertex3f(0.0f, 0.0f, 0.0f);
	  glVertex3f(0.0f, 0.0f, length);
	glEnd();
//arrow heads for axises
	double tanXZ = atan2(0.0, 0.0) * 180.0f / M_PI;
	double angYxz = atan2(1.0, 0.0) * 180.0f / M_PI;

	glColor3ub(0, 255, 0);
	glPushMatrix();
	glTranslatef(0.0f, length, 0.0f);
	glRotatef(tanXZ, 0.0f, 1.0f, 0.0f);
	glRotatef(angYxz, -1.0f, 0.0f, 0.0f);
	drawArrowHead3d(0.1);
	glPopMatrix();
	tanXZ = atan2(1.0, 0.0) * 180.0f / M_PI;
	angYxz = atan2(0.0, 1.0) * 180.0f / M_PI;

  glColor3ub(255, 0, 0);
	glPushMatrix();
	glTranslatef(length, 0.0f, 0.0f);
	glRotatef(tanXZ, 0.0f, 1.0f, 0.0f);
	glRotatef(angYxz, -1.0f, 0.0f, 0.0f);
	drawArrowHead3d(0.1);
	glPopMatrix();
	tanXZ = atan2(0.0, 1.0) * 180.0f / M_PI;
	angYxz = atan2(0.0, 1.0) * 180.0f / M_PI;

	glColor3ub(0, 0, 255);
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, length);
	glRotatef(tanXZ, 0.0f, 1.0f, 0.0f);
	glRotatef(angYxz, -1.0f, 0.0f, 0.0f);
	drawArrowHead3d(0.1);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
}
