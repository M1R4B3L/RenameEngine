
#include "Globals.h"
#include "GL/glew.h"
#include "Primitive.h"
#include "Dependencies/MathGeolib/MathGeoLib.h"

// ------------------------------------------------------------
Primitive::Primitive() : transform(float4x4::identity), color(White), wire(false), axis(false), type(PrimitiveTypes::Primitive_Point)
{}

// ------------------------------------------------------------
PrimitiveTypes Primitive::GetType() const
{
	return type;
}

// ------------------------------------------------------------
void Primitive::Render() const
{
	glPushMatrix();
	glMultMatrixf(transform.ptr());

	if(axis == true)
	{
		// Draw Axis Grid
		glLineWidth(2.0f);

		glBegin(GL_LINES);

		glColor4f(1.0f, 0.0f, 0.0f, 0.25f);

		glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(1.0f, 0.0f, 0.0f);
		glVertex3f(1.0f, 0.1f, 0.0f); glVertex3f(1.1f, -0.1f, 0.0f);
		glVertex3f(1.1f, 0.1f, 0.0f); glVertex3f(1.0f, -0.1f, 0.0f);

		glColor4f(0.0f, 1.0f, 0.0f, 1.0f);

		glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, 1.0f, 0.0f);
		glVertex3f(-0.05f, 1.25f, 0.0f); glVertex3f(0.0f, 1.15f, 0.0f);
		glVertex3f(0.05f, 1.25f, 0.0f); glVertex3f(0.0f, 1.15f, 0.0f);
		glVertex3f(0.0f, 1.15f, 0.0f); glVertex3f(0.0f, 1.05f, 0.0f);

		glColor4f(0.0f, 0.0f, 1.0f, 1.0f);

		glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, 0.0f, 1.0f);
		glVertex3f(-0.05f, 0.1f, 1.05f); glVertex3f(0.05f, 0.1f, 1.05f);
		glVertex3f(0.05f, 0.1f, 1.05f); glVertex3f(-0.05f, -0.1f, 1.05f);
		glVertex3f(-0.05f, -0.1f, 1.05f); glVertex3f(0.05f, -0.1f, 1.05f);

		glEnd();

		glLineWidth(1.0f);
	}

	glColor3f(color.r, color.g, color.b);

	InnerRender();

	glPopMatrix();
}

// ------------------------------------------------------------
void Primitive::InnerRender() const
{
	glPointSize(5.0f);

	glBegin(GL_POINTS);

	glVertex3f(0.0f, 0.0f, 0.0f);

	glEnd();

	glPointSize(1.0f);
}

// ------------------------------------------------------------
void Primitive::SetPos(float x, float y, float z)
{
	transform.Translate(x, y, z);
}

// ------------------------------------------------------------
void Primitive::SetRotation(float angle, const float3 &u)
{
	transform.RotateAxisAngle(u, angle);
}

// ------------------------------------------------------------
void Primitive::Scale(float x, float y, float z)
{
	transform.Scale(x, y, z);
}

// PCube ============================================
PCube::PCube() : Primitive(), size(1.0f, 1.0f, 1.0f)
{
	type = PrimitiveTypes::Primitive_PCube;
}

PCube::PCube(float sizeX, float sizeY, float sizeZ) : Primitive(), size(sizeX, sizeY, sizeZ)
{
	type = PrimitiveTypes::Primitive_PCube;
}

void PCube::InnerRender() const
{	
	float sx = size.x * 0.5f;
	float sy = size.y * 0.5f;
	float sz = size.z * 0.5f;

	glBegin(GL_QUADS);

	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-sx, -sy, sz);
	glVertex3f( sx, -sy, sz);
	glVertex3f( sx,  sy, sz);
	glVertex3f(-sx,  sy, sz);

	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f( sx, -sy, -sz);
	glVertex3f(-sx, -sy, -sz);
	glVertex3f(-sx,  sy, -sz);
	glVertex3f( sx,  sy, -sz);

	glNormal3f(1.0f, 0.0f, 0.0f);
	glVertex3f(sx, -sy,  sz);
	glVertex3f(sx, -sy, -sz);
	glVertex3f(sx,  sy, -sz);
	glVertex3f(sx,  sy,  sz);

	glNormal3f(-1.0f, 0.0f, 0.0f);
	glVertex3f(-sx, -sy, -sz);
	glVertex3f(-sx, -sy,  sz);
	glVertex3f(-sx,  sy,  sz);
	glVertex3f(-sx,  sy, -sz);

	glNormal3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-sx, sy,  sz);
	glVertex3f( sx, sy,  sz);
	glVertex3f( sx, sy, -sz);
	glVertex3f(-sx, sy, -sz);

	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f(-sx, -sy, -sz);
	glVertex3f( sx, -sy, -sz);
	glVertex3f( sx, -sy,  sz);
	glVertex3f(-sx, -sy,  sz);

	glEnd();
}

// PSphere ============================================
PSphere::PSphere() : Primitive(), radius(1.0f)
{
	type = PrimitiveTypes::Primitive_PSphere;
}

PSphere::PSphere(float radius) : Primitive(), radius(radius)
{
	type = PrimitiveTypes::Primitive_PSphere;
}

/*void PSphere::InnerRender() const
{
	glutSolidPSphere(radius, 25, 25);
}*/


// PCylinder ============================================
PCylinder::PCylinder() : Primitive(), radius(1.0f), height(1.0f)
{
	type = PrimitiveTypes::Primitive_PCylinder;
}

PCylinder::PCylinder(float radius, float height) : Primitive(), radius(radius), height(height)
{
	type = PrimitiveTypes::Primitive_PCylinder;
}

void PCylinder::InnerRender() const
{
	int n = 30;

	// PCylinder Bottom
	glBegin(GL_POLYGON);
	
	for(int i = 360; i >= 0; i -= (360 / n))
	{
		float a = i * pi / 180; // degrees to radians
		glVertex3f(-height*0.5f, radius * cos(a), radius * sin(a));
	}
	glEnd();

	// PCylinder Top
	glBegin(GL_POLYGON);
	glNormal3f(0.0f, 0.0f, 1.0f);
	for(int i = 0; i <= 360; i += (360 / n))
	{
		float a = i * pi / 180; // degrees to radians
		glVertex3f(height * 0.5f, radius * cos(a), radius * sin(a));
	}
	glEnd();

	// PCylinder "Cover"
	glBegin(GL_QUAD_STRIP);
	for(int i = 0; i < 480; i += (360 / n))
	{
		float a = i * pi / 180; // degrees to radians

		glVertex3f(height*0.5f,  radius * cos(a), radius * sin(a) );
		glVertex3f(-height*0.5f, radius * cos(a), radius * sin(a) );
	}
	glEnd();
}

// PLine ==================================================
PLine::PLine() : Primitive(), origin(0, 0, 0), destination(1, 1, 1)
{
	type = PrimitiveTypes::Primitive_PLine;
}

PLine::PLine(float x, float y, float z) : Primitive(), origin(0, 0, 0), destination(x, y, z)
{
	type = PrimitiveTypes::Primitive_PLine;
}

void PLine::InnerRender() const
{
	glLineWidth(2.0f);

	glBegin(GL_LINES);

	glVertex3f(origin.x, origin.y, origin.z);
	glVertex3f(destination.x, destination.y, destination.z);

	glEnd();

	glLineWidth(1.0f);
}

// PPlane ==================================================
PPlane::PPlane() : Primitive(), normal(0, 1, 0), constant(1)
{
	type = PrimitiveTypes::Primitive_PPlane;
}

PPlane::PPlane(float x, float y, float z, float d) : Primitive(), normal(x, y, z), constant(d)
{
	type = PrimitiveTypes::Primitive_PPlane;
}

void PPlane::InnerRender() const
{
	glLineWidth(1.0f);

	glBegin(GL_LINES);

	float d = 1000.0f;

	for(float i = -d; i <= d; i += 5.0f)
	{
		glVertex3f(i, 0.0f, -d);
		glVertex3f(i, 0.0f, d);
		glVertex3f(-d, 0.0f, i);
		glVertex3f(d, 0.0f, i);
	}

	glEnd();
}


DefaultPCube::DefaultPCube(): myArrayId(0), myIndexArrayId(1), IndexBuffer(2)
{
	float array[36 * 3] = { 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, -1.f, 1.f, 1.f, 0.f, 1.f, 0.f, -1.f, 1.f, 1.f, -1.f, 1.f, 1.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, -1.f, 0.f, 1.f, 0.f, 0.f, 1.f, -1.f, 0.f, 0.f, -1.f, 0.f, 1.f, -1.f, 1.f, 1.f, -1.f, 1.f, 1.f, -1.f, 1.f, 0.f, -1.f, 0.f, 0.f, -1.f, 1.f, 1.f, 0.f, 0.f, 1.f, -1.f, 0.f, 1.f, 0.f, 1.f, 1.f, 0.f, 1.f, 1.f, -1.f, 0.f, 1.f, -1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, -1.f, 1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 1.f, 0.f, -1.f };
	glGenBuffers(1, &myArrayId);
	glBindBuffer(GL_ARRAY_BUFFER, myArrayId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 36 * 3, array, GL_STATIC_DRAW);

	float iarray[8 * 3] = {
	0.f,0.f,0.f, //a 0
	1.f,0.f,0.f, //b 1
	0.f,1.f,0.f, //c 2
	1.f,1.f,0.f, //d 3
	0.f,0.f,-1.f, //e 4
	1.f,0.f,-1.f, //f 5
	0.f,1.f,-1.f, //g 6
	1.f,1.f,-1.f, //h 7
	};

	unsigned int indices[36] = {
		0, 1, 2,
		1, 3, 2,
		1, 5, 3,
		5, 7, 3,
		4, 0, 2,
		4, 2, 6,
		4, 6, 7,
		7, 5, 4,
		3, 6, 2,
		3, 7, 6,
		1, 0, 4,
		1, 4, 5
	};

	glGenBuffers(1, &myIndexArrayId);
	glBindBuffer(GL_ARRAY_BUFFER, myIndexArrayId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8 * 3, iarray, GL_STATIC_DRAW);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	glGenBuffers(1, &IndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 12 * 3, indices, GL_STATIC_DRAW);
}

DefaultPCube::~DefaultPCube()
{
}

void DefaultPCube::RenderDirect()
{
	//Drawing the PCube direct method
	glBegin(GL_TRIANGLES);
	//front
	glVertex3f(0.f, 0.f, 0.f);
	glVertex3f(1.f, 0.f, 0.f);
	glVertex3f(0.f, 1.f, 0.f);
	glVertex3f(1.f, 0.f, 0.f);
	glVertex3f(1.f, 1.f, 0.f);
	glVertex3f(0.f, 1.f, 0.f);
	//right
	glVertex3f(1.f, 0.f, 0.f);
	glVertex3f(1.f, 0.f, -1.f);
	glVertex3f(1.f, 1.f, 0.f);
	glVertex3f(1.f, 0.f, -1.f);
	glVertex3f(1.f, 1.f, -1.f);
	glVertex3f(1.f, 1.f, 0.f);
	//left
	glVertex3f(0.f, 0.f, -1.f);
	glVertex3f(0.f, 0.f, 0.f);
	glVertex3f(0.f, 1.f, 0.f);
	glVertex3f(0.f, 0.f, -1.f);
	glVertex3f(0.f, 1.f, 0.f);
	glVertex3f(0.f, 1.f, -1.f);
	//back
	glVertex3f(0.f, 0.f, -1.f);
	glVertex3f(0.f, 1.f, -1.f);
	glVertex3f(1.f, 1.f, -1.f);
	glVertex3f(1.f, 1.f, -1.f);
	glVertex3f(1.f, 0.f, -1.f);
	glVertex3f(0.f, 0.f, -1.f);
	//top
	glVertex3f(1.f, 1.f, 0.f);
	glVertex3f(0.f, 1.f, -1.f);
	glVertex3f(0.f, 1.f, 0.f);
	glVertex3f(1.f, 1.f, 0.f);
	glVertex3f(1.f, 1.f, -1.f);
	glVertex3f(0.f, 1.f, -1.f);
	//bottom
	glVertex3f(1.f, 0.f, 0.f);
	glVertex3f(0.f, 0.f, 0.f);
	glVertex3f(0.f, 0.f, -1.f);
	glVertex3f(1.f, 0.f, 0.f);
	glVertex3f(0.f, 0.f, -1.f);
	glVertex3f(1.f, 0.f, -1.f);
	glEnd();
}

void DefaultPCube::RenderArrayBuffer()
{
	//PCube using the vertex array
	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, myArrayId);
	glVertexPointer(3, GL_FLOAT, 0, NULL);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void DefaultPCube::RenderIndexBuffer()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, NULL);
	glDisableClientState(GL_VERTEX_ARRAY);
}
