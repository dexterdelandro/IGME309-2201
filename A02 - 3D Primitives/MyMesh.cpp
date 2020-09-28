#include "MyMesh.h"
void MyMesh::Init(void)
{
	m_bBinded = false;
	m_uVertexCount = 0;

	m_VAO = 0;
	m_VBO = 0;

	m_pShaderMngr = ShaderManager::GetInstance();
}
void MyMesh::Release(void)
{
	m_pShaderMngr = nullptr;

	if (m_VBO > 0)
		glDeleteBuffers(1, &m_VBO);

	if (m_VAO > 0)
		glDeleteVertexArrays(1, &m_VAO);

	m_lVertex.clear();
	m_lVertexPos.clear();
	m_lVertexCol.clear();
}
MyMesh::MyMesh()
{
	Init();
}
MyMesh::~MyMesh() { Release(); }
MyMesh::MyMesh(MyMesh& other)
{
	m_bBinded = other.m_bBinded;

	m_pShaderMngr = other.m_pShaderMngr;

	m_uVertexCount = other.m_uVertexCount;

	m_VAO = other.m_VAO;
	m_VBO = other.m_VBO;
}
MyMesh& MyMesh::operator=(MyMesh& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyMesh temp(other);
		Swap(temp);
	}
	return *this;
}
void MyMesh::Swap(MyMesh& other)
{
	std::swap(m_bBinded, other.m_bBinded);
	std::swap(m_uVertexCount, other.m_uVertexCount);

	std::swap(m_VAO, other.m_VAO);
	std::swap(m_VBO, other.m_VBO);

	std::swap(m_lVertex, other.m_lVertex);
	std::swap(m_lVertexPos, other.m_lVertexPos);
	std::swap(m_lVertexCol, other.m_lVertexCol);

	std::swap(m_pShaderMngr, other.m_pShaderMngr);
}
void MyMesh::CompleteMesh(vector3 a_v3Color)
{
	uint uColorCount = m_lVertexCol.size();
	for (uint i = uColorCount; i < m_uVertexCount; ++i)
	{
		m_lVertexCol.push_back(a_v3Color);
	}
}
void MyMesh::AddVertexPosition(vector3 a_v3Input)
{
	m_lVertexPos.push_back(a_v3Input);
	m_uVertexCount = m_lVertexPos.size();
}
void MyMesh::AddVertexColor(vector3 a_v3Input)
{
	m_lVertexCol.push_back(a_v3Input);
}
void MyMesh::CompileOpenGL3X(void)
{
	if (m_bBinded)
		return;

	if (m_uVertexCount == 0)
		return;

	CompleteMesh();

	for (uint i = 0; i < m_uVertexCount; i++)
	{
		//Position
		m_lVertex.push_back(m_lVertexPos[i]);
		//Color
		m_lVertex.push_back(m_lVertexCol[i]);
	}
	glGenVertexArrays(1, &m_VAO);//Generate vertex array object
	glGenBuffers(1, &m_VBO);//Generate Vertex Buffered Object

	glBindVertexArray(m_VAO);//Bind the VAO
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);//Bind the VBO
	glBufferData(GL_ARRAY_BUFFER, m_uVertexCount * 2 * sizeof(vector3), &m_lVertex[0], GL_STATIC_DRAW);//Generate space for the VBO

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)0);

	// Color attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)(1 * sizeof(vector3)));

	m_bBinded = true;

	glBindVertexArray(0); // Unbind VAO
}
void MyMesh::Render(matrix4 a_mProjection, matrix4 a_mView, matrix4 a_mModel)
{
	// Use the buffer and shader
	GLuint nShader = m_pShaderMngr->GetShaderID("Basic");
	glUseProgram(nShader); 

	//Bind the VAO of this object
	glBindVertexArray(m_VAO);

	// Get the GPU variables by their name and hook them to CPU variables
	GLuint MVP = glGetUniformLocation(nShader, "MVP");
	GLuint wire = glGetUniformLocation(nShader, "wire");

	//Final Projection of the Camera
	matrix4 m4MVP = a_mProjection * a_mView * a_mModel;
	glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(m4MVP));
	
	//Solid
	glUniform3f(wire, -1.0f, -1.0f, -1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);  

	//Wire
	glUniform3f(wire, 1.0f, 0.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.f, -1.f);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);
	glDisable(GL_POLYGON_OFFSET_LINE);

	glBindVertexArray(0);// Unbind VAO so it does not get in the way of other objects
}
void MyMesh::AddTri(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft)
{
	//C
	//| \
	//A--B
	//This will make the triangle A->B->C 
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);
}
void MyMesh::AddQuad(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft, vector3 a_vTopRight)
{
	//C--D
	//|  |
	//A--B
	//This will make the triangle A->B->C and then the triangle C->B->D
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);

	AddVertexPosition(a_vTopLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopRight);
}
void MyMesh::GenerateCube(float a_fSize, vector3 a_v3Color)
{
	if (a_fSize < 0.01f)
		a_fSize = 0.01f;

	Release();
	Init();

	float fValue = a_fSize * 0.5f;
	//3--2
	//|  |
	//0--1

	vector3 point0(-fValue,-fValue, fValue); //0
	vector3 point1( fValue,-fValue, fValue); //1
	vector3 point2( fValue, fValue, fValue); //2
	vector3 point3(-fValue, fValue, fValue); //3

	vector3 point4(-fValue,-fValue,-fValue); //4
	vector3 point5( fValue,-fValue,-fValue); //5
	vector3 point6( fValue, fValue,-fValue); //6
	vector3 point7(-fValue, fValue,-fValue); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCuboid(vector3 a_v3Dimensions, vector3 a_v3Color)
{
	Release();
	Init();

	vector3 v3Value = a_v3Dimensions * 0.5f;
	//3--2
	//|  |
	//0--1
	vector3 point0(-v3Value.x, -v3Value.y, v3Value.z); //0
	vector3 point1(v3Value.x, -v3Value.y, v3Value.z); //1
	vector3 point2(v3Value.x, v3Value.y, v3Value.z); //2
	vector3 point3(-v3Value.x, v3Value.y, v3Value.z); //3

	vector3 point4(-v3Value.x, -v3Value.y, -v3Value.z); //4
	vector3 point5(v3Value.x, -v3Value.y, -v3Value.z); //5
	vector3 point6(v3Value.x, v3Value.y, -v3Value.z); //6
	vector3 point7(-v3Value.x, v3Value.y, -v3Value.z); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCone(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	GLfloat angle = (2.0f * PI / a_nSubdivisions);
	float halfHeight = a_fHeight / 2.0f;
	std::vector<vector3> vertices;

	//calculates and adds vertices
	for (int i = 0; i < a_nSubdivisions; i++) {
		vertices.push_back(vector3(cos(angle*i)* a_fRadius, halfHeight, sin(angle * i) * a_fRadius));
	}

	//draw tri's
	for (int i = 0; i < a_nSubdivisions; i++) {
		AddTri(vertices[(i + 1) % a_nSubdivisions], vertices[i], vector3(0, halfHeight,0));
		AddTri(vertices[i], vertices[(i + 1) % a_nSubdivisions], vector3(0, -halfHeight, 0));
	}


	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCylinder(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	GLfloat angle = (2.0f * PI / a_nSubdivisions);
	float halfHeight = a_fHeight / 2.0f;

	std::vector<vector3> topVertices; //verticies on top
	std::vector<vector3> bottomVertices; //verticies on bottom

	//fill up vertices
	for (int i = 0; i < a_nSubdivisions; i++) {
		topVertices.push_back(vector3(cos(angle*i)*a_fRadius, halfHeight, sin(angle*i)* a_fRadius));
		bottomVertices.push_back(vector3(cos(angle * i) * a_fRadius, -halfHeight, sin(angle * i) * a_fRadius));
	}

	//draw the tri's
	for (int i = 0; i < a_nSubdivisions; i++) {
		AddTri(bottomVertices[i], bottomVertices[(i + 1) % a_nSubdivisions], vector3(0, -halfHeight, 0));
		AddTri(topVertices[(i + 1) % a_nSubdivisions], topVertices[i], vector3(0, halfHeight, 0));
		AddQuad(bottomVertices[(i + 1) % a_nSubdivisions], bottomVertices[i], topVertices[(i + 1) % a_nSubdivisions], topVertices[i]);
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTube(float a_fOuterRadius, float a_fInnerRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	GLfloat angle = (2.0f * PI / a_nSubdivisions);
	float halfHeight = a_fHeight / 2.0f;

	std::vector<vector3> outerTopVerticies; //outer top vertices
	std::vector<vector3> innerTopVerticies; //inner top vertices
	std::vector<vector3> outerBottomVerticies; //outer bottom vertices
	std::vector<vector3> innerBottomVerticies; //inner bottom vertices

	//fill up vertices
	for (int i = 0; i < a_nSubdivisions; i++) {

		outerTopVerticies.push_back(vector3(cos(angle * i) * a_fOuterRadius, halfHeight, sin(angle * i) * a_fOuterRadius));
		innerTopVerticies.push_back(vector3(cos(angle * i) * a_fInnerRadius, halfHeight, sin(angle * i) * a_fInnerRadius));
		outerBottomVerticies.push_back(vector3(cos(angle * i) * a_fOuterRadius, -halfHeight, sin(angle * i) * a_fOuterRadius));
		innerBottomVerticies.push_back(vector3(cos(angle * i) * a_fInnerRadius, -halfHeight, sin(angle * i) * a_fInnerRadius));
	}
	
	//draw tri's
	for (int i = 0; i < a_nSubdivisions; i++) {
		AddQuad(outerBottomVerticies[i], outerBottomVerticies[(i + 1) % a_nSubdivisions], innerBottomVerticies[i], innerBottomVerticies[(i + 1) % a_nSubdivisions]);
		AddQuad(outerTopVerticies[(i + 1) % a_nSubdivisions], outerTopVerticies[i], innerTopVerticies[(i + 1) % a_nSubdivisions], innerTopVerticies[i]);
		AddQuad(outerBottomVerticies[(i + 1) % a_nSubdivisions], outerBottomVerticies[i], outerTopVerticies[(i + 1) % a_nSubdivisions], outerTopVerticies[i]);
		AddQuad(innerBottomVerticies[i], innerBottomVerticies[(i + 1) % a_nSubdivisions], innerTopVerticies[i], innerTopVerticies[(i + 1) % a_nSubdivisions]);
	}
	
	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTorus(float a_fOuterRadius, float a_fInnerRadius, int a_nSubdivisionsA, int a_nSubdivisionsB, vector3 a_v3Color)
{

	
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_nSubdivisionsA < 3)
		a_nSubdivisionsA = 3;
	if (a_nSubdivisionsA > 360)
		a_nSubdivisionsA = 360;

	if (a_nSubdivisionsB < 3)
		a_nSubdivisionsB = 3;
	if (a_nSubdivisionsB > 360)
		a_nSubdivisionsB = 360;

	Release();
	Init();

	//set up variables so looping is easier
	float diffRad = (a_fOuterRadius - a_fInnerRadius) / 2;
	float sumRad = (a_fOuterRadius + a_fInnerRadius) / 2;
	float outerAngle = 2.0f * PI / a_nSubdivisionsA;
	float innerAngle = 2.0f * PI / a_nSubdivisionsB;
	float x;
	float y;
	float z;

	//set up vertices list
	std::vector<std::vector<vector3>> vertices;
	for (int i = 0; i < a_nSubdivisionsB; i++) vertices.push_back(std::vector<vector3>());

	//caluclate and add verticies to list
	for (int i = 0; i < a_nSubdivisionsA; i++) {
		for (int j = 0; j < a_nSubdivisionsB; j++) {
			//parametric equation for torus
			x = (sumRad + (diffRad * cos(outerAngle * i))) * cos(innerAngle * j);
			y = (sumRad + (diffRad * cos(outerAngle * i))) * sin(innerAngle * j);
			z = diffRad * sin(outerAngle * i);
			vertices[j].push_back(vector3(x, y, z));
		}
	}

	//draw quads
	for (int i = 0; i < a_nSubdivisionsA; i++) {
		for (int j = 0; j < a_nSubdivisionsB; j++) {
			AddQuad(vertices[j][i], vertices[j][(i+1) % a_nSubdivisionsA], vertices[(j+1) % a_nSubdivisionsB][i], vertices[(j + 1) % a_nSubdivisionsB][(i + 1) % a_nSubdivisionsA]);
		}
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateSphere(float a_fRadius, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	//Sets minimum and maximum of subdivisions
	if (a_nSubdivisions < 1)
	{
		GenerateCube(a_fRadius * 2.0f, a_v3Color);
		return;
	}
	if (a_nSubdivisions > 6)
		a_nSubdivisions = 6;

	Release();
	Init();

	
	std::vector<vector3> v1;
	std::vector<vector3> v2;
	std::vector<vector3> v3;
	std::vector<vector3> v4;

	for (int i = 0; i < a_nSubdivisions; i++)
	{
		GLfloat t1 = PI*((GLfloat)i /a_nSubdivisions);
		GLfloat t2 = PI*((GLfloat)(i + 1)/a_nSubdivisions);

		for (int j = 0; j < a_nSubdivisions; j++)
		{
			GLfloat p1 = 2.0f*PI*((GLfloat)j /a_nSubdivisions);
			GLfloat p2 = 2.0f*PI*((GLfloat)(j + 1) /a_nSubdivisions);

			v1.push_back(vector3(sin(t1) * cos(p1), sin(t1) * sin(p1), cos(t1)) * a_fRadius);
			v2.push_back(vector3(sin(t1) * cos(p2), sin(t1) * sin(p2), cos(t1)) * a_fRadius);
			v3.push_back(vector3(sin(t2) * cos(p2), sin(t2) * sin(p2), cos(t2)) * a_fRadius);
			v4.push_back(vector3(sin(t2) * cos(p1), sin(t2) * sin(p1), cos(t2)) * a_fRadius);
		}
	}

	//draw the tri's and quads
	AddTri(v4[0],v3[0], v1[0]);//top tri
	for (int i = 0; i < v1.size(); i++) {
		AddQuad(v4[i], v3[i], v1[i], v2[i]); //middle quads
	}
	AddTri(v2[a_nSubdivisions-1], v1[a_nSubdivisions - 1], v3[a_nSubdivisions - 1]); //bottom tri

	

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}