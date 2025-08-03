#ifndef __FORCECAL__
#define __FORCECAL__

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <math.h>
#include <numeric>

#include "definition.h"
#include "Connecter.hpp"



class calculator {
public:
	calculator() {
		startTime = clock();
	};
	~calculator() {};

	glm::vec3 skewlineForce(const glm::vec3&, const glm::vec3&, const glm::vec3&, const glm::vec3&, float, float);

private:
	clock_t startTime = 0;
};


class ForceManage {
public:
	ForceManage() {};
	~ForceManage() {};

	enum state {
		start, // start render force
		touching, // skew line

		none,

	};

	void schedule();

	glm::mat4 tool_pose;
	state mode = start;

	glm::vec3 force;

	OutputLayer* pOutput;


private:

};


#endif
