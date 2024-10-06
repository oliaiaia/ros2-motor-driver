//
// Created by Olia on 20.09.2024.
//

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

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "robot_interfaces/msg/num.hpp"                                            //
#include "robot_interfaces/msg/motor_data.hpp"

using std::placeholders::_1;
using namespace std::chrono_literals;

class Driver {
public:

    Driver(unsigned int raspi_number_pin_IN1, unsigned int raspi_number_pin_IN2, unsigned int raspi_number_pin_IN3, unsigned int raspi_number_pin_IN4) {
        this->raspi_number_pin_IN1 = raspi_number_pin_IN1;
        this->raspi_number_pin_IN2 = raspi_number_pin_IN2;
        this->raspi_number_pin_IN3 = raspi_number_pin_IN3;
        this->raspi_number_pin_IN4 = raspi_number_pin_IN4;

        pin_in1 = gpiod_chip_get_line(chip, raspi_number_pin_IN1);
        pin_in2 = gpiod_chip_get_line(chip, raspi_number_pin_IN2);
        pin_in3 = gpiod_chip_get_line(chip, raspi_number_pin_IN3);
        pin_in4 = gpiod_chip_get_line(chip, raspi_number_pin_IN4);

        /*      if (!pin_in1 || !pin_in2 || !pin_in3 || !pin_in4) {
                  perror("gpiod_chip_get_line");
                  std::cout << "gpiod_chip_get_line ERROR";
                  gpiod_chip_close(chip);
              }*/
    }

    void init_ () {
        gpiod_line_request_output(pin_in1, "pin_in1", 0);
        gpiod_line_request_output(pin_in2, "pin_in2", 0);
        gpiod_line_request_output(pin_in3, "pin_in3", 0);
        gpiod_line_request_output(pin_in4, "pin_in4", 0);

        /*if (gpiod_line_request_output(pin_in1, "pin_in1", 0) < 0 || gpiod_line_request_output(pin_in2, "pin_in2", 0) < 0
            || gpiod_line_request_output(pin_in3, "pin_in3", 0) < 0 ||
            gpiod_line_request_output(pin_in4, "pin_in4", 0) < 0) {
            perror("gpiod_line_request_output");
            std::cout << "gpiod_line_request_output ERROR";
            gpiod_chip_close(chip);
        }*/

    }

    void close_all_pins() {

        gpiod_line_release(pin_in1);
        gpiod_line_release(pin_in2);
        gpiod_line_release(pin_in3);
        gpiod_line_release(pin_in4);
        gpiod_chip_close(chip);
    }

    void get_command(int motor_number, char state) {
        std::cout << "get_comand";
        gpiod_line *line1;
        gpiod_line *line2;
        if (motor_number == 1) {
            line1 = pin_in1;
            line2 = pin_in2;
        } else {
            line1 = pin_in3;
            line2 = pin_in4;
        }
        switch (state) {
            case 'F':
                set_pins_direct1(line1, line2);
                break;
            case 'S':
                set_pins_stop(line1, line2);
                break;
            case 'B':
                set_pins_direct2(line1, line2);
                break;
        }
    }

private:

    void set_pins_direct1(gpiod_line *line1, gpiod_line *line2) {

        if (gpiod_line_set_value(line1, 1) < 0 || gpiod_line_set_value(line2, 0) < 0) {
            std::cout << "Error with line";
            perror("gpiod_line_set_value");
            gpiod_line_release(line1);
            gpiod_line_release(line2);
            gpiod_chip_close(chip);
        }

    }

    void set_pins_direct2(gpiod_line *line1, gpiod_line *line2) {

        if (gpiod_line_set_value(line1, 0) < 0 || gpiod_line_set_value(line2, 1) < 0) {
            std::cout << "Error with line";
            perror("gpiod_line_set_value");
            gpiod_line_release(line1);
            gpiod_line_release(line2);
            gpiod_chip_close(chip);
        }
    }

    void set_pins_stop(gpiod_line *line1, gpiod_line *line2) {

        if (gpiod_line_set_value(line1, 0) < 0 || gpiod_line_set_value(line2, 0) < 0) {
            std::cout << "Error with line";
            perror("gpiod_line_set_value");
            gpiod_line_release(line1);
            gpiod_line_release(line2);
            gpiod_chip_close(chip);
        }
    }

    gpiod_line *pin_in1;
    gpiod_line *pin_in2;
    gpiod_line *pin_in3;
    gpiod_line *pin_in4;
    const char *chipname = "gpiochip0";
    struct gpiod_chip *chip = gpiod_chip_open_by_name(chipname);

    unsigned int raspi_number_pin_IN1;
    unsigned int raspi_number_pin_IN2;
    unsigned int raspi_number_pin_IN3;
    unsigned int raspi_number_pin_IN4;

    //добавить для регулировки скорости + добавить конструктор
//    unsigned int raspi_number_pin_ENA;
//    unsigned int raspi_number_pin_ENB;
//    struct gpiod_line *pin_ena;
//    struct gpiod_line *pin_enb;

};


class Motor : public rclcpp::Node
{
public:
    Driver driver = Driver(20, 21, 12, 16);
    Driver driver2 = Driver(6, 13, 19, 26);


    Motor()
            : Node("motor")
    {
        publisher_ = this->create_publisher<std_msgs::msg::String>("motor_logs", 10);
        motor_publisher_ = this->create_publisher<robot_interfaces::msg::MotorData>("motor", 10);
        timer_ = this->create_wall_timer(2000ms, std::bind(&Motor::timer_callback, this));
        motor_subscription_ = this->create_subscription<robot_interfaces::msg::MotorData>(
                "motor_direction", 10, std::bind(&Motor::topic_callback, this, _1));
        timer_driver = this->create_wall_timer(1000ms, std::bind(&Motor::driver_timer, this));

        // driver.close_all_pins();

        // driver = Driver(20, 21, 12, 16);
        init_();

        auto message=std_msgs::msg::String();
        message.data = "Init sucsess!";
        std::cout << message.data;
    }


    void go_like_car_with_controller() {
        std::cout << "Введите номер мотора 1 или 2 и команду 'F' 'S' 'B': ";
        std::cin >> motor_num_control_cmd_1;
        std::cin >> motor_command_control_cmd_1;
        std::cout << "Введите номер мотора 1 или 2 и команду 'F' 'S' 'B': ";
        std::cin >> motor_num_control_cmd_2;
        std::cin >> motor_command_control_cmd_2;


        std::cout << "Введите номер мотора 1 (3) или 2 (4) и команду 'F' 'S' 'B': ";
        std::cin >> motor_num_control_cmd_3;
        std::cin >> motor_command_control_cmd_3;
        std::cout << "Введите номер мотора 1 (3) или 2 (4) и команду 'F' 'S' 'B': ";
        std::cin >> motor_num_control_cmd_4;
        std::cin >> motor_command_control_cmd_4;


        driver.get_command(motor_num_control_cmd_1, motor_command_control_cmd_1);
        driver.get_command(motor_num_control_cmd_2, motor_command_control_cmd_2);
        driver2.get_command(motor_num_control_cmd_3, motor_command_control_cmd_3);
        driver2.get_command(motor_num_control_cmd_4, motor_command_control_cmd_4);
    }


    void go_forward() {
        driver.get_command(1, 'B');
	driver.get_command(2, 'B');
	
        driver2.get_command(1, 'B');
	driver2.get_command(2, 'B');
    }
    void go_backward() {
        driver.get_command(1, 'F');
	driver.get_command(2, 'F');
	
        driver2.get_command(1, 'F');
	driver2.get_command(2, 'F');
	}
    void go_left() {
        driver.get_command(1, 'F');
	driver.get_command(2, 'F');
	
        driver2.get_command(1, 'B');
	driver2.get_command(2, 'B');
	}
    void go_right() {
        driver2.get_command(1, 'F');
	driver2.get_command(2, 'F');
	
        driver.get_command(1, 'B');
	driver.get_command(2, 'B');
	}
    void left_forward_diagonal() {
	driver.get_command(1, 'B');
	driver.get_command(2, 'B');
	
        driver2.get_command(1, 'S');
	driver2.get_command(2, 'S');
	}
    void left_backward_diagonal() {
	driver.get_command(1, 'B');
	driver.get_command(2, 'B');
	
        driver2.get_command(1, 'S');
	driver2.get_command(2, 'S');
	}
    void right_backward_diagonal() {
	driver.get_command(1, 'S');
	driver.get_command(2, 'S');
	
        driver2.get_command(1, 'F');
	driver2.get_command(2, 'F');
	}
    void right_forward_diagonal() {
	driver.get_command(1, 'S');
	driver.get_command(2, 'S');
	
        driver2.get_command(1, 'B');
	driver2.get_command(2, 'B');
	}
    void turn_left() {
	driver.get_command(1, 'F');
	driver.get_command(2, 'B');
	
        driver2.get_command(1, 'F');
	driver2.get_command(2, 'B');
	}
    void turn_right() {
	driver.get_command(2, 'F');
	driver.get_command(1, 'B');
	
        driver2.get_command(2, 'F');
	driver2.get_command(1, 'B');
	}
 
    void stop() {
        driver.get_command(1,'S');
        driver.get_command(2, 'S');

        driver2.get_command(1,'S');
        driver2.get_command(2, 'S');
    }
    
    int choose_command() {
   	std::cout << "Куда: f (go_forward), b (go_backward), l (go_left), r (go_right), fl (left_forward_diagonal), fr (right_forward_diagonal), bl (left_backward_diagonal), br (right_backward_diagonal), tl (turn_left), tr (turn_right), s (stop)" << std::endl;
    	std::string direction; 
    	std::cin >> direction;
    	if(direction.compare("f") == 0) {
    		go_forward();
    		return 0;
    	}
    	if(direction.compare("b") == 0) {
    		go_backward();
    		return 0;
    	}
    	if(direction.compare("l") == 0) {
    		go_left();
    		return 0;
    	}
    	if(direction.compare( "r") == 0) {
    		go_right();
    		return 0;
    	}
    	if(direction.compare( "fl") == 0) {
    		left_forward_diagonal();
    		return 0;
    	}
    	if(direction.compare( "fl") == 0) {
    		left_forward_diagonal();
    		return 0;
    	}
    	if(direction.compare( "fr") == 0) {
    		right_forward_diagonal();
    		return 0;
    	}
    	if(direction.compare("bl") == 0) {
    		left_backward_diagonal();
    		return 0;
    	}
    	if(direction.compare("br") == 0) {
    		right_backward_diagonal();
    		return 0;
    	}
    	if(direction.compare("tl") == 0) {
    		turn_left();
    		return 0;
    	}
    	if(direction.compare("tr") == 0) {
    		turn_right();
    		return 0;
    	}
    	if(direction.compare("s") == 0) {
    		stop();
    		return 0;
    	}
    	return 0;	
    	
    }
    
    void get_motor_command_from_suscr(char state1, char state2, char state3, char state4) {
    	motor_command_control_cmd_1 = state1;
        motor_command_control_cmd_2 = state2;
        driver.get_command(motor_num_control_cmd_1, motor_command_control_cmd_1);
        driver.get_command(motor_num_control_cmd_2, motor_command_control_cmd_2);
    }


    ~Motor(){
    	stop();
        driver.close_all_pins();
        driver2.close_all_pins();
        std::cout << "Close pins";
    }

private:

    void init_() {
        driver.init_();
        driver2.init_();
    }

    void topic_callback(const robot_interfaces::msg::MotorData &msg) const {
        RCLCPP_INFO(this->get_logger(), "'%c' '%c' '%c' '%c'", msg.state1, msg.state2, msg.state3, msg.state4);
        //char state1 = msg.state1, state2 = msg.state1 , state3 = msg.state1, state4 = msg.state1;
	//get_motor_command_from_suscr(state1, state2, state3, state4);
    }

    void timer_callback()

    {
        auto s_message = std_msgs::msg::String();
        s_message.data = "Motor!";
        RCLCPP_INFO(this->get_logger(), "'%s'", s_message.data.c_str());
        publisher_->publish(s_message);

        //ненужные
        /*auto motor_message = robot_interfaces::msg::MotorData();
        //motor_message.state1 = 2;
        RCLCPP_INFO_STREAM(this->get_logger(), "Publishing: '" << motor_message.state1 << "'");
        motor_publisher_->publish(motor_message);*/
    }

    void driver_timer() {
        //go_like_car_with_controller();
        choose_command();
	auto motor_message = robot_interfaces::msg::MotorData();
        motor_message.state1 = motor_command_control_cmd_1;
        motor_message.state2 = motor_command_control_cmd_2;
        motor_message.state3 = motor_command_control_cmd_3;
        motor_message.state4 = motor_command_control_cmd_4;
        motor_publisher_->publish(motor_message);
    }

    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::TimerBase::SharedPtr timer_driver;
    rclcpp::Publisher<robot_interfaces::msg::MotorData>::SharedPtr motor_publisher_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
    rclcpp::Subscription<robot_interfaces::msg::MotorData>::SharedPtr motor_subscription_;


    int motor_num_control_cmd_1 = 1;
    char motor_command_control_cmd_1 = 'S';
    int motor_num_control_cmd_2 = 2;
    char motor_command_control_cmd_2 = 'S';

    int motor_num_control_cmd_3 = 1;
    char motor_command_control_cmd_3 = 'S';
    int motor_num_control_cmd_4 = 2;
    char motor_command_control_cmd_4 = 'S';


};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Motor>());
    rclcpp::shutdown();

    return 0;
}



