#pragma once

#include <uuid/common.h>
#include <uuid/console.h>


#include <uuid/console.h>
#include <uuid/log.h>

#include "helpers.h"
#include "system.h"
#include "mqtt.h"

using uuid::flash_string_vector;
using uuid::read_flash_string;
using uuid::console::Commands;
using uuid::console::Shell;
using uuid::log::Level;

namespace emsesp {

class Display {
    public:
        Display();
        ~Display();
        void process(float temperature, float setPoint, float humidity, float reltemp);
        void initialize();
    private:
        void testdrawline();
        static uuid::log::Logger logger_;
};
} //namespace