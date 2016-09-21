#pragma once

#include "../Core/Types.h"
#include "../Math/Vector.h"
#include "../Math/BoundingBox.h"
#include "Color.h"

#include "../Containers/Array.h"
#include "../Core/CString.h"

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
		char strBumpPath[MAX_PATH];
		Color color;
		Color specColor;
		Color transColor;
		float bumpScale;

		ObjMaterial(const char* name = "")
			: color(0.85f, 0.85f, 0.85f)
			, specColor(0.0f, 0.0f, 0.0f)
			, transColor(0.0f, 0.0f, 0.0f)
			, bumpScale(1.0f)
		{
			CStringUtil::Strcpy(strName, MAX_PATH, name);
			Memory::Memset(strTexturePath, 0, MAX_PATH);
			Memory::Memset(strBumpPath, 0, MAX_PATH);
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

		Array<MeshVertex> mVertices;
		Array<uint> mIndices;
		Array<MeshFace> mFaces;
		Array<CacheEntry*> mCache;

		Array<ObjMaterial> mMaterials;
		Array<uint> mMaterialIdx;
		Array<uint> mSubsetStartIdx;
		Array<uint> mSubsetMtlIdx;
		uint mNumSubsets;

		BoundingBox mBounds;

		bool mNormaled;
		bool mTextured;

	public:
		ObjMesh()
			: mVertexCount(0)
			, mTriangleCount(0)
			, mNormaled(false)
			, mTextured(false)
		{
		}

		bool LoadFromObj(const Vector3& pos,
			const Vector3& scl,
			const Vector3& rot,
			const char* path,
			const bool forceComputeNormal = false,
			const bool makeLeftHanded = true);
		void LoadPlane(const Vector3& pos,
			const Vector3& scl,
			const Vector3& rot,
			const float length);
		void LoadSphere(const Vector3& pos,
			const Vector3& scl,
			const Vector3& rot,
			const float radius,
			const int slices = 64,
			const int stacks = 64);

		void ComputeVertexNormals();
		void LoadMaterialsFromMtl(const char* path);
		uint AddVertex(uint iHash, const MeshVertex* pVertex);

		inline const uint* GetIndexAt(int iNum) const { return mIndices.Data() + 3 * iNum; }
		inline const MeshVertex& GetVertexAt(int iIndex) const { return mVertices[iIndex]; }
		inline uint GetVertexCount() const { return mVertexCount; }
		inline uint GetTriangleCount() const { return mTriangleCount; }
		inline bool IsNormaled() const { return mNormaled; }
		inline bool IsTextured() const { return mTextured; }

		const Array<ObjMaterial>& GetMaterialInfo() const { return mMaterials; }
		inline const Array<uint>& GetMaterialIdxBuf() const { return mMaterialIdx; }
		inline uint GetMaterialIdx(int iTri) const { return mMaterialIdx[iTri]; }

		const uint GetSubsetCount() const { return mNumSubsets; }
		const uint GetSubsetStartIdx(int setIdx) { return mSubsetStartIdx[setIdx]; }
		const uint GetSubsetMtlIndex(int setIdx) { return mSubsetMtlIdx[setIdx]; }

		inline const BoundingBox GetBounds() const { return mBounds; }

		void Release();
	};
}