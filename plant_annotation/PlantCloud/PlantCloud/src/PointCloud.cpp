#pragma once

#include "PointCloud.h"
#include <fstream>
#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include "Icosphere.h"
#include <math.h>

PointCloud::PointCloud(std::string& filepath)
	:m_Filepath(filepath), numPoints(0)
{
	LoadPoints();
	//AddSpherePoints();
}

void PointCloud::AddSpherePoints()
{
	Icosphere sphere(0.1f, 5, true);
	const float* vertices = sphere.getVertices();
	int num_vertices = sphere.getVertexCount();

	for (int j = 0; j < num_vertices*3; j+=3) 
	{
		glm::vec3 v = glm::vec3(vertices[j], vertices[j + 1], vertices[j + 2]);
		AddPoint(v);
		glm::vec3 c = glm::vec3(0.5f,0.5f,0.5f);
		AddColor(c);
	}
	// update indices
	//first add pointcloud indices then add sphere indices
	for (int j = 0; j < numPoints; j ++)
	{
		indices.push_back(j);
	}
	int sphere_index_count = sphere.getIndexCount();
	for (int j = 0; j < sphere_index_count; j++)
	{
		indices.push_back(j+numPoints);
	}
	totalNumIndices = numPoints + sphere_index_count;
	totalNumPoints = numPoints + num_vertices;
}

void PointCloud::ReInitialize(std::string& filepath)
{

	m_Filepath = filepath;
	numPoints = 0;
	lowerbb = glm::vec3();
	upperbb = glm::vec3();
		LoadPoints();
		//AddSpherePoints();
	
}

void PointCloud::LoadPoints()
{
	meancoords = glm::vec3();
	

	std::ifstream file(m_Filepath);
	if (!file.is_open()) 
	{
		std::cout << "Failed to open file: " << m_Filepath << std::endl;
	}
	
	points.clear();
	glm::vec3 aPoint;
	glm::vec3 aColor;
	//int counter = 1;
	while (!file.eof())
	{
		if (file >> aPoint.x >> aPoint.y >> aPoint.z >> aColor.x >> aColor.y >> aColor.z)
		{
			if (numPoints == 0) 
			{
				lowerbb.x = aPoint.x;
				lowerbb.y = aPoint.y;
				lowerbb.z = aPoint.z;
				upperbb.x = aPoint.x;
				upperbb.y = aPoint.y;
				upperbb.z = aPoint.z;
			}
			AddPoint(aPoint);
			AddColor(aColor);
			numPoints += 1;

			meancoords.x += aPoint.x;
			meancoords.y += aPoint.y;
			meancoords.z += aPoint.z;
		}

		if (numPoints % 10000 == 0) std::cout << "numPoints: "<<numPoints << std::endl;
	}
	std::cout << "Point cloud loaded: \nNumer of Points" << numPoints << std::endl;

	std::cout << "Loaded Points: " << std::endl;
	
	meancoords.x /= numPoints;
	meancoords.y /= numPoints;
	meancoords.z /= numPoints;


	for (int j = 0; j < numPoints; j++)
	{
		indices.push_back(j);
	}
	totalNumIndices = numPoints;
	totalNumPoints = numPoints ;
	//for (int i=0;i<points.size();i++) std::cout << glm::to_string(points[i]) << std::endl;

}




void PointCloud::AddPoint(glm::vec3& aPoint)
{
	if (aPoint.x < lowerbb.x)	lowerbb.x = aPoint.x;
	if (aPoint.y < lowerbb.y)	lowerbb.y = aPoint.y;
	if (aPoint.z < lowerbb.z)	lowerbb.z = aPoint.z;
	if (aPoint.x > upperbb.x)	upperbb.x = aPoint.x;
	if (aPoint.y > upperbb.y)	upperbb.y = aPoint.y;
	if (aPoint.z > upperbb.z)	upperbb.z = aPoint.z;
	points.push_back(aPoint);
}

void PointCloud::AddColor(glm::vec3& aColor)
{
	points.push_back(aColor);
	colors.push_back(aColor);
}

void PointCloud::do_transform(glm::mat4 mvp)
{
	
	for (int i = 0; i < points.size(); i += 2)
	{ 
		glm::vec4 avec = glm::vec4(points[i][0], points[i][1], points[i][2], 1.0f);

		//std::cout << glm::to_string(mvp * avec) << std::endl;
	}
}
void PointCloud::do_color()
{
	for (int i = 1; i < points.size(); i += 2)
	{
		points[i][0] = 0.1f;
		points[i][1] = 0.8f;
		points[i][2] = 0.3f;
		//glm::vec4 avec = glm::vec4(points[i][0], points[i][1], points[i][2], 1.0f);

		//std::cout << glm::to_string(mvp * avec) << std::endl;
	}
}

void PointCloud::do_color_small(float x, float y, glm::mat4 mvp, float radius, float red, float green, float blue)
{
	for (int i = 1; i < points.size(); i += 2)
	{
		glm::vec4 res = mvp * glm::vec4(points[i - 1], 1.0f);

		float dist = pow( pow((res[0] - x), 2) + pow((res[1] - y), 2), 0.5);

		if (dist < radius) { //6
			points[i][0] = red; // 0.1f;
			points[i][1] = green; // 0.8f;
			points[i][2] = blue; // 0.7f;
		}

	}
	std::string colorstr = std::to_string(red) + "," + std::to_string(green) + "," + std::to_string(blue);

	if (colormap.find(colorstr) == colormap.end()) 
	{
		maxLabel += 1;
		colormap[colorstr] = 1;
		label2colormap[maxLabel] = glm::vec3(red, green, blue);
	}// if key is not present then it is a new label
}


void PointCloud::do_erase(float x, float y, glm::mat4 mvp, float radius)
{
	for (int i = 1; i < points.size(); i += 2)
	{
		//std::cout << "Here is i: " << i << std::endl;
		glm::vec4 res = mvp * glm::vec4(points[i - 1], 1.0f);

		float dist = pow(pow((res[0] - x), 2) + pow((res[1] - y), 2), 0.5);

		if (dist < radius) {
			points[i][0] = colors[(i - 1) / 2].x; // 0.1f;   //// these x y z are actually colors values // this is because c++ allows to keep value of x y z component of our vector
			points[i][1] = colors[(i - 1) / 2].y; // 0.8f;
			points[i][2] = colors[(i - 1) / 2].z; // 0.7f;
		}
	}
}


void PointCloud::SaveFile(char* inputfname)
{
	std::ofstream file(inputfname);
	if (!file.is_open())
	{
		std::cout << "Failed to open file: " << inputfname << std::endl;
		return;
	}

	for (int i = 0; i < points.size(); i += 2)
	{
		file << points[i][0] << " " << points[i][1] << " " << points[i][2] << " " << points[i + 1][0] << " " << points[i + 1][1] << " " << points[i + 1][2] << std::endl;

			//points[i][0] = colors[(i - 1) / 2].x; // 0.1f;   //// these x y z are actually colors values // this is because c++ allows to keep value of x y z component of our vector
			//points[i][1] = colors[(i - 1) / 2].y; // 0.8f;
			//points[i][2] = colors[(i - 1) / 2].z; // 0.7f;
		
	}

		return;
}



// 0 2 4 6 8 10 12
// 0 1 2 3 4 5 6 



//glm::vec4 avec = glm::vec4(points[i][0], points[i][1], points[i][2], 1.0f);

//std::cout << glm::to_string(mvp * avec) << std::endl;

//void PointCloud::SetColor(glm::vec3 color)
//{
//	colors = std::vector<glm::vec3>(numPoints, color);
//}
