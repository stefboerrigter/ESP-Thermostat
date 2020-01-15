#pragma once


class Display {
    public:
        Display();
        ~Display();
        void process(float temperature, float setPoint);
        void initialize();
    private:
        void testdrawline();
};