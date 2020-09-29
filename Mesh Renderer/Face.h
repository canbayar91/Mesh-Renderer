#ifndef _FACE_H_
#define _FACE_H_

enum DrawingMode {
	VERTEX_MODE,
	EDGE_MODE,
	FACE_MODE
};

class Face {
public:

	Face() {
		this->vertex0 = 0;
		this->vertex1 = 0;
		this->vertex2 = 0;
	}

	Face(int vertex0, int vertex1, int vertex2) {
		this->vertex0 = vertex0;
		this->vertex1 = vertex1;
		this->vertex2 = vertex2;
	}

	int get_vertex0() {
		return vertex0;
	}

	int get_vertex1() {
		return vertex1;
	}

	int get_vertex2() {
		return vertex2;
	}

private:

	int vertex0;
	int vertex1;
	int vertex2;

};

#endif // _FACE_H_