// --- utils.h ------------------------------------------------------
// (c) 2006  -  A.J. Pretorius  -  Eindhoven University of Technology
// ---------------------------  *  ----------------------------------

#ifndef UTILS_H
#define UTILS_H

#include <cstddef>
#include <cstdlib>
#include <cmath>
#include <sstream>
#include <string>
using namespace std;

const double PI = 3.14159265;
const double E  = 2.71828183;

class Utils
{
public:
    // -- string conversion functions -------------------------------
    static string dblToStr( const double &d );
    static string intToStr( const int &i );
    static double strToDbl( const string &s );
    static int strToInt( const string &s );
    
    // -- trig functions --------------------------------------------
    static double radToDegr( const double &r );
    static double degrToRad( const double &d );

    static double calcAngleDg( 
        const double &x, 
        const double &y );
    static double calcAngleRd( 
        const double &x, 
        const double &y );

    // -- math functions --------------------------------------------
    static int rndToInt( const double &f );
    static double rndToNearestMult( 
        const double &value,
        const double &factor );
    
    static double dist( 
        const double &x1, const double &y1,
        const double &x2, const double &y2 );
    static double abs( const double &val );
	static double maxx(
        const double &d0, 
        const double &d1 );
    static double minn(
        const double &d0,
        const double &d1 );
    
    static double perc(
        const int &numr,
        const int &denm );
    static double perc(
        const double &numr,
        const double &denm );

    static double fishEye( 
        const double &distortion,
        const double &value );
};

#endif

// -- end -----------------------------------------------------------
