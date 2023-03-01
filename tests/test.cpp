#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "../ext/catch.hpp"
#include "../source/HarmonicFlipper.cpp"

TEST_CASE("Test the tests -- should_pass", "[test_tag]") {
    REQUIRE(true);
}

TEST_CASE("Test the tests -- should_fail", "[test_tag]") {
    REQUIRE(false);
}

//########################//
// HARMONIC FLIPPER TESTS //
//########################//
TEST_CASE("Running various pairs through various flippers w various notes", "[HarmonicFlipper]"){
    HarmonicFlipper testFlipper;
    NoteInfo testInfo;
    double a, b, c, d;
    std::pair<double&, double&> test1 = {a, b};
    std::pair<double&, double&> test2 = {c, d};
    
    SECTION("Verify harmonics can/can't be set properly"){
        ///TRYING TO ADD HARMONICS OUT OF RANGE
        REQUIRE_THROWS(testFlipper.setStatus(0,HarmonicFlipper::Active));
        REQUIRE_THROWS(testFlipper.setStatus(-20,HarmonicFlipper::Active));
        REQUIRE_THROWS(testFlipper.setStatus(20,HarmonicFlipper::Active));
        REQUIRE_THROWS(testFlipper.setStatus(11,HarmonicFlipper::Inactive));
        REQUIRE_THROWS(testFlipper.setStatus(-9,HarmonicFlipper::Inactive));
        
        ///CHECK ALL VALID ONES TOO, state they already have
        for(int i = -8; i <= 10; ++i){
            if (i != 0){
                REQUIRE_NOTHROW(testFlipper.setStatus(i,HarmonicFlipper::Inactive));
            }
        }
        //Check inverted
        for(int i = -8; i <= 10; ++i){
            if (i != 0){
                REQUIRE_NOTHROW(testFlipper.setStatus(i,HarmonicFlipper::Inverted));
            }
        }
        //Check regular
        for(int i = -8; i <= 10; ++i){
            if (i != 0){
                REQUIRE_NOTHROW(testFlipper.setStatus(i,HarmonicFlipper::Active));
            }
        }
        //Deactivate
        for(int i = -8; i <= 10; ++i){
            if (i != 0){
                REQUIRE_NOTHROW(testFlipper.setStatus(i,HarmonicFlipper::Inactive));
            }
        }
    }
    
    
    
    SECTION("Initialized array should never flip (+,-)"){
        a = c = 0.9099434;
        b = d = -0.22822;
        
        testFlipper.process(test1, testInfo);
        REQUIRE(test1 == test2);
        REQUIRE(a == c);
        REQUIRE(b == d);
    }
    SECTION("Initialized array should never flip (+,+)"){
        a = c = 0.440443;
        b = d = 0.559;
        testFlipper.process(test1, testInfo);
        REQUIRE(test1 == test2);
        REQUIRE(a == c);
        REQUIRE(b == d);
    }
    SECTION("Initialized array should never flip (-,-)"){
        a = c = -1.0;
        b = d = -0.01;
        testFlipper.process(test1, testInfo);
        REQUIRE(test1 == test2);
        REQUIRE(a == c);
        REQUIRE(b == d);
    }
    SECTION("Initialized array should never flip (-,+)"){
        a = c = -0.76778787744044;
        b = d = -0.680903;
        testFlipper.process(test1, testInfo);
        REQUIRE(test1 == test2);
        REQUIRE(a == c);
        REQUIRE(b == d);
    }
    SECTION("Initialized array should never flip (0,0)"){
        a = c = 0;
        b = d = 0;
        testFlipper.process(test1, testInfo);
        REQUIRE(test1 == test2);
        REQUIRE(a == c);
        REQUIRE(b == d);
    }

    ///ADDING A PITCH to the NoteInfo
    testInfo.fundamental = 440;
    SECTION("Initialized array should never flip, #2", "[test_tag]"){
        a = c = 0;
        b = d = 1;
        testFlipper.process(test1, testInfo);
        REQUIRE(test1 == test2);
        REQUIRE(a == c);
        REQUIRE(b == d);
    }

    testInfo.time = 1.1/440;
    SECTION("Initialized array should never flip, #3", "[test_tag]"){
        a = c = 1;
        b = d = 0;
        testFlipper.process(test1, testInfo);
        REQUIRE(test1 == test2);
        REQUIRE(a == c);
        REQUIRE(b == d);
    }

    //Activate the fundamental flipper
    testFlipper.setStatus(1, HarmonicFlipper::Active);
    SECTION("Activate the fundamental flipper", "[test_tag]"){
        a = c = -1;
        b = d = 0;
        testFlipper.process(test1, testInfo);
        REQUIRE(test1 != test2);
        REQUIRE(a == -c);
        REQUIRE(b == -d);
    }

    //Invert the fundamental flipper
    testFlipper.setStatus(1, HarmonicFlipper::Inverted);
    SECTION("Invert the fundamental flipper", "[test_tag]"){
        a = c = 0;
        b = d = -1;
        testFlipper.process(test1, testInfo);
        REQUIRE(test1 == test2);
        REQUIRE(a == c);
        REQUIRE(b == d);
    }

    //Invert the fundamental flipper and use even period
    testInfo.time = 0.9/440;
    SECTION("Invert the fundamental flipper and use even period", "[test_tag]"){
        a = c = 1;
        b = d = -1;
        testFlipper.process(test1, testInfo);
        REQUIRE(test1 != test2);
        REQUIRE(a == -c);
        REQUIRE(b == -d);
    }

    //Reset the fundamental flipper and use even period
    testFlipper.setStatus(1, HarmonicFlipper::Active);
    SECTION("Reset the fundamental flipper and use even period", "[test_tag]"){
        a = c = -0.9099434;
        b = d = -0.22822;
        testFlipper.process(test1, testInfo);
        REQUIRE(test1 == test2);
        REQUIRE(a == c);
        REQUIRE(b == d);
    }

    ///TESTING SUBHARMONICS
    testFlipper.setStatus(1, HarmonicFlipper::Inactive);
    testFlipper.setStatus(-1, HarmonicFlipper::Active);
    testInfo.time = .1/440;
    SECTION("Test even period subharmonics", "[test_tag]"){
        a = c = -0.9099434;
        b = d = 0.22822;
        testFlipper.process(test1, testInfo);
        REQUIRE(test1 == test2);
        REQUIRE(a == c);
        REQUIRE(b == d);
    }

    testInfo.time = 3.1/440;
    SECTION("Test odd period subharmonics", "[test_tag]"){
        a = c = 0.9099434;
        b = d = 0.22822;
        testFlipper.process(test1, testInfo);
        REQUIRE(test1 != test2);
        REQUIRE(a == -c);
        REQUIRE(b == -d);
    }


    ///TESTING CONFLICTING HARMONICS
    testFlipper.setStatus(1, HarmonicFlipper::Active);
    testInfo.time = .1/440;
    SECTION("Test conflicting harmonics, #1", "[test_tag]"){
        a = c = -0.9099434;
        b = d = 0.22822;
        testFlipper.process(test1, testInfo);
        REQUIRE(test1 == test2);
        REQUIRE(a == c);
        REQUIRE(b == d);
    }

    testInfo.time = 1.1/440;
    SECTION("Test conflicting harmonics, #2", "[test_tag]"){
        a = c = -0.9099434;
        b = d = 0.22822;
        testFlipper.process(test1, testInfo);
        REQUIRE(test1 != test2);
        REQUIRE(a == -c);
        REQUIRE(b == -d);
    }

    testInfo.time = 2.1/440;
    SECTION("Test conflicting harmonics, #3", "[test_tag]"){
        a = c = -0.9099434;
        b = d = 0.22822;
        testFlipper.process(test1, testInfo);
        REQUIRE(test1 != test2);
        REQUIRE(a == -c);
        REQUIRE(b == -d);
    }

    testInfo.time = 3.1/440;
    SECTION("Test conflicting harmonics, #4", "[test_tag]"){
        a = c = -0.9099434;
        b = d = 0.22822;
        testFlipper.process(test1, testInfo);
        REQUIRE(test1 == test2);
        REQUIRE(a == c);
        REQUIRE(b == d);
    }

    //Testing conflicting harmonics with pitchbend
    //Bend is making the pitch lower, so a little slower
    testInfo.pitchbend = 1-.0000001;
    testInfo.time = 1.0/440;
    SECTION("With pitchbend, A440 should not have flipped yet", "[test_tag]"){
        a = c = -0.9099434;
        b = d = 0.22822;
        testFlipper.process(test1, testInfo);
        REQUIRE(test1 == test2);
        REQUIRE(a == c);
        REQUIRE(b == d);
    }

    testInfo.time = 2.0/440;
    SECTION("Test conflicting harmonics w/pitchbend, #2: after 2/440 time, the fundamental should not have flipped back yet, and the subharmonic should not have flipped for the first time", "[test_tag]"){
        std::cout << testInfo.time << std::endl;
        a = c = -0.9099434;
        b = d = 0.22822;
        testFlipper.process(test1, testInfo);
        REQUIRE(test1 != test2);
        REQUIRE(a == -c);
        REQUIRE(b == -d);
    }

    testInfo.time = 3.0/440;
    SECTION("Test conflicting harmonics w/pitchbend, #3", "[test_tag]"){
        a = c = 0.9099434;
        b = d = -0.22822;
        testFlipper.process(test1, testInfo);
        REQUIRE(test1 != test2);
        REQUIRE(a == -c);
        REQUIRE(b == -d);
    }
    
    SECTION("Test conflicting harmonics w/pitchbend, #3.1", "[test_tag]"){
        a = c = 0;
        b = d = 0;
        testFlipper.process(test1, testInfo);
        REQUIRE(test1 == test2);
        REQUIRE(a == -c);
        REQUIRE(b == -d);
    }
    SECTION("Test conflicting harmonics w/pitchbend, #3.2", "[test_tag]"){
        a = c = 1;
        b = d = 0;
        testFlipper.process(test1, testInfo);
        REQUIRE(test1 != test2);
        REQUIRE(a == -c);
        REQUIRE(b == d);
    }
    SECTION("Test conflicting harmonics w/pitchbend, #3.3", "[test_tag]"){
        a = c = 1;
        b = d = 1;
        testFlipper.process(test1, testInfo);
        REQUIRE(test1 != test2);
        REQUIRE(a == -c);
        REQUIRE(b == -d);
    }
    SECTION("Test conflicting harmonics w/pitchbend, #3.4", "[test_tag]"){
        a = c = 0;
        b = d = 1;
        testFlipper.process(test1, testInfo);
        REQUIRE(test1 != test2);
        REQUIRE(a == c);
        REQUIRE(b == -d);
    }
    SECTION("Test conflicting harmonics w/pitchbend, #3.5", "[test_tag]"){
        a = c = -1;
        b = d = 0;
        testFlipper.process(test1, testInfo);
        REQUIRE(test1 != test2);
        REQUIRE(a == -c);
        REQUIRE(b == d);
    }
    SECTION("Test conflicting harmonics w/pitchbend, #3.6", "[test_tag]"){
        a = c = -1;
        b = d = 1;
        testFlipper.process(test1, testInfo);
        REQUIRE(test1 != test2);
        REQUIRE(a == -c);
        REQUIRE(b == -d);
    }
    SECTION("Test conflicting harmonics w/pitchbend, #3.7", "[test_tag]"){
        a = c = -1;
        b = d = -1;
        testFlipper.process(test1, testInfo);
        REQUIRE(test1 != test2);
        REQUIRE(a == -c);
        REQUIRE(b == -d);
    }
    SECTION("Test conflicting harmonics w/pitchbend, #3.8", "[test_tag]"){
        a = c = 0;
        b = d = -1;
        testFlipper.process(test1, testInfo);
        REQUIRE(test1 != test2);
        REQUIRE(a == c);
        REQUIRE(b == -d);
    }

    testInfo.time = 4.0/440;
    SECTION("Test conflicting harmonics w/pitchbend, #4", "[test_tag]"){
        a = c = -0.9099434;
        b = d = -0.22822;
        testFlipper.process(test1, testInfo);
        REQUIRE(test1 == test2);
        REQUIRE(a == c);
        REQUIRE(b == d);
    }
    
}

