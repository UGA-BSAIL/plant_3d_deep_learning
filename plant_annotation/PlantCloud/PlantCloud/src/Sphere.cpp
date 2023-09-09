#include "Sphere.h"
#include "Icosphere.h"

Sphere::Sphere(float radius, int subdivisions, bool smooth)
	:radius(radius), subdivisions(subdivisions), smooth(smooth)
{
	Icosphere sphere(radius, subdivisions, smooth);
	totalNumIndices =  sphere.getIndexCount();
	totalNumPoints = sphere.getVertexCount();

	const float* vertices = sphere.getVertices();
	const unsigned int* sphere_indices = sphere.getIndices();
	// indices
	for (int j = 0; j < totalNumIndices; j++)	indices.push_back(sphere_indices[j]);
	// vertices // compute mean of points as well so that later it can normalized and to mouse center
	for (int j = 0; j < totalNumPoints * 3; j+=3) 
	{ 
		points.push_back(glm::vec3(vertices[j], vertices[j+1], vertices[j+2])); // points
		points.push_back(glm::vec3(0.75f,0.75f,0.75f)); // colors
	}

}

Sphere::~Sphere()
{
}

void Sphere::LoadPoints()
{
}
