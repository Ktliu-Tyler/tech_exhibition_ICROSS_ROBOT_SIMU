#include "iCROSS.h"

iCROSS::iCROSS() {
	current_value << 700, 0, 0, 0, 0, 0;
}
iCROSS::iCROSS(const double* link_params, const double* init_val, const double joint_lim[6][2]) {
	for (int i = 0; i < 5; ++i) {
		link_parameters[i] = link_params[i];
	}

	for (int i = 0; i < 6; ++i) {
		current_value[i] = init_val[i];
	}
	for (int i = 0; i < 6; ++i) {
		if (i < 2) {
			// Joint 1 and 2 units in [mm]
			joint_limits[i][0] = joint_lim[i][0];
			joint_limits[i][1] = joint_lim[i][1];
		}
		else {
			// Joint 3 to 6 units in [rad]
			joint_limits[i][0] = joint_lim[i][0] * M_PI / 180;
			joint_limits[i][1] = joint_lim[i][1] * M_PI / 180;
		}
	}

	jacobian.setZero(12, 6);
	jacobian(0, 0) = 1;
	jacobian(2, 1) = -1;

	forward(current_value);
	update_jacobian(current_value);
}
void iCROSS::update_target_pos_by_roll_pitch_yaw(double target[6]) {
	double x = target[0],
		y = target[1],
		z = target[2];
	double roll = target[3],
		pitch = target[4],
		yaw = target[5];

	target_pos[0] = x;
	target_pos[1] = y;
	target_pos[2] = z;

	target_pos[3] = cos(yaw) * cos(pitch);
	target_pos[4] = cos(yaw) * sin(pitch) * sin(roll) - sin(yaw) * cos(roll);
	target_pos[5] = cos(yaw) * sin(pitch) * cos(roll) + sin(yaw) * sin(roll);

	target_pos[6] = sin(yaw) * cos(pitch);
	target_pos[7] = sin(yaw) * sin(pitch) * sin(roll) + cos(yaw) * cos(roll);
	target_pos[8] = sin(yaw) * sin(pitch) * cos(roll) - cos(yaw) * sin(roll);

	target_pos[9] = -sin(pitch);
	target_pos[10] = cos(pitch) * sin(roll);
	target_pos[11] = cos(pitch) * cos(roll);
}
void iCROSS::update_targetPosition(hduMatrix worldPose) {
	target_pos[0] = worldPose[0][3]; // x
	target_pos[1] = worldPose[1][3]; // y
	target_pos[2] = worldPose[2][3]; // z
	// flatten the rotation matrix
	target_pos[3] = worldPose[0][0];
	target_pos[4] = worldPose[0][1];
	target_pos[5] = worldPose[0][2];
	target_pos[6] = worldPose[1][0];
	target_pos[7] = worldPose[1][1];
	target_pos[8] = worldPose[1][2];
	target_pos[9] = worldPose[2][0];
	target_pos[10] = worldPose[2][1];
	target_pos[11] = worldPose[2][2];
}
bool iCROSS::check_valid(Eigen::Matrix<double, 1, 6>& tmp_value, Eigen::Matrix<double, 1, 6>& clipped_value) {
	bool isValid = true;

	for (int i = 0; i < 6; ++i) { // Loop through each joint value
		clipped_value[i] = tmp_value[i]; // Initialize clipped value to joint value

		// Check and clip to lower limit
		if (tmp_value[i] < joint_limits[i][0]) {
			clipped_value[i] = joint_limits[i][0];
			isValid = false; // Mark as invalid if clipping occurs
		}
		// Check and clip to upper limit
		else if (tmp_value[i] > joint_limits[i][1]) {
			clipped_value[i] = joint_limits[i][1];
			isValid = false; // Mark as invalid if clipping occurs
		}
		//for (int i = 0; i < 6; i++) {
		//    if (clipped_value[i] - tmp_value[i] < saturation_accuracy)
		//        isValid = true;
		//    else
		//        isValid = false;
		//}


	}

	return isValid;
}
void iCROSS::update_jacobian(Eigen::Matrix<double, 1, 6>& joint_value) {

	double  d1 = joint_value[0],
		d2 = joint_value[1],
		th1 = joint_value[2],
		th2 = joint_value[3],
		th3 = joint_value[4],
		th4 = joint_value[5];

	double l1 = link_parameters[0],
		l2 = link_parameters[1],
		l3 = link_parameters[2],
		l4 = link_parameters[3],
		h = link_parameters[4],
		w = l1 + l2 + l3;

	// 1st row of the rotation matrix
	jacobian(0, 2) = l4 * (cos(th1) * sin(th3) - cos(th3) * sin(th1) * sin(th2)) - l3 * (cos(th1) * cos(th3) + sin(th1) * sin(th2) * sin(th3)) - l2 * cos(th1);
	jacobian(0, 3) = cos(th1) * cos(th2) * (l4 * cos(th3) + l3 * sin(th3));
	jacobian(0, 4) = l3 * (sin(th1) * sin(th3) + cos(th1) * cos(th3) * sin(th2)) + l4 * (cos(th3) * sin(th1) - cos(th1) * sin(th2) * sin(th3));

	//% 2nd row of the rotation matrix
	jacobian(1, 2) = l4 * (sin(th1) * sin(th3) + cos(th1) * cos(th3) * sin(th2)) - l3 * (cos(th3) * sin(th1) - cos(th1) * sin(th2) * sin(th3)) - l2 * sin(th1);
	jacobian(1, 3) = cos(th2) * sin(th1) * (l4 * cos(th3) + l3 * sin(th3));
	jacobian(1, 4) = -l3 * (cos(th1) * sin(th3) - cos(th3) * sin(th1) * sin(th2)) - l4 * (cos(th1) * cos(th3) + sin(th1) * sin(th2) * sin(th3));

	//% 3rd row of the rotation matrix
	jacobian(2, 3) = sin(th2) * (l4 * cos(th3) + l3 * sin(th3));
	jacobian(2, 4) = -cos(th2) * (l3 * cos(th3) - l4 * sin(th3));

	//% 4th row of the rotation matrix
	jacobian(3, 2) = -cos(th2) * cos(th4) * sin(th1) - cos(th1) * cos(th3) * sin(th4) - sin(th1) * sin(th2) * sin(th3) * sin(th4);
	jacobian(3, 3) = -cos(th1) * (cos(th4) * sin(th2) - cos(th2) * sin(th3) * sin(th4));
	jacobian(3, 4) = sin(th4) * (sin(th1) * sin(th3) + cos(th1) * cos(th3) * sin(th2));
	jacobian(3, 5) = cos(th1) * cos(th4) * sin(th2) * sin(th3) - cos(th3) * cos(th4) * sin(th1) - cos(th1) * cos(th2) * sin(th4);

	//% 5th row of the rotation matrix
	jacobian(4, 2) = cos(th2) * sin(th1) * sin(th4) - cos(th1) * cos(th3) * cos(th4) - cos(th4) * sin(th1) * sin(th2) * sin(th3);
	jacobian(4, 3) = cos(th1) * (sin(th2) * sin(th4) + cos(th2) * cos(th4) * sin(th3));
	jacobian(4, 4) = cos(th4) * (sin(th1) * sin(th3) + cos(th1) * cos(th3) * sin(th2));
	jacobian(4, 5) = cos(th3) * sin(th1) * sin(th4) - cos(th1) * cos(th2) * cos(th4) - cos(th1) * sin(th2) * sin(th3) * sin(th4);

	//% 6th row of the rotation matrix
	jacobian(5, 2) = cos(th3) * sin(th1) * sin(th2) - cos(th1) * sin(th3);
	jacobian(5, 3) = -cos(th1) * cos(th2) * cos(th3);
	jacobian(5, 4) = cos(th1) * sin(th2) * sin(th3) - cos(th3) * sin(th1);

	//% 7th row of the rotation matrix
	jacobian(6, 2) = cos(th1) * cos(th2) * cos(th4) - cos(th3) * sin(th1) * sin(th4) + cos(th1) * sin(th2) * sin(th3) * sin(th4);
	jacobian(6, 3) = -sin(th1) * (cos(th4) * sin(th2) - cos(th2) * sin(th3) * sin(th4));
	jacobian(6, 4) = -sin(th4) * (cos(th1) * sin(th3) - cos(th3) * sin(th1) * sin(th2));
	jacobian(6, 5) = cos(th1) * cos(th3) * cos(th4) - cos(th2) * sin(th1) * sin(th4) + cos(th4) * sin(th1) * sin(th2) * sin(th3);

	//% 8th row of the rotation matrix
	jacobian(7, 2) = cos(th1) * cos(th4) * sin(th2) * sin(th3) - cos(th3) * cos(th4) * sin(th1) - cos(th1) * cos(th2) * sin(th4);
	jacobian(7, 3) = sin(th1) * (sin(th2) * sin(th4) + cos(th2) * cos(th4) * sin(th3));
	jacobian(7, 4) = -cos(th4) * (cos(th1) * sin(th3) - cos(th3) * sin(th1) * sin(th2));
	jacobian(7, 5) = -cos(th2) * cos(th4) * sin(th1) - cos(th1) * cos(th3) * sin(th4) - sin(th1) * sin(th2) * sin(th3) * sin(th4);

	//% 9th row of the rotation matrix
	jacobian(8, 2) = -sin(th1) * sin(th3) - cos(th1) * cos(th3) * sin(th2);
	jacobian(8, 3) = -cos(th2) * cos(th3) * sin(th1);
	jacobian(8, 4) = cos(th1) * cos(th3) + sin(th1) * sin(th2) * sin(th3);

	//% 10th row of the rotation matrix
	jacobian(9, 3) = cos(th2) * cos(th4) + sin(th2) * sin(th3) * sin(th4);
	jacobian(9, 4) = -cos(th2) * cos(th3) * sin(th4);
	jacobian(9, 5) = -sin(th2) * sin(th4) - cos(th2) * cos(th4) * sin(th3);

	//% 11th row of the rotation matrix
	jacobian(10, 3) = cos(th4) * sin(th2) * sin(th3) - cos(th2) * sin(th4);
	jacobian(10, 4) = -cos(th2) * cos(th3) * cos(th4);
	jacobian(10, 5) = cos(th2) * sin(th3) * sin(th4) - cos(th4) * sin(th2);

	//% 12th row of the rotation matrix
	jacobian(11, 3) = -cos(th3) * sin(th2);
	jacobian(11, 4) = -cos(th2) * sin(th3);

}
void iCROSS::forward(Eigen::Matrix<double, 1, 6> joint_value) {

	double  d1 = joint_value[0];
	double  d2 = joint_value[1];
	double th1 = joint_value[2];
	double th2 = joint_value[3];
	double th3 = joint_value[4];
	double th4 = joint_value[5];

	double l1 = link_parameters[0];
	double l2 = link_parameters[1];
	double l3 = link_parameters[2];
	double l4 = link_parameters[3];
	double  h = link_parameters[4];

	// x
	current_pos[0] = d1 - l3 * (cos(th3) * sin(th1) - cos(th1) * sin(th2) * sin(th3)) + l4 * (sin(th1) * sin(th3) + cos(th1) * cos(th3) * sin(th2)) - l2 * sin(th1);

	// y
	current_pos[1] = l3 * (cos(th1) * cos(th3) + sin(th1) * sin(th2) * sin(th3)) - l3 - l2 - l4 * (cos(th1) * sin(th3) - cos(th3) * sin(th1) * sin(th2)) + l2 * cos(th1);

	// z
	current_pos[2] = h - d2 - l4 * cos(th2) * cos(th3) - l3 * cos(th2) * sin(th3);

	// First row of the rotation matrix
	current_pos[3] = cos(th1) * cos(th2) * cos(th4) - cos(th3) * sin(th1) * sin(th4) + cos(th1) * sin(th2) * sin(th3) * sin(th4);
	current_pos[4] = cos(th1) * cos(th4) * sin(th2) * sin(th3) - cos(th3) * cos(th4) * sin(th1) - cos(th1) * cos(th2) * sin(th4);
	current_pos[5] = -sin(th1) * sin(th3) - cos(th1) * cos(th3) * sin(th2);

	// Second row of the rotation matrix
	current_pos[6] = cos(th2) * cos(th4) * sin(th1) + cos(th1) * cos(th3) * sin(th4) + sin(th1) * sin(th2) * sin(th3) * sin(th4);
	current_pos[7] = cos(th1) * cos(th3) * cos(th4) - cos(th2) * sin(th1) * sin(th4) + cos(th4) * sin(th1) * sin(th2) * sin(th3);
	current_pos[8] = cos(th1) * sin(th3) - cos(th3) * sin(th1) * sin(th2);

	// Third row of the rotation matrix
	current_pos[9] = cos(th4) * sin(th2) - cos(th2) * sin(th3) * sin(th4);
	current_pos[10] = -sin(th2) * sin(th4) - cos(th2) * cos(th4) * sin(th3);
	current_pos[11] = cos(th2) * cos(th3);
}
bool iCROSS::solve_inverse() {


	Eigen::Matrix<double, 6, 12> i_jacobian;
	Eigen::Matrix<double, 1, 6> temp_value;
	Eigen::Matrix<double, 1, 12> diff;
	Eigen::Matrix<double, 1, 12> new_diff;
	Eigen::Matrix<double, 6, 1> de;
	Eigen::Matrix<double, 1, 3> position_diff;
	Eigen::Matrix<double, 1, 9> orientation_diff;


	clock_t start_time = clock();
	temp_value = current_value;

	int it = 0;
	while (it < solver_timeout) {
		it = it + 1;
		diff = target_pos - current_pos;
		// update jacobian and calculate psuedo inverse
		update_jacobian(temp_value);


		i_jacobian = jacobian.completeOrthogonalDecomposition().pseudoInverse();

		de = i_jacobian * diff.transpose();

		de[0] = de[0];

		//cout << i_jacobian << endl<<endl;
		//cout << diff.transpose() << endl <<endl;
		//cout << de << endl << endl;


		temp_value = temp_value + de.transpose();
		//cout << temp_value << endl;



		forward(temp_value);
		//cout << current_pos << endl;
		new_diff = target_pos - current_pos;

		for (int i = 0; i < 3; i++) {
			position_diff[i] = new_diff[i];
		}
		for (int i = 3; i < 12; i++) {
			orientation_diff[i - 3] = new_diff[i];
		}
		//cout << position_diff << endl;

		// Check if position and orientation are within accuracy thresholds
		if (position_diff.norm() < solver_position_accuracy && orientation_diff.norm() < solver_rotation_accuracy) {
			Eigen::Matrix<double, 1, 6> clipped_value;
			bool valid = check_valid(temp_value, clipped_value);
			if (valid) { // no saturation
				current_value = clipped_value;
				forward(current_value);

				if (log_mode) {
					std::cout << "current_pos: ";
					std::cout << current_pos << endl;
					std::cout << ", time consumption: " << (clock() - start_time) / (double)CLOCKS_PER_SEC << ", it: " << it << std::endl << std::flush;

				}

				return true;

			}
			else { // saturation happens
				current_value = clipped_value;
				forward(current_value);
				//cout << "Saturation happens!!!" << endl;

				return false;
			}
		}

	}

	forward(current_value);
	if (log_mode) {
		std::cout << "Timeout! Time consumption: " << (clock() - start_time) / (double)CLOCKS_PER_SEC << ", it: " << it << std::endl;
	}
	return false;

}