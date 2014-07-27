#pragma once

#include "../EDXPrerequisites.h"
#include "../Math/Vector.h"
#include "Color.h"
#include <set>

#define MAX_PATH 260

namespace EDX
{
	struct MeshVertex
	{
		Vector3 position;
		Vector3 normal;
		float fU, fV;

		MeshVertex() {}
		MeshVertex(const Vector3& pos,
			const Vector3& norm,
			float u,
			float v)
			: position(pos)
			, normal(norm)
			, fU(u)
			, fV(v)
		{
		}
	};
	struct MeshFace
	{
		int aiIndices[3];
		int iSmoothingGroup;
	};
	struct CacheEntry
	{
		uint iIndex;
		CacheEntry* pNext;
	};
	struct ObjMaterial
	{
		char strName[MAX_PATH];
		char strTexturePath[MAX_PATH];
		Color color;

		ObjMaterial(const char* name)
			: color(0.9f, 0.9f, 0.9f)
			//, strTexturePath("")
		{
			strcpy_s(strName, MAX_PATH, name);
			memset(strTexturePath, 0, MAX_PATH);
		}

		bool operator == (const ObjMaterial& rhs) const
		{
			return strcmp(strName, rhs.strName) == 0;
		}
	};

	class ObjMesh
	{
	protected:
		uint mVertexCount, mTriangleCount;

		vector<MeshVertex> mVertices;
		vector<uint> mIndices;
		vector<MeshFace> mFaces;
		vector<CacheEntry*> mCache;

		vector<ObjMaterial> mMaterials;
		vector<uint> mMaterialIdx;

		bool mbNormaled;
		bool mbTextured;

	public:
		ObjMesh()
			: mVertexCount(0)
			, mTriangleCount(0)
			, mbNormaled(false)
			, mbTextured(false)
		{
		}

		bool LoadFromObj(const Vector3& ptPos,
			const Vector3& vScl,
			const Vector3& vRot,
			const char* strPath);
		void LoadPlane(const Vector3& ptPos,
			const Vector3& vScl,
			const Vector3& vRot,
			const float fLength);
		void LoadSphere(const Vector3& ptPos,
			const Vector3& vScl,
			const Vector3& vRot,
			const float fRadius);

		void ComputeVertexNormals();
		void LoadMaterialsFromMtl(const char* strPath);
		uint AddVertex(uint iHash, const MeshVertex* pVertex);

		inline const uint* GetIndexAt(int iNum) const { return mIndices.data() + 3 * iNum; }
		inline const MeshVertex& GetVertexAt(int iIndex) const { return mVertices[iIndex]; }
		inline uint GetVertexCount() const { return mVertexCount; }
		inline uint GetTriangleCount() const { return mTriangleCount; }
		inline bool IsNormaled() const { return mbNormaled; }
		inline bool IsTextured() const { return mbTextured; }

		const vector<ObjMaterial>& GetMaterialInfo() const { return mMaterials; }
		inline uint GetMaterialIdx(int iTri) const { return mMaterialIdx[iTri]; }

		void Release();
	};
}