#pragma once
#define _USE_MATH_DEFINES
#include <iostream>
#include <cmath>
#include <ctime>
#include <chrono>
#include <vector>
#include <Eigen/Dense>
#include "Definition.h"
#include <HDU/hduMatrix.h>

using namespace std;
class iCROSS {
    public:
        bool log_mode = false;
        double link_parameters[5] = { 91.5, 155, 24.5, 200, 520 };
        double joint_limits[6][2] = { {0, 350}, {0, 150}, {-90, 0}, {-90, 90}, {-60, 60}, {-360, 360} };
        Eigen::Matrix<double, 1, 6> current_value;
        Eigen::Matrix<double, 1, 12> current_pos;
        Eigen::Matrix<double, 1, 12> target_pos;
        Eigen::Matrix<double, 12, 6> jacobian;
        double solver_timeout = 500;
        double solver_position_accuracy = 1e-2;
        double solver_rotation_accuracy = 1e-4;
        double saturation_accuracy = 1e-6;
        iCROSS();
        ~iCROSS(){};
        iCROSS(const double* link_params, const double* init_val, const double joint_lim[6][2]);
        void update_target_pos_by_roll_pitch_yaw(double target[6]);
        void update_targetPosition(hduMatrix worldPose);
        void forward(Eigen::Matrix<double, 1, 6> joint_value);
        bool solve_inverse();
    private:
        void update_jacobian(Eigen::Matrix<double, 1, 6>& joint_value);
        bool check_valid(Eigen::Matrix<double, 1, 6>& tmp_value, Eigen::Matrix<double, 1, 6>& clipped_value);
    
};