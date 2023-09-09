#pragma once

#include <vector>
#include "glm/glm.hpp"
#include <string>
#include <unordered_map>
#include <string>
#include <string>
#include <string>

class PointCloud
{
private:
	std::string m_Filepath;
	std::vector<glm::vec3> points;
	std::vector<glm::vec3> colors;
	glm::vec3 lowerbb;
	glm::vec3 upperbb;
	glm::vec3 meancoords;

	int numPoints;
	int totalNumPoints;
	int totalNumIndices;
	
	std::vector<unsigned int> indices;


public:
	PointCloud(std::string& filepath);
	void LoadPoints();
	void AddSpherePoints();

	void ReInitialize(std::string& filepath); //std::vector<glm::vec3>().swap(x);


	void AddPoint(glm::vec3& aPoint);
	void AddColor(glm::vec3& aColor);
	void SetColor(glm::vec3 color);

	inline int GetNumPoints() { return numPoints; }
	inline int GetTotalNumPoints() { return totalNumPoints; }
	inline int GetTotalNumIndices() { return totalNumIndices; }

	inline std::vector<glm::vec3>& GetPoints() { return points; }
	inline std::vector<glm::vec3>& GetColor() { return colors; }
	inline std::vector<unsigned int>& GetIndices() { return indices; }

	inline glm::vec3 Getlowerbb() { return lowerbb; }
	inline glm::vec3 Getupperbb() { return upperbb; }
	inline glm::vec3 Getmeancoords() { return meancoords; }


	void do_transform(glm::mat4 mvp);
	void do_color();
	void do_color_small(float x, float y, glm::mat4 mvp, float radius, float red, float green, float blue);
	void do_erase(float x, float y, glm::mat4 mvp, float radius);


	void SaveFile(char* inputfname);

	int maxLabel = 0;
	std::unordered_map<std::string, int> colormap;
	std::unordered_map<int, glm::vec3> label2colormap;


};
