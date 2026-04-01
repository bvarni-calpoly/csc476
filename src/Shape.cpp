#include "Shape.h"
#include <iostream>
#include <cassert>

#include "GLSL.h"
#include "Program.h"

using namespace std;
using namespace glm;

// copy the data from the shape to this object
void Shape::createShape(tinyobj::shape_t & shape)
{
	posBuf = shape.mesh.positions;
	norBuf = shape.mesh.normals;
	texBuf = shape.mesh.texcoords;
	eleBuf = shape.mesh.indices;
}

void Shape::measure()
{
	float minX, minY, minZ;
	float maxX, maxY, maxZ;

	minX = minY = minZ = std::numeric_limits<float>::max();
	maxX = maxY = maxZ = -std::numeric_limits<float>::max();

	//Go through all vertices to determine min and max of each dimension
	for (size_t v = 0; v < posBuf.size() / 3; v++)
	{
		if (posBuf[3*v+0] < minX) minX = posBuf[3 * v + 0];
		if (posBuf[3*v+0] > maxX) maxX = posBuf[3 * v + 0];

		if (posBuf[3*v+1] < minY) minY = posBuf[3 * v + 1];
		if (posBuf[3*v+1] > maxY) maxY = posBuf[3 * v + 1];

		if (posBuf[3*v+2] < minZ) minZ = posBuf[3 * v + 2];
		if (posBuf[3*v+2] > maxZ) maxZ = posBuf[3 * v + 2];
	}

	min.x = minX;
	min.y = minY;
	min.z = minZ;
	max.x = maxX;
	max.y = maxY;
	max.z = maxZ;
	center.x = minX + (maxX-minX)/2.0;
	center.y = minY + (maxY-minY)/2.0;
	center.z = minZ + (maxZ-minZ)/2.0;
}

float Shape::largeExtent() {
	float xExt = max.x-min.x;
	float yExt = max.y-min.y;
	float zExt = max.z-min.z;
	float maxE = xExt;
	if (maxE < yExt && yExt > zExt) {
		maxE = yExt;
	} else if (maxE < zExt && zExt > yExt) {
		maxE = zExt;
	}
	return maxE;
}

void Shape::debugOut() {
	cout << "center: " << center.x << " " << center.y << " " << center.y << endl;
	cout << "min: " << min.x << " " << min.y << " " << min.z << endl;
	cout << "max: " << max.x << " " << max.x << " " << max.z << endl;
	cout << "largest extent: " << this->largeExtent() << endl;
	cout << "sizeEl: " << eleBuf.size() << " thus expect faces: " << eleBuf.size()/3.0 << endl;
	cout << "sizePosB: " << posBuf.size() << " thus expect verts: " << posBuf.size()/3.0 << endl;
}

void Shape::init()
{
	// Initialize the vertex array object
	CHECKED_GL_CALL(glGenVertexArrays(1, &vaoID));
	CHECKED_GL_CALL(glBindVertexArray(vaoID));

	// Send the position array to the GPU
	CHECKED_GL_CALL(glGenBuffers(1, &posBufID));
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, posBufID));
	CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW));

	// Send the normal array to the GPU
	if (norBuf.empty())
	{
		//norBufID = 0;
		cout << "NO NORMALS FOUND, CALCULATING NORMALS" << endl;

		// initialize norBuf to the length of posBuf (number of vertices, every vertex has a normal)
		norBuf.assign(posBuf.size(), 0.0f);
		
		// iterate over all faces, retriving the vertices, and calculating the normal vector for each one
		for (size_t i = 0; i < eleBuf.size() / 3; i++)
		{
			// three vertex positions of the face
			int v1idx = eleBuf[3 * i + 0];
			int v2idx = eleBuf[3 * i + 1];
			int v3idx = eleBuf[3 * i + 2];
			
			// vertex data
			vec3 v1 = vec3(posBuf[3 * v1idx + 0], posBuf[3 * v1idx + 1], posBuf[3 * v1idx + 2]);
			vec3 v2 = vec3(posBuf[3 * v2idx + 0], posBuf[3 * v2idx + 1], posBuf[3 * v2idx + 2]);
			vec3 v3 = vec3(posBuf[3 * v3idx + 0], posBuf[3 * v3idx + 1], posBuf[3 * v3idx + 2]);
			
			// compute edge vectors for one vertice
			// (vector between two points, cross product is the same for all three vertices)
			vec3 vec12 = v2 - v1;
			vec3 vec13 = v3 - v1;

			// normal vector
			vec3 normal = normalize(cross(vec12, vec13));

			norBuf[3 * v1idx + 0] = normal.x, norBuf[3 * v1idx + 1] = v1.y, norBuf[3 * v1idx + 2] = normal.z;
			norBuf[3 * v2idx + 0] = normal.x, norBuf[3 * v2idx + 1] = v1.y, norBuf[3 * v2idx + 2] = normal.z;
			norBuf[3 * v3idx + 0] = normal.x, norBuf[3 * v3idx + 1] = v1.y, norBuf[3 * v3idx + 2] = normal.z;
		}
	}

	// Send the normal array to the GPU
	CHECKED_GL_CALL(glGenBuffers(1, &norBufID));
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, norBufID));
	CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW));

	// Send the texture array to the GPU
	if (texBuf.empty())
	{
		texBufID = 0;
		cout << "warning no textures!" << endl;
	}
	else
	{
		CHECKED_GL_CALL(glGenBuffers(1, &texBufID));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, texBufID));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW));
	}

	// Send the element array to the GPU
	CHECKED_GL_CALL(glGenBuffers(1, &eleBufID));
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID));
	CHECKED_GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBuf.size()*sizeof(unsigned int), &eleBuf[0], GL_STATIC_DRAW));

	// Unbind the arrays
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

void Shape::draw(const shared_ptr<Program> prog) const
{
	int h_pos, h_nor, h_tex;
	h_pos = h_nor = h_tex = -1;

	CHECKED_GL_CALL(glBindVertexArray(vaoID));

	// Bind position buffer
	h_pos = prog->getAttribute("vertPos");
	GLSL::enableVertexAttribArray(h_pos);
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, posBufID));
	CHECKED_GL_CALL(glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));

	// Bind normal buffer
	h_nor = prog->getAttribute("vertNor");
	if (h_nor != -1 && norBufID != 0)
	{
		GLSL::enableVertexAttribArray(h_nor);
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, norBufID));
		CHECKED_GL_CALL(glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));
	}

	if (texBufID != 0)
	{
		// Bind texcoords buffer
		h_tex = prog->getAttribute("vertTex");

		if (h_tex != -1 && texBufID != 0)
		{
			GLSL::enableVertexAttribArray(h_tex);
			CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, texBufID));
			CHECKED_GL_CALL(glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0));
		}
	}

	// Bind element buffer
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID));

	// Draw
	CHECKED_GL_CALL(glDrawElements(GL_TRIANGLES, (int)eleBuf.size(), GL_UNSIGNED_INT, (const void *)0));

	// Disable and unbind
	if (h_tex != -1)
	{
		GLSL::disableVertexAttribArray(h_tex);
	}
	if (h_nor != -1)
	{
		GLSL::disableVertexAttribArray(h_nor);
	}
	GLSL::disableVertexAttribArray(h_pos);
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}