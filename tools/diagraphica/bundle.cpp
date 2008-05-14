//  Author(s): A.j. (Hannes) pretorius
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  LICENSE_1_0.txt)
//
/// \file ./bundle.cpp

// --- bundle.cpp ---------------------------------------------------
// (c) 2007  -  A.J. Pretorius  -  Eindhoven University of Technology
// ---------------------------  *  ----------------------------------


#include "bundle.h"


// -- constructors and destructors ----------------------------------


// -------------
Bundle::Bundle()
// -------------
{
    index = -1;
}


// -----------------------------
Bundle::Bundle( const int &idx )
// -----------------------------
{
    index      = idx;
    parent     = NULL;
	inCluster  = NULL;
	outCluster = NULL;
}


// --------------------------------
Bundle::Bundle( const Bundle &bdl )
// --------------------------------
{
    index      = bdl.index;
    parent     = bdl.parent;
    children   = bdl.children;
    inCluster  = bdl.inCluster;
	outCluster = bdl.outCluster;
    edges      = bdl.edges;
    labels     = bdl.labels;
}


// --------------
Bundle::~Bundle()
// --------------
{
    clearParent();
    clearChildren();
    clearInCluster();
    clearOutCluster();
    clearEdges();
}
	
  
// -- set functions -------------------------------------------------


// ------------------------------------
void Bundle::setIndex( const int &idx )
// ------------------------------------
{
    index = idx;
}


// --------------------------------
void Bundle::setParent( Bundle* p )
// --------------------------------
{
    parent = p;
}


// -------------------------------
void Bundle::addChild( Bundle* c )
// -------------------------------
{
    children.push_back( c );
}


// -------------------------------------
void Bundle::setInCluster( Cluster* in )
// -------------------------------------
{
    inCluster = in;
}


// ---------------------------------------
void Bundle::setOutCluster( Cluster* out )
// ---------------------------------------
{
    outCluster = out;
}


// ----------------------------
void Bundle::addEdge( Edge* e )
// ----------------------------
{
    edges.push_back( e );
    
    labels.insert( pair< string, string >( e->getLabel(), "MAY" ) );
}


// ----------------------------------------------
void Bundle::setEdges( const vector< Edge* > &e )
// ----------------------------------------------
{
    clearEdges();
    edges = e;

    labels.clear();
    for( size_t i = 0; i < edges.size(); ++i )
        labels.insert( pair< string, string >( edges[i]->getLabel(), "MAY" ) );
}
    

// -----------------------
void Bundle::updateLabel(
    const string &lbl,
    const string &status )
// -----------------------
{
    labels[lbl] = status;
}
    

// -- get functions -------------------------------------------------


// -------------------
int Bundle::getIndex()
// -------------------
{
    return index;
}
    

// ------------------------
Bundle* Bundle::getParent()
// ------------------------
{
    return parent;
}


// --------------------------
int Bundle::getSizeChildren()
// --------------------------
{
    return children.size();
}


// ---------------------------------------
Bundle* Bundle::getChild( const size_t &idx )
// ---------------------------------------
{
    Bundle* result = NULL;
    if ( 0 <= idx && idx < children.size() )
        result = children[idx];
    return result;
}
        

// ----------------------------
Cluster* Bundle::getInCluster()
// ----------------------------
{
    return inCluster;
}


// -----------------------------
Cluster* Bundle::getOutCluster()
// -----------------------------
{
    return outCluster;
}


// -----------------------
int Bundle::getSizeEdges()
// -----------------------
{
    return edges.size();
}


// ------------------------------------
Edge* Bundle::getEdge( const size_t &idx )
// ------------------------------------
{
    if ( 0 <= idx && idx < edges.size() )
        return edges[idx];
    else
        throw new string( "Error retrieving bundle edge." );

}


// ---------------------------------------------
void Bundle::getLabels( vector< string > &lbls )
// ---------------------------------------------
{
    lbls.clear();

    map< string, string >::iterator it;
    for( it = labels.begin(); it != labels.end(); ++it )
    {
        lbls.push_back( it->first );
        //status.push_back( it.second );
    }

}


// ---------------------------
void Bundle::getLabels( 
    vector< string > &lbls,
    vector< string > &status )
// ---------------------------
{
    lbls.clear();
    status.clear();

    map< string, string >::iterator it;
    for( it = labels.begin(); it != labels.end(); ++it )
    {
        lbls.push_back( it->first );
        status.push_back( it->second );
    }
}


// --------------------
void Bundle::getLabels( 
    string &separator,
    string &lbls )
// --------------------
{
    lbls = "";

    int size  = labels.size();
    int count = 0;
    
    map< string, string >::iterator it;
    for( it = labels.begin(); it != labels.end(); ++it )
    {
        count += 1;
        lbls  += it->first;
        lbls  += "[";
        lbls  += it->second;
        lbls  += "]";

        if ( count < size )
            lbls += separator;
    }
}

  
// -- clear functions -----------------------------------------------


// -----------------------
void Bundle::clearParent()
// -----------------------
{
    parent = NULL;
}


// -------------------------
void Bundle::clearChildren()
// -------------------------
{
    for ( size_t i = 0; i < children.size(); ++i )
        children[i] = NULL;
    children.clear();
}


// --------------------------
void Bundle::clearInCluster()
// --------------------------
{
    inCluster = NULL;
}
    

// ---------------------------
void Bundle::clearOutCluster()
// ---------------------------
{
    outCluster = NULL;
}


// ----------------------
void Bundle::clearEdges()
// ----------------------
{
    for ( size_t i = 0; i < edges.size(); ++i )
        edges[i] = NULL;
    edges.clear();

    labels.clear();
}


// -- end -----------------------------------------------------------
