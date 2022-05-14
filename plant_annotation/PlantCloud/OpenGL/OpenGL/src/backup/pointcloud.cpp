#pragma once

#include "PointCloud.h"
#include <fstream>
#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include "Icosphere.h"

PointCloud::PointCloud(std::string& filepath)
	:m_Filepath(filepath), numPoints(0)
{
	LoadPoints();
	AddSpherePoints();
}

void PointCloud::AddSpherePoints()
{
	Icosphere sphere(0.1f, 5, true);
	const float* vertices = sphere.getVertices();
	int num_vertices = sphere.getVertexCount();

	for (int j = 0; j < num_vertices * 3; j += 3)
	{
		glm::vec3 v = glm::vec3(vertices[j], vertices[j + 1], vertices[j + 2]);
		AddPoint(v);
		glm::vec3 c = glm::vec3(0.5f, 0.5f, 0.5f);
		AddPoint(c);
	}
	// update indices
	//first add pointcloud indices then add sphere indices
	for (int j = 0; j < numPoints; j++)
	{
		indices.push_back(j);
	}
	int sphere_index_count = sphere.getIndexCount();
	for (int j = 0; j < sphere_index_count; j++)
	{
		indices.push_back(j + numPoints);
	}
	totalNumIndices = numPoints + sphere_index_count;
	totalNumPoints = numPoints + num_vertices;
}

void PointCloud::LoadPoints()
{
	std::ifstream file(m_Filepath);
	if (!file.is_open())
	{
		std::cout << "Failed to open file: " << m_Filepath << std::endl;
	}

	points.clear();
	glm::vec3 aPoint;
	glm::vec3 aColor;
	while (!file.eof())
	{
		if (file >> aPoint.x >> aPoint.y >> aPoint.z >> aColor.x >> aColor.y >> aColor.z)
		{
			AddPoint(aPoint);
			AddColor(aColor);
			numPoints += 1;
		}

		if (numPoints % 10000 == 0) std::cout << "numPoints: " << numPoints << std::endl;
	}
	std::cout << "Point cloud loaded: \nNumer of Points" << numPoints << std::endl;

	std::cout << "Loaded Points: " << std::endl;

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
}

void PointCloud::SetColor(glm::vec3 color)
{
	colors = std::vector<glm::vec3>(numPoints, color);
}
