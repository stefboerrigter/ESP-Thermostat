#pragma once


class Display {
    public:
        Display();
        ~Display();
        void process(float temperature, float setPoint, float humidity, float reltemp);
        void initialize();
    private:
        void testdrawline();
};