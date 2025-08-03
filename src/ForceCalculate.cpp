#include "ForceCalculate.hpp"



glm::vec3 calculator::skewlineForce(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C, const glm::vec3& D, float thres, float magnitude) {
	// Input:
	// - glm::vec3& A : the first end point of line one
	// - glm::vec3& B : the second end point of line one
	// - glm::vec3& C : the first end point of line two (fixed)
	// - glm::vec3& D : the second end point of line two (fixed)
	// - float thres : when the distance of skew line is less the thres then render the force
	// - float magnitude :  the coeff of spring force
	// 
	// Output:
	// - glm::vec3 force
	//


	glm::vec3 direction;
	float distance;
	glm::vec3 force = { 0.0f,0.0f,0.0f };

	//// calculate distance
	// calculate vector
	glm::vec3 AB = B - A;
	glm::vec3 CD = D - C;
	// calculate normal vector
	glm::vec3 n = glm::cross(AB, CD);

	glm::vec3 AC = C - A;

	float n_length2 = glm::length(n);
	if (n_length2 < 1e-8f) {
		// if the lengh of n near to 0, that means two lines almost parallel to each other

		glm::vec3 CD_dir = glm::normalize(CD);
		glm::vec3 projection = AC - glm::dot(AC, CD_dir) * CD_dir;
		distance = glm::length(projection);
		direction = glm::normalize(projection);
	}
	else {
		// distance : d = |(AC · n) / |n| |
		distance = std::abs(glm::dot(AC, n)) / std::sqrt(n_length2);
		glm::vec3 projected_AC_on_n = glm::dot(AC, n) / glm::length(n) * n;
		direction = glm::normalize(projected_AC_on_n);
	}

	if (distance < thres) {
		force = (thres - distance) * magnitude * direction;
	}


	return force;

}


void ForceManage::schedule() {

	glm::vec3 toolWeight{ 0.0f,0.0f,0.2f };



	if (mode == start) {
		mode = touching;
	}

	// 2. touching mode
	// skew line
	if (mode == touching) {
		// TODO: rewrite the haptic method
	}


	if (mode == none) {

	}


	force = force + toolWeight;

}


