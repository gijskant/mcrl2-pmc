// Author(s): Bas Ploeger and Carst Tankink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file state.cpp
/// \brief Source file for State class

#include "state.h"
#include "transition.h"

using namespace std;
using namespace Utils;

State::State(int aid) {
  cluster = NULL;
  rank = 0;
  positionRadius = 0.0f;
  positionAngle = -1.0f;
  id = aid;
  simulated = false;
  selected = false;
  zoomLevel = 0;
}

State::~State() {
  unsigned int i;
  for (i = 0; i < outTransitions.size(); ++i) {
    delete outTransitions[i];
  }
  for (i = 0; i < loops.size(); ++i) {
    delete loops[i];
  }
}

void State::addInTransition(Transition* trans) {
  inTransitions.push_back(trans);
}

void State::addOutTransition(Transition* trans) {
  outTransitions.push_back(trans);
}

void State::addLoop(Transition* trans) {
  loops.push_back(trans);
}

void State::addMatchedRule(int mr) {
  matchedRules.insert(mr);
}

//returns true iff an element has actually been removed
bool State::removeMatchedRule(int mr) {
  return (matchedRules.erase(mr) > 0);
}

void State::getMatchedRules(std::vector< int > &mrs) {
  mrs.assign(matchedRules.begin(),matchedRules.end());
}

int State::getNumMatchedRules() {
  return matchedRules.size();
}

bool State::isDeadlock() const {
  return (outTransitions.size() + loops.size() == 0);
}

bool State::isSelected() const {
  return selected;
}

void State::select() {
  selected = true;
}

void State::deselect() {
  selected = false;
}

int State::getID() {
	return id;
}

void State::setID(int i) {
	id = i;
}

int State::getRank() const {
  return rank;
}

void State::setRank(int r) {
  rank = r;
}

bool State::isCentered() const {
  return (positionAngle < -0.9f);
}

void State::center() {
  positionRadius = 0.0f;
  positionAngle = -1.0f;
}

float State::getPositionAngle() const {
  return positionAngle;
}

float State::getPositionRadius() const {
  return positionRadius;
}

Point3D State::getPositionAbs() const {
  return positionAbs;
}

Point3D State::getOutgoingControl() const {
  return outgoingControl;
}

Point3D State::getIncomingControl() const {
  return incomingControl;
}

Point3D State::getLoopControl1() const
{
  return loopControl1;
}

Point3D State::getLoopControl2() const
{
  return loopControl2;
}

void State::setPositionRadius(float r) {
  positionRadius = r;
}

void State::setPositionAngle(float a) {
  positionAngle = a;
}

void State::setPositionAbs(Point3D &p) {
  positionAbs.x = p.x;
  positionAbs.y = p.y;
  positionAbs.z = p.z;
}

void State::setOutgoingControl(Point3D &p) {
  outgoingControl = p;
}

void State::setIncomingControl(Point3D &p) {
  incomingControl = p;
}

void State::setLoopControl1(Point3D &p)
{
  loopControl1 = p;
}

void State::setLoopControl2(Point3D &p)
{
  loopControl2 = p;
}


Cluster* State::getCluster() const {
  return cluster;
}

void State::setCluster(Cluster* c) {
  cluster = c;
}

Transition* State::getInTransition(int i) const {
  return inTransitions[i];
}

int State::getNumInTransitions() const {
  return inTransitions.size();
}

Transition* State::getOutTransition(int i) const {
  return outTransitions[i];
}

int State::getNumOutTransitions() const {
  return outTransitions.size();
}

Transition* State::getLoop(int i) const {
  return loops[i];
}

int State::getNumLoops() const {
  return loops.size();
}

void State::setSimulated(bool simulated) {
  this->simulated = simulated;
}

bool State::isSimulated() const {
  return simulated;
}

int State::getZoomLevel() const {
  return zoomLevel;
}


void State::setZoomLevel(const int level) {
  zoomLevel = level;
}

Point3D State::getForce() {
  return force;
}

void State::resetForce() {
  force.x = 0.0f;
  force.y = 0.0f;
  force.z = 0.0f;
}

void State::addForce(Point3D f) {
  force.x += f.x;
  force.y += f.y;
  force.z += f.z;
}

Vect State::getVelocity() {
  return velocity;
}

void State::resetVelocity() {
  velocity.x = 0.0f;
  velocity.y = 0.0f;
}

void State::setVelocity(Vect v) {
  velocity = v;
}
