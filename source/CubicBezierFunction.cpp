#ifndef CubicBezierFunction_cpp
#define CubicBezierFunction_cpp

#include ExpBezierPt.hpp
#include <vector>
#include <function>

//A class that utilizes cubic polynomials to simulate a
//1-to-1 Cubic-Bezier-type function


CubicBezierFunction::CubicBezierFunction(short desiredCapacity){
    points = std::vector<BezierPoint>(desiredCapacity);
    equations = std::vector<std::function>(desiredCapacity - 1);
}

int CubicBezierFunction::indexOfPointNear(double x) const;
bool CubicBezierFunction::setX(short index, double newX);
bool CubicBezierFunction::setY(short index, double newY);
bool CubicBezierFunction::setIncomingSlope(int index, double newSlope);
bool CubicBezierFunction::setOutgoingSlope(int index, double newSlope);
bool CubicBezierFunction::setIncomingExtension(int index, double newLxt);
bool CubicBezierFunction::setOutgoingExtension(int index, double newRxt);
bool CubicBezierFunction::addNewPoint(double x, double y, double inSlope, double outSlope);
bool CubicBezierFunction::removePoint(short index);

double CubicBezierFunction::getValueAt(double x){
    int segment = -1;
    for (int i = 0; i < points.size(); ++i){
        if ( x > points[i] ){
            ++segment;
        }else{ i = points.size(); }
    }
}

void CubicBezierFunction::updateFunctions(){
    functions.clear[];
    for (int i = 0; i < (points.size() - 1); ){
        functions.append( calcEquation(points[i], points[++i]) );
    }
}


std::function CubicBezierFunction::calcEquation(const ExpBezierPt& first, const ExpBezierPt& second){
    //Used Mathematica to solve for equations for a,b,c,d such that there is a polynomial function
    //of the form y = ax^3 + bx^2 + cx + d where f(first.x) = first.y, f(second.x) = second.y,
    //f'(first.x) = first.outSlope, and f'(second.x) = second.inSlope
    const double denom = (second.x - first.x) * (second.x - first.x) * (second.x - first.x);
    
    const double a = ( 2*(first.y - second.y) - (second.inSlope + first.outSlope) * (first.x - second.x) )
                    / ( denom );
    const double b = ( second.x * second.x * (2 * first.outSlope + second.inSlope)
                      + first.x * first.x * (0 - first.outSlope - 2 * second.inSlope)
                      + 3 * (second.y - first.y) * (second.x + first.x)
                      + second.x * second.y * (second.inSlope - first.outSlope) )
                    / ( denom );
    const double c = ( second.x * second.x * second.x * first.inSlope
                      - second.inSlope * first.x * first.x * first.x
                      + second.x * second.x * first.x * (2 * second.inSlope + first.outSlope)
                      - first.x * second.x * (6 * second.y + second.inSlope * first.x - 6 * first.y + 2 * first.x * first.outSlope) )
                    / ( denom );
    const double d = (  second.x * second.x * first.x * first.x * (3 * second.y + second.inSlope * first.x)
                      - second.y * first.x * first.x * first.x
                      + second.x * second.x * second.x * (first.y - first.x * first.outSlope)
                      + second.x * second.x * first.x * (first.x * first.outSlope - second.inSlope * first.x - 3 * first.y) )
                    / (denom);
    
    return std::function<double (const double xCoord)>{ return (((a * xCoord) + b) * xCoord + c) * xCoord + d ;}
    
}





#endif /* CubicBezierFunction_hpp */

