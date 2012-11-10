#ifndef GEOMETRYMANAGER_H
#define GEOMETRYMANAGER_H

#include <GL/glew.h>

const float vertexData[] = {
	1.0f, 1.0f, 0.0f, 1.0f,
	1.0f, -1.0f, 0.0f, 1.0f,
	-1.0f, 1.0f, 0.0f, 1.0f,
	-1.0f, -1.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 1.0f,
    0.0f, 0.0f, 0.0f, 1.0f,
	1.0f, 0.0f,
	1.0f, 1.0f,
	0.0f, 0.0f,
	0.0f, 1.0f,
};

const unsigned int indexData[] = {
    0, 1, 2, 3
};

class GeometryManager {
private:
    GLuint positionBufferObject;
    GLuint vertexArrayObject;
    GLuint indexBufferObject;
public:
    GeometryManager();
    ~GeometryManager();
    void Draw();
};

#endif
