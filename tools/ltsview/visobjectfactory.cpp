// Author(s): Bas Ploeger and Carst Tankink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file visobjectfactory.cpp
/// \brief Add your file description here.

#include <algorithm>
#include <cstdlib>
#include "visobjectfactory.h"
using namespace std;
using namespace Utils;

class VisObject 
{
  public:
    VisObject();
    ~VisObject();
    float* getMatrixP() const;
    RGB_Color getColor() const;
    Point3D getCoordinates() const;
    int getPrimitive() const;
    void setColor(Utils::RGB_Color c);
    void setPrimitive(int p);
    void draw(PrimitiveFactory *pf,unsigned char alpha);
    void addIdentifier(int id);
  private:
    float* matrix;
    RGB_Color color;
    int primitive;
    vector<int> identifiers;
                
};

VisObject::VisObject() {
	matrix = (float*)malloc(16*sizeof(float));
	color.r = 150;
	color.g = 150;
	color.b = 150;
}

VisObject::~VisObject() {
	free(matrix);
}

float* VisObject::getMatrixP() const {
	return matrix;
}

RGB_Color VisObject::getColor() const {
	return color;
}

int VisObject::getPrimitive() const {
	return primitive;
}

Point3D VisObject::getCoordinates() const {
  Point3D result = { matrix[12],matrix[13],matrix[14] };
  return result;
}

void VisObject::setColor(RGB_Color c) {
	color = c;
}

void VisObject::setPrimitive(int p) {
	primitive = p;
}

void VisObject::draw(PrimitiveFactory *pf,unsigned char alpha) {
	glColor4ub(color.r,color.g,color.b,alpha);
	glPushMatrix();
        
	glMultMatrixf(matrix);

        for(size_t i = 0; i < identifiers.size(); ++i)
        {
          glPushName(identifiers[i]);
        }
        
	pf->drawPrimitive(primitive);

        for(size_t i = 0; i < identifiers.size(); ++i)
        {
          glPopName();
        }
	glPopMatrix();
}

void VisObject::addIdentifier(int id)
{
  identifiers.push_back(id);
}
/* -------------------- Distance -------------------------------------------- */

class Distance {
  private:
    Point3D viewpoint;
  public:
    explicit Distance(const Point3D vp) : viewpoint(vp) {}
    bool operator()(const VisObject* o1,const VisObject* o2) const;
};

bool Distance::operator()(const VisObject *o1, const VisObject *o2) const {
  Point3D d1 = o1->getCoordinates() - viewpoint;
  Point3D d2 = o2->getCoordinates() - viewpoint;
  return (dot_product(d1,d1) > dot_product(d2,d2));
}

/* -------------------- VisObjectFactory ------------------------------------ */

VisObjectFactory::VisObjectFactory() {
}

VisObjectFactory::~VisObjectFactory() {
	clear();
}

void VisObjectFactory::sortObjects(Point3D viewpoint) {
	stable_sort(objects_sorted.begin(),objects_sorted.end(),Distance(viewpoint));
}

void VisObjectFactory::drawObjects(PrimitiveFactory *pf,unsigned char alpha) {
	for (unsigned int i = 0; i < objects_sorted.size(); ++i) {
		objects_sorted[i]->draw(pf,alpha);
	}
}

void VisObjectFactory::clear() {
	for (unsigned int i = 0; i < objects.size(); ++i) {
		delete objects[i];
	}
	objects.clear();
	objects_sorted.clear();
}

int VisObjectFactory::makeObject(int primitive, vector<int> ids) {
	VisObject *vo = new VisObject();
	glGetFloatv(GL_MODELVIEW_MATRIX,(GLfloat*)vo->getMatrixP());
	vo->setPrimitive(primitive);
        
        for(size_t i = 0; i < ids.size(); ++i)
        {
          vo->addIdentifier(ids[i]);
        }
        
	objects.push_back(vo);
	objects_sorted.push_back(vo);
	return objects.size()-1;
}

void VisObjectFactory::updateObjectMatrix(int obj) {
	glGetFloatv(GL_MODELVIEW_MATRIX,(GLfloat*)objects[obj]->getMatrixP());
}

void VisObjectFactory::updateObjectColor(int obj,RGB_Color color) {
	objects[obj]->setColor(color);
}
