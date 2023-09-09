#include <glm/ext/vector_float3.hpp>
#include <vector>
class Sphere 
{
private:
	float radius;
	int subdivisions;
	bool smooth;
	std::vector<glm::vec3> points;
	std::vector<unsigned int> indices;

	int totalNumPoints;
	int totalNumIndices;
public:
	Sphere(float radius, int subdivisions, bool smooth);
	~Sphere();
	void LoadPoints();

	inline int GetTotalNumIndices() { return totalNumIndices; }
	inline int GetTotalNumPoints() { return totalNumPoints; }
	inline std::vector<glm::vec3>& GetPoints() { return points; }
	inline std::vector<unsigned int>& GetIndices() { return indices; }

};