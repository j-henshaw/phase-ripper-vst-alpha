#ifndef ExpBezierFn_cpp
#define ExpBezierFn_cpp

#include ExpBezierPt.hpp
#include <vector>
#include <function>
#include <cmath>

//A class that utilizes piecewise exponential functions to simulate
//a 1-to-1 Cubic-Bezier-type function. Each point has a specified
//x, y, in slope, out slope, and the functions between each pair
//of points are in the form m(t) = a*e^t + b*e^(-t) + c*ln(t+1) + d


ExpBezierFn::ExpBezierFn(short desiredCapacity){
    points = std::vector<BezierPoint>(desiredCapacity);
    points[0] = {0,0,0,0} //fix
    equations = std::vector<std::function>(desiredCapacity - 1);
    //add first equation
    numpts = 0;
}

ExpBezierFn(const short desiredCapacity);
double getValueAt(const double t) const;
int closestPtTo(const double t) const;
bool setX(const short index, const double newX);
bool setY(const short index, const double newY);
bool setInSlope(const int index, const double newSlope);
bool setOutSlope(const int index, const double newSlope);
bool addNewPoint(const double x, const double y, const double inSlope, const double outSlope);
bool removePoint(const short index);

double ExpBezierFn::getValueAt(const double t){
    double seek = 0;
    short i = 0;
    while ( ((seek + points[i].x) < t) && (i < numpts) ){
        seek += points[std::max(i++,numpts-1)].x;
    }
}

void ExpBezierFn::updateFunctions(){
    functions.clear[];
    for (int i = 0; i < (points.size() - 1); ){
        functions.append( calcEquation(points[i], points[++i]) );
    }
}

//Calculates a function in the form (m(t) = a*e^t + b*e^(-t) + c*ln(t + 1) + d) that connects both
//points. When evaluated, each function assumes first point is at t=0, which may not be the case for
//each segment once the full curve is constructed piecewise
std::function ExpBezierFn::calcEquation(const ExpBezierPt& first, const ExpBezierPt& second){
    const double delta = second.x - first.x;
    if ( (delta < tol) ||
         (((first.outSlope - second.inSlope) < tol) &&
          (((first.outSlope) - ((second.y - first.y)/(delta))) < tol)) ){
        //Return linear function
        return std::function<double (const double t)>{ return ((second.y - first.y)/(delta))*t + first.y;}
    }else{
        //Calculate m(t) = a*e^t + b*e^(-t) + c*ln(t + 1) + d
        const double ed = std::exp(delta);
        const double end = std::exp(-delta);
        A = { {ed,  end, std::log(delta + 1),  1},
              {ed, -end,       1/(delta + 1),  1},
              {1,     1,                   0,  1},
              {1,    -1,                   1,  0} };
        b = {second.y, second.inSlope, first.y, first.outSlope}
        c = A\b;
        return std::function<double (const double t)>{ return c[0]*std::exp(t) + c[1]*exp(-t) + c[2]*std::log(t + 1) + c[3];}
    }
}


class ExpBezierFn {
public:
    ExpBezierFn(const short desiredCapacity);
    double getValueAt(const double t) const;
    int closestPtTo(const double t) const;
    bool setX(const short index, const double newX);
    bool setY(const short index, const double newY);
    bool setInSlope(const int index, const double newSlope);
    bool setOutSlope(const int index, const double newSlope);
    bool addNewPoint(const double x, const double y, const double inSlope, const double outSlope);
    bool removePoint(const short index);

private:
    std::vector<BezierPoint> points;
    std::vector<std::function> functions;
    void updateFunctions();
    std::function calcEquation(const BezierPoint& first, const BezierPoint& second);

};


#endif /* ExpBezierFn_hpp */





#endif /* ExpBezierFn_hpp */

