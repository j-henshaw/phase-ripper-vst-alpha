#pragma once

struct ExpBezierPt {
    //Desired slope approaching this point from the left
    double inSlope;
    //Horizontal location of this point
    double x;
    //Vertical location of this point
    double y;
    //Desired slope leaving this point on the right
    double outSlope;
    

    //Overloading < operator to compare x values of points
    inline bool operator < (const ExpBezierPt& rhs) const {
        return x < rhs.x;
    }
    
    //Overloading <= operator to compare x values of points
    inline bool operator <= (const ExpBezierPt& rhs) const {
        return x <= rhs.x;
    }
    
    //Overloading > operator to compare x values of points
    inline bool operator > (const ExpBezierPt& rhs) const {
        return x > rhs.x;
    }
    
    //Overloading >= operator to compare x values of points
    inline bool operator > (const ExpBezierPt& rhs) const {
        return x >= rhs.x;
    }
    
    //Overloading == operator to compare x values of points
    inline bool operator > (const ExpBezierPt& rhs) const {
        return x == rhs.x;
    }
    
};


