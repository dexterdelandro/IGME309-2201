#include "MyOctant.h"
using namespace Simplex;

uint MyOctant::m_uOctantCount = 0;
uint MyOctant::m_uMaxLevel = 3;
uint MyOctant::m_uIdealEntityCount = 5;

Simplex::MyOctant::MyOctant(uint a_nMaxLevel, uint a_nIdealEntityCount)
{
	//set initial variables
	Init();
	m_uOctantCount = 0;
	m_uMaxLevel = a_nMaxLevel;
	m_uIdealEntityCount = a_nIdealEntityCount;
	m_uID = m_uOctantCount;
	m_pRoot = this;
	m_lChild.clear();

	std::vector<vector3> boundingObjs;

	uint numObj = m_pEntityMngr->GetEntityCount();
	for (int i = 0; i < numObj; i++){
		MyEntity* pEntity = m_pEntityMngr->GetEntity(i);

		MyRigidBody* pRigidBody = pEntity->GetRigidBody();

		boundingObjs.push_back(pRigidBody->GetMinGlobal());
		boundingObjs.push_back(pRigidBody->GetMaxGlobal());
	}

	//set new variables
	MyRigidBody* pRigidBody = new MyRigidBody(boundingObjs);
	vector3 vHalfWidth = pRigidBody->GetHalfWidth();
	float halfWMax = vHalfWidth.x;

	//don't count [0]
	for (int i = 1; i < 3; i++){
		if (halfWMax < vHalfWidth[i]) halfWMax = vHalfWidth[i];
	}

	m_v3Center = pRigidBody->GetCenterLocal();
	boundingObjs.clear();
	SafeDelete(pRigidBody);

	m_fSize = halfWMax * 2.0f;
	//get minmax by subtracting/adding max of halfwidth to center
	m_v3Min = m_v3Center - (vector3(halfWMax));
	m_v3Max = m_v3Center + (vector3(halfWMax));

	m_uOctantCount++;

	ConstructTree(m_uMaxLevel);
}

Simplex::MyOctant::MyOctant(vector3 a_v3Center, float a_fSize)
{
	Init();
	m_v3Center = a_v3Center;
	m_fSize = a_fSize;
	//divide by 2 cuz halfwidth
	m_v3Max = m_v3Center + (vector3(m_fSize) / 2.0f);
	m_v3Min = m_v3Center - (vector3(m_fSize) / 2.0f);
	m_uOctantCount++;
}

Simplex::MyOctant::MyOctant(MyOctant const& other)
{
	//just other's variables to this object's one
	m_uChildren = other.m_uChildren;
	m_v3Center = other.m_v3Center;
	m_fSize = other.m_fSize;
	m_uID = other.m_uID;
	m_uLevel = other.m_uLevel;
	m_pParent = other.m_pParent;
	m_pRoot = other.m_pRoot;
	m_lChild = other.m_lChild;
	m_v3Max = other.m_v3Max;
	m_v3Min = other.m_v3Min;
	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	for (int i = 0; i < 8; i++) m_pChild[i], other.m_pChild[i];
}

MyOctant& Simplex::MyOctant::operator=(MyOctant const& other)
{
	if(this != &other){
		Release();
		Init();
		MyOctant temp(other);
		Swap(temp);
	}
	return *this;
}

Simplex::MyOctant::~MyOctant(void)
{
	Release();
}

void Simplex::MyOctant::Swap(MyOctant& other)
{
	//swap all variables with other
	std::swap(m_uChildren, other.m_uChildren);
	std::swap(m_v3Center, other.m_v3Center);
	std::swap(m_fSize, other.m_fSize);
	std::swap(m_uID, other.m_uID);
	std::swap(m_uLevel, other.m_uLevel);
	std::swap(m_pParent, other.m_pParent);
	std::swap(m_pRoot, other.m_pRoot);
	std::swap(m_lChild, other.m_lChild);
	std::swap(m_v3Min, other.m_v3Min);
	std::swap(m_v3Max, other.m_v3Max);

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();
	for (int i = 0; i < 8; i++){
		std::swap(m_pChild[i], other.m_pChild[i]);
	}
}

float Simplex::MyOctant::GetSize(void)
{
	return m_fSize;
}

vector3 Simplex::MyOctant::GetCenterGlobal(void)
{
	return m_v3Center;
}

vector3 Simplex::MyOctant::GetMinGlobal(void)
{
	return m_v3Min;
}

vector3 Simplex::MyOctant::GetMaxGlobal(void)
{
	return m_v3Max;
}

bool Simplex::MyOctant::IsColliding(uint a_uRBIndex)
{
	uint nObjectCount = m_pEntityMngr->GetEntityCount();
	if (a_uRBIndex >= nObjectCount)
	{
		return false;
	}
	MyEntity* pEntity = m_pEntityMngr->GetEntity(a_uRBIndex);
	MyRigidBody* pRigidBody = pEntity->GetRigidBody();
	vector3 v3Max = pRigidBody->GetMaxGlobal();
	vector3 v3Min = pRigidBody->GetMinGlobal();

	//check collision by checking min max pos in all 3 axis
	//x
	if (m_v3Max.x < v3Min.x) return false;
	if (m_v3Min.x > v3Max.x) return false;

	//y
	if (m_v3Max.y < v3Min.y) return false;
	if (m_v3Min.y > v3Max.y) return false;

	//z
	if (m_v3Max.z < v3Min.z) return false;
	if (m_v3Min.z > v3Max.z) return false;

	//true if it gets this far
	return true;

}

void Simplex::MyOctant::Display(uint a_nIndex, vector3 a_v3Color)
{
	if (m_uID == a_nIndex){
		m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center)*glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
		//dont call children's display
		return;
	}

	//call all children's display function
	for (int i = 0; i < m_uChildren; i++) m_pChild[i]->Display(a_nIndex);
}

void Simplex::MyOctant::Display(vector3 a_v3Color)
{
	//call children display
	for (int i = 0; i < m_uChildren; i++)m_pChild[i]->Display(a_v3Color);

	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center)*glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
}

void Simplex::MyOctant::DisplayLeafs(vector3 a_v3Color)
{
	uint numLeaves = m_lChild.size();
	for (int i = 0; i < numLeaves; i++) m_lChild[i]->DisplayLeafs(a_v3Color);
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center)*glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
}

void Simplex::MyOctant::ClearEntityList(void)
{
	for (int i = 0; i < m_uChildren; i++) m_pChild[i]->ClearEntityList();
	m_EntityList.clear();
}

void Simplex::MyOctant::Subdivide(void)
{
	//don't subdivide if level is max or no children
	if (m_uLevel >= m_uMaxLevel) return;
	if (m_uChildren != 0) return;
	m_uChildren = 8;

	float size = m_fSize / 4.0f;

	vector3 v3Center;

	//create all the octants 

	//octant 0 - low left back
	v3Center = m_v3Center;
	v3Center.x -= size;
	v3Center.y -= size;
	v3Center.z -= size;
	m_pChild[0] = new MyOctant(v3Center, size * 2.0f);

	// 1 - low right back
	v3Center.x += size * 2.0f;
	m_pChild[1] = new MyOctant(v3Center, size * 2.0f);

	// 2 - low right front
	v3Center.z += size * 2.0f;
	m_pChild[2] = new MyOctant(v3Center, size * 2.0f);

	// 3 - low left front
	v3Center.x -= size * 2.0f;
	m_pChild[3] = new MyOctant(v3Center, size * 2.0f);

	// 4 - top left front
	v3Center.y += size * 2.0f;
	m_pChild[4] = new MyOctant(v3Center, size * 2.0f);

	// 5 - top left back
	v3Center.z -= size * 2.0f;
	m_pChild[5] = new MyOctant(v3Center, size * 2.0f);

	// 6 - top right back
	v3Center.x += size * 2.0f;
	m_pChild[6] = new MyOctant(v3Center, size * 2.0f);

	// 7 - top right back
	v3Center.z += size * 2.0f;
	m_pChild[7] = new MyOctant(v3Center, size * 2.0f);

	for (int i = 0; i < 8; i++)
	{
		//set variables
		m_pChild[i]->m_pRoot = m_pRoot;
		m_pChild[i]->m_pParent = this;
		m_pChild[i]->m_uLevel = m_uLevel+1;

		//subdivide if space
		if (m_pChild[i]->ContainsMoreThan(m_uIdealEntityCount))
		{
			m_pChild[i]->Subdivide();
		}
	}
}

MyOctant* Simplex::MyOctant::GetChild(uint a_nChild)
{
	//deny if input is over 7
	if (a_nChild>7)return nullptr;

	return m_pChild[a_nChild];
}

MyOctant* Simplex::MyOctant::GetParent(void)
{
	return m_pParent;
}

bool Simplex::MyOctant::IsLeaf(void)
{
	//leaf if no children
	if(m_uChildren==0)return true;

	return false;
}

bool Simplex::MyOctant::ContainsMoreThan(uint a_nEntities)
{
	int counter = 0;
	uint numObj = m_pEntityMngr->GetEntityCount();

	for (int i = 0; i < numObj; i++){
		if (IsColliding(i)) counter++;

		if (counter > a_nEntities) return true;
	}
	return false;
}

void Simplex::MyOctant::KillBranches(void)
{
	for (int i = 0; i < m_uChildren; i++){
		//have each child kill their own branches
		m_pChild[i]->KillBranches();

		//delete this child
		delete m_pChild[i];
		m_pChild[i] = nullptr;
	}

	m_uChildren = 0;
}

void Simplex::MyOctant::ConstructTree(uint a_nMaxLevel)
{
	if (m_uLevel != 0) return;

	m_uMaxLevel = a_nMaxLevel;
	m_uOctantCount = 1;

	//empty lists and children
	m_EntityList.clear();
	KillBranches();
	m_lChild.clear();

	if (ContainsMoreThan(m_uIdealEntityCount))
	{
		Subdivide();
	}
	AssignIDtoEntity();
	ConstructList();
}

void Simplex::MyOctant::AssignIDtoEntity(void)
{
	for (int i = 0; i < m_uChildren; i++) {
		m_pChild[i]->AssignIDtoEntity();
	}

	if (m_uChildren == 0) {
		int numEntities = m_pEntityMngr->GetEntityCount();

		for (int i = 0; i < numEntities; i++) {
			if (IsColliding(i)) {
				m_EntityList.push_back(i);
				m_pEntityMngr->AddDimension(i, m_uID);
			}
		}
	}
}

uint Simplex::MyOctant::GetOctantCount(void)
{
	return m_uOctantCount;
}

void Simplex::MyOctant::Release(void)
{
	if (m_uLevel == 0) KillBranches(); //kill it

	//otherwise reset
	m_uChildren = 0;
	m_fSize = 0.0;
	m_EntityList.clear();
	m_lChild.clear();
}

void Simplex::MyOctant::Init(void)
{
	//set everything to default values
	m_uChildren = 0;
	m_fSize = 0.0f;
	m_uID = m_uOctantCount;
	m_uLevel = 0;

	m_v3Center = vector3(0.0f);
	m_v3Min = vector3(0.0f);
	m_v3Max = vector3(0.0f);

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	m_pRoot = nullptr;
	m_pParent = nullptr;
	for (int i = 0; i < 8; i++)m_pChild[i] = nullptr;
}

void Simplex::MyOctant::ConstructList(void)
{
	for (int i = 0; i < m_uChildren; i++) {
		m_pChild[i]->ConstructList();
	}
	
	if (m_EntityList.size() > 0) {
		m_pRoot->m_lChild.push_back(this);
	}
}
