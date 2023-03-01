#ifndef CubicBezierFunction_hpp
#define CubicBezierFunction_hpp

#include <vector>
#include <function>

//A class that utilizes piecewise cubic polynomials to simulate a
//1-to-1 Cubic-Bezier-type function. Each point has a specified
//x, y, in slope, out slope

class CubicBezierFunction {
public:
    CubicBezierFunction(short desiredCapacity);
    double getValueAt(double x) const;
    int indexOfPointNear(double x) const;
    bool setX(short index, double newX);
    bool setY(short index, double newY);
    bool setIncomingSlope(int index, double newSlope);
    bool setOutgoingSlope(int index, double newSlope);
    bool addNewPoint(double x, double y, double inSlope, double outSlope);
    bool removePoint(short index);

private:
    std::vector<BezierPoint> points;
    std::vector<std::function> functions;
    void updateFunctions();
    std::function calcEquation(const CubicBezierPoint& first, const CubicBezierPoint& second);

};


#endif /* CubicBezierFunction_hpp */

