#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <gpiod.hpp>
#include <iostream>
#include <unistd.h>
#include <thread>
#include <cstdlib>
#include <filesystem>


int main(int argc, char * argv[]) {

    struct gpiod_chip *chip =  gpiod_chip_open_by_name("gpiochip0");
    struct gpiod_line *line = gpiod_chip_get_line(chip, 21);;
    struct gpiod_line *line1 = gpiod_chip_get_line(chip, 20);;

    int req, value;

    req = gpiod_line_request_input(line, "gpio_state");
    value = gpiod_line_get_value(line);
    printf("GPIO value is: %d\n", value);

    gpiod_line_request_output(line1, "example", 1);
    gpiod_line_set_value(line1, 1);
    printf("GPIO value set. \n");


    gpiod_line_release(line1);
    gpiod_line_release(line);
    gpiod_chip_close(chip);



   return 0;
}
