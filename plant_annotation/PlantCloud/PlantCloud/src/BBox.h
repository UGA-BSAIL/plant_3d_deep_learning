#pragma once

#include "glm/glm.hpp"


class BBox
{
public:

	glm::vec3 bbox_translation; // (0.0f, 0.0f, 0.0f);
	glm::vec3 bbox_rotation; // (0.0f, 0.0f, 0.0f);
	glm::vec3 bbox_scale; // (1.0f, 1.0f, 1.0f);

	glm::mat4 curr_mvp;
	//char* labelname;
	char* labelname = new char[256];
	/*public*/
	BBox();

};
