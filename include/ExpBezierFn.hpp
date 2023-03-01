#ifndef CubicBezierFunction_hpp
#define CubicBezierFunction_hpp

#include <vector>
#include <function>

//A class that utilizes piecewise exponential functions to simulate
//a 1-to-1 Cubic-Bezier-type function. Each point has a specified
//x, y, in slope, out slope, and the functions between each pair
//of points are in the form m(t) = a*e^t + b*e^(-t) + c*ln(t+1) + d

class ExpBezierFn {
public:
    ExpBezierFn(const short desiredCapacity);
    double getValueAt(const double t) const;
    short closestPtTo(const double t) const;
    bool setX(const short index, const double newX);
    bool setY(const short index, const double newY);
    bool setInSlope(const int index, const double newSlope);
    bool setOutSlope(const int index, const double newSlope);
    bool addNewPoint(const double x, const double y, const double inSlope, const double outSlope);
    bool removePoint(const short index);

private:
    const double tol = 1e-12;
    std::vector<BezierPoint> points;
    std::vector<std::function> functions;
    short numpts;
    void updateFunctions();
    std::function calcEquation(const BezierPoint& first, const BezierPoint& second);

};


#endif /* CubicBezierFunction_hpp */

