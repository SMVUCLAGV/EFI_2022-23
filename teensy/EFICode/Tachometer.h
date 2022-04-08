#ifndef TACHOMETER_H
#define TACHOMETER_H

class Tachometer {
    public:
        void countRevolution();
        void updateRPM();
    private:
        int revolutions; //misc
        unsigned long totalRevolutions; //misc
        unsigned long startingRevolutions; //misc
        unsigned long previousRev; //misc
};

#endif