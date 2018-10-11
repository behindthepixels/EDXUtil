#include "ObjMesh.h"
#include "../Math/Matrix.h"
#include "../Core/Memory.h"

namespace EDX
{
	bool ObjMesh::LoadFromObj(const Vector3& pos,
		const Vector3& scl,
		const Vector3& rot,
		const char* strPath,
		const bool forceComputeNormal,
		const bool makeLeftHanded)
	{
		Array<Vector3> positionBuf;
		Array<Vector3> normalBuf;
		Array<float> texCoordBuf;
		int iSmoothingGroup = forceComputeNormal ? 1 : 0;
		bool hasSmoothGroup = false;
		int iCurrentMtl = 0;
		char strMaterialFilename[MAX_PATH] = { 0 };

		Matrix mWorld, mWorldInv;

		Vector3 leftHandedScl = makeLeftHanded ? Vector3(-1.0f, 1.0f, 1.0f) : Vector3::UNIT_SCALE;
		Matrix::CalcTransform(pos, scl * leftHandedScl, rot, &mWorld, &mWorldInv);

		char strCommand[MAX_PATH] = { 0 };
		FILE* pInFile = 0;
		fopen_s(&pInFile, strPath, "rt");
		assert(pInFile);

		Vector3 minPt = Math::EDX_INFINITY;
		Vector3 maxPt = Math::EDX_NEG_INFINITY;
		while (!feof(pInFile))
		{
			fscanf_s(pInFile, "%s", strCommand, MAX_PATH);

			if (0 == CStringUtil::Strcmp(strCommand, "#"))
			{
				// Comment
			}
			else if (0 == CStringUtil::Strcmp(strCommand, "v"))
			{
				// Vertex Position
				float x, y, z;
				fscanf_s(pInFile, "%f %f %f", &x, &y, &z);
				positionBuf.Add(Matrix::TransformPoint(Vector3(x, y, z), mWorld));

				if (x > maxPt.x)
					maxPt.x = x;
				if (y > maxPt.y)
					maxPt.y = y;
				if (z > maxPt.z)
					maxPt.z = z;
				if (x < minPt.x)
					minPt.x = x;
				if (y < minPt.y)
					minPt.y = y;
				if (z < minPt.z)
					minPt.z = z;
			}
			else if (0 == CStringUtil::Strcmp(strCommand, "vt"))
			{
				// Vertex TexCoord
				float u, v;
				fscanf_s(pInFile, "%f %f", &u, &v);
				texCoordBuf.Add(u);
				texCoordBuf.Add(1.0f - v);
				mTextured = true;
			}
			else if (0 == CStringUtil::Strcmp(strCommand, "vn"))
			{
				// Vertex Normal
				float x, y, z;
				fscanf_s(pInFile, "%f %f %f", &x, &y, &z);
				normalBuf.Add(Matrix::TransformNormal(Vector3(x, y, z), mWorldInv));
				mNormaled = true;
			}
			else if (0 == CStringUtil::Strcmp(strCommand, "f"))
			{
				// Parse face
				fgets(strCommand, MAX_PATH, pInFile);
				int length = strlen(strCommand);

				// Face
				int posIdx, texIdx, normalIdx;
				MeshVertex Vertex;
				MeshFace Face, quadFace;

				uint faceIdx[4] = { 0, 0, 0, 0 };
				int vertexCount = 0;

				int slashCount = -1;
				bool doubleSlash = false;
				auto startIdx = 0;
				for (auto i = 0; i < length; i++)
				{
					auto c = strCommand[i];
					if (strCommand[i] != ' ' && strCommand[i] != '\t' && strCommand[i] != '\n' && strCommand[i] != '\0')
						continue;
					if (startIdx == i)
					{
						startIdx++;
						continue;
					}

					if (slashCount == -1)
					{
						slashCount = 0;
						bool prevIsSlash = false;
						for (auto cur = startIdx; cur < i; cur++)
						{
							if (strCommand[cur] == '/')
							{
								if (prevIsSlash)
									doubleSlash = true;

								slashCount++;
								prevIsSlash = true;
							}
							else
							{
								prevIsSlash = false;
							}
						}
					}

					if (!doubleSlash)
					{
						if (slashCount == 0)
						{
							sscanf_s(strCommand + startIdx, "%d", &posIdx);
							if (posIdx < 0)
								posIdx = positionBuf.Size() + posIdx + 1;
							Vertex.position = positionBuf[posIdx - 1];
						}
						else if (slashCount == 1)
						{
							sscanf_s(strCommand + startIdx, "%d/%d", &posIdx, &texIdx);
							if (posIdx < 0)
								posIdx = positionBuf.Size() + posIdx + 1;
							if (texIdx < 0)
								texIdx = texCoordBuf.Size() / 2 + texIdx + 1;
							Vertex.position = positionBuf[posIdx - 1];
							Vertex.fU = texCoordBuf[2 * texIdx - 2];
							Vertex.fV = texCoordBuf[2 * texIdx - 1];
						}
						else if (slashCount == 2)
						{
							sscanf_s(strCommand + startIdx, "%d/%d/%d", &posIdx, &texIdx, &normalIdx);
							if (posIdx < 0)
								posIdx = positionBuf.Size() + posIdx + 1;
							if (texIdx < 0)
								texIdx = texCoordBuf.Size() / 2 + texIdx + 1;
							if (normalIdx < 0)
								normalIdx = normalBuf.Size() + normalIdx + 1;
							Vertex.position = positionBuf[posIdx - 1];
							Vertex.fU = texCoordBuf[2 * texIdx - 2];
							Vertex.fV = texCoordBuf[2 * texIdx - 1];
							Vertex.normal = normalBuf[normalIdx - 1];
						}
					}
					else
					{
						sscanf_s(strCommand + startIdx, "%d//%d", &posIdx, &normalIdx);
						if (posIdx < 0)
							posIdx = positionBuf.Size() + posIdx + 1;
						if (normalIdx < 0)
							normalIdx = normalBuf.Size() + normalIdx + 1;
						Vertex.position = positionBuf[posIdx - 1];
						Vertex.normal = normalBuf[normalIdx - 1];
					}


					if (vertexCount >= 4)
						break;

					faceIdx[vertexCount] = AddVertex(posIdx - 1, &Vertex);
					vertexCount++;
					startIdx = i + 1;
				}

				if (makeLeftHanded)
				{
					Face.aiIndices[0] = faceIdx[0];
					Face.aiIndices[1] = faceIdx[2];
					Face.aiIndices[2] = faceIdx[1];
				}
				else
				{
					Face.aiIndices[0] = faceIdx[0];
					Face.aiIndices[1] = faceIdx[1];
					Face.aiIndices[2] = faceIdx[2];
				}

				mIndices.Add(Face.aiIndices[0]);
				mIndices.Add(Face.aiIndices[1]);
				mIndices.Add(Face.aiIndices[2]);

				// Add face
				Face.iSmoothingGroup = iSmoothingGroup;
				mFaces.Add(Face);
				mMaterialIdx.Add(iCurrentMtl);

				if (vertexCount == 4)
				{
					// Triangularize quad
					{
						if (makeLeftHanded)
						{
							quadFace.aiIndices[0] = faceIdx[3];
							quadFace.aiIndices[1] = Face.aiIndices[1];
							quadFace.aiIndices[2] = Face.aiIndices[0];
						}
						else
						{
							quadFace.aiIndices[0] = faceIdx[3];
							quadFace.aiIndices[1] = Face.aiIndices[0];
							quadFace.aiIndices[2] = Face.aiIndices[2];
						}

						mIndices.Add(quadFace.aiIndices[0]);
						mIndices.Add(quadFace.aiIndices[1]);
						mIndices.Add(quadFace.aiIndices[2]);
					}

					quadFace.iSmoothingGroup = iSmoothingGroup;
					mFaces.Add(quadFace);
					mMaterialIdx.Add(iCurrentMtl);
				}
			}
			else if (0 == CStringUtil::Strcmp(strCommand, "s")) // Handle smoothing group for normal computation
			{
				fscanf_s(pInFile, "%s", strCommand, MAX_PATH);

				if (strCommand[0] >= '1' && strCommand[0] <= '9')
				{
					hasSmoothGroup = true;
					sscanf_s(strCommand, "%d", &iSmoothingGroup);
				}
				else
					iSmoothingGroup = 0;
			}
			else if (0 == CStringUtil::Strcmp(strCommand, "mtllib"))
			{
				// Material library
				fscanf_s(pInFile, "%s", strMaterialFilename, MAX_PATH);
			}
			else if (0 == CStringUtil::Strcmp(strCommand, "usemtl"))
			{
				// Material
				char strName[MAX_PATH] = { 0 };
				fscanf_s(pInFile, "%s", strName, MAX_PATH);

				ObjMaterial currMtl = ObjMaterial(strName);
				auto itMtl = mMaterials.Find(currMtl);
				if (itMtl != INDEX_NONE)
				{
					iCurrentMtl = itMtl;
				}
				else
				{
					iCurrentMtl = mMaterials.Size();
					mMaterials.Add(currMtl);
				}

				mSubsetStartIdx.Add(mIndices.Size());
				mSubsetMtlIdx.Add(iCurrentMtl);
				mNumSubsets++;
			}
			else
			{
				while (!feof(pInFile) && fgetc(pInFile) != '\n');
			}
		}

		fclose(pInFile);

		// Correct subsets index
		if (mNumSubsets == 0)
		{
			mSubsetStartIdx.Add(0);
			mNumSubsets = 1;
			mSubsetMtlIdx.Add(0);
		}

		mSubsetStartIdx.Add(mIndices.Size());

		mVertexCount = mVertices.Size();
		mTriangleCount = mIndices.Size() / 3;

		// Init bounds
		mBounds = Matrix::TransformBBox(BoundingBox(minPt, maxPt), mWorld);

		// Recompute per-vertex normals
		if (forceComputeNormal || hasSmoothGroup || !mNormaled)
			ComputeVertexNormals();

		// Delete cache
		for (uint i = 0; i < mCache.Size(); i++)
		{
			CacheEntry* pEntry = mCache[i];
			while (pEntry != NULL)
			{
				CacheEntry* pNext = pEntry->pNext;
				Memory::SafeDelete(pEntry);
				pEntry = pNext;
			}
		}
		mCache.Clear();

		if (strMaterialFilename[0])
		{
			const char* path1 = CStringUtil::Strrchr(strPath, '/');
			const char* path2 = CStringUtil::Strrchr(strPath, '\\');
			int idx = (path1 ? path1 : path2) - strPath + 1;
			char strMtlPath[MAX_PATH] = { 0 };
			CStringUtil::Strncpy(strMtlPath, MAX_PATH, strPath, idx);
			CStringUtil::Strcat(strMtlPath, MAX_PATH, strMaterialFilename);

			LoadMaterialsFromMtl(strMtlPath);
		}

		if (mMaterials.Size() == 0)
			mMaterials.Add(ObjMaterial(""));

		return true;
	}

	// Detect whether there is a duplicate vertex, returns the existing
	// index if yes, otherwise create add new vertex to the vertex buffer
	uint ObjMesh::AddVertex(uint iHash, const MeshVertex* pVertex)
	{
		bool bFound = false;
		uint iIndex = 0;

		if (mCache.Size() > iHash)
		{
			CacheEntry* pEntry = mCache[iHash];
			while (pEntry != NULL)
			{
				MeshVertex* pCacheVertex = mVertices.Data() + pEntry->iIndex;
				if (memcmp(pCacheVertex, pVertex, sizeof(MeshVertex)) == 0)
				{
					bFound = true;
					iIndex = pEntry->iIndex;
					break;
				}
				pEntry = pEntry->pNext;
			}
		}

		if (!bFound)
		{
			iIndex = mVertices.Size();
			mVertices.Add(*pVertex);

			CacheEntry* pEntryNew = new CacheEntry();
			if (pEntryNew == NULL)
				return uint(-1);

			pEntryNew->iIndex = iIndex;
			pEntryNew->pNext = NULL;

			while (mCache.Size() <= iHash)
				mCache.Add(NULL);

			CacheEntry* pEntryCached = mCache[iHash];

			if (pEntryCached == NULL)
				mCache[iHash] = pEntryNew;
			else
			{
				while (pEntryCached->pNext != NULL)
					pEntryCached = pEntryCached->pNext;

				pEntryCached->pNext = pEntryNew;
			}
		}

		return iIndex;
	};

	void ObjMesh::LoadMaterialsFromMtl(const char* strPath)
	{
		char strCommand[MAX_PATH] = { 0 };
		FILE* pInFile = 0;
		fopen_s(&pInFile, strPath, "rt");
		assert(pInFile);

		int itCurrMaterial = INDEX_NONE;
		while (!feof(pInFile))
		{
			fscanf_s(pInFile, "%s", strCommand, MAX_PATH);

			if (0 == CStringUtil::Strcmp(strCommand, "#"))
			{
				// Comment
			}
			else if (0 == CStringUtil::Strcmp(strCommand, "newmtl"))
			{
				// Switching active materials
				char strName[MAX_PATH] = { 0 };
				fscanf_s(pInFile, "%s", strName, MAX_PATH);

				ObjMaterial tmpMtl = ObjMaterial(strName);

				itCurrMaterial = mMaterials.Find(tmpMtl);
			}

			if (itCurrMaterial == INDEX_NONE)
				continue;

			else if (0 == CStringUtil::Strcmp(strCommand, "Kd"))
			{
				// Diffuse color
				float r, g, b;
				fscanf_s(pInFile, "%f %f %f", &r, &g, &b);
				mMaterials[itCurrMaterial].color = Color(r, g, b);
			}
			else if (0 == CStringUtil::Strcmp(strCommand, "Ks"))
			{
				// Diffuse color
				float r, g, b;
				fscanf_s(pInFile, "%f %f %f", &r, &g, &b);
				mMaterials[itCurrMaterial].specColor = Color(r, g, b);
			}
			else if (0 == CStringUtil::Strcmp(strCommand, "Tf"))
			{
				// Diffuse color
				float r, g, b;
				fscanf_s(pInFile, "%f %f %f", &r, &g, &b);
				mMaterials[itCurrMaterial].transColor = Color(r, g, b);
			}
			else if (0 == CStringUtil::Strcmp(strCommand, "d") || 0 == CStringUtil::Strcmp(strCommand, "Tr"))
			{
				// Alpha
				fscanf_s(pInFile, "%f", &mMaterials[itCurrMaterial].color.a);
			}
			else if (0 == CStringUtil::Strcmp(strCommand, "map_Kd"))
			{
				if (!mMaterials[itCurrMaterial].strTexturePath[0])
				{
					// Texture
					char strTexName[MAX_PATH] = { 0 };
					fgets(strTexName, MAX_PATH, pInFile);

					if (strTexName[strlen(strTexName) - 1] == '\n')
						strTexName[strlen(strTexName) - 1] = '\0';

					const char* path1 = CStringUtil::Strrchr(strPath, '/');
					const char* path2 = CStringUtil::Strrchr(strPath, '\\');
					int idx = (path1 ? path1 : path2) - strPath + 1;

					const char* path3 = CStringUtil::Strrchr(mMaterials[itCurrMaterial].strName, '/');
					const char* path4 = CStringUtil::Strrchr(mMaterials[itCurrMaterial].strName, '\\');
					int idx2 = (path3 ? path3 : path4) - mMaterials[itCurrMaterial].strName + 1;

					char strMtlPath[MAX_PATH] = { 0 };
					CStringUtil::Strncpy(mMaterials[itCurrMaterial].strTexturePath, MAX_PATH, strPath, idx);
					CStringAnsi::Strncat(mMaterials[itCurrMaterial].strTexturePath, mMaterials[itCurrMaterial].strName + 1, CStringUtil::Strlen(mMaterials[itCurrMaterial].strTexturePath) + idx2);
					CStringUtil::Strcat(mMaterials[itCurrMaterial].strTexturePath, MAX_PATH, strTexName + 1);
				}
			}
			else if (0 == CStringUtil::Strcmp(strCommand, "bump"))
			{
				if (!mMaterials[itCurrMaterial].strBumpPath[0])
				{
					// Texture
					char strTexName[MAX_PATH] = { 0 };
					fgets(strTexName, MAX_PATH, pInFile);

					if (strTexName[strlen(strTexName) - 1] == '\n')
						strTexName[strlen(strTexName) - 1] = '\0';

					const char* path1 = CStringUtil::Strrchr(strPath, '/');
					const char* path2 = CStringUtil::Strrchr(strPath, '\\');
					int idx = (path1 ? path1 : path2) - strPath + 1;

					char strMtlPath[MAX_PATH] = { 0 };
					CStringUtil::Strncpy(mMaterials[itCurrMaterial].strBumpPath, MAX_PATH, strPath, idx);
					CStringUtil::Strcat(mMaterials[itCurrMaterial].strBumpPath, MAX_PATH, strTexName + 1);
				}
			}
			else
			{
				while (!feof(pInFile) && fgetc(pInFile) != '\n');
			}
		}

		fclose(pInFile);
		return;
	}

	void ObjMesh::ComputeVertexNormals()
	{
		// First compute per face normals
		Array<Vector3> vFaceNormals;
		vFaceNormals.Resize(mFaces.Size());
		for (auto i = 0; i < mFaces.Size(); i++)
		{
			const Vector3& pt1 = GetVertexAt(mFaces[i].aiIndices[0]).position;
			const Vector3& pt2 = GetVertexAt(mFaces[i].aiIndices[1]).position;
			const Vector3& pt3 = GetVertexAt(mFaces[i].aiIndices[2]).position;

			Vector3 vEdge1 = pt2 - pt1;
			Vector3 vEdge2 = pt3 - pt1;
			Vector3 crossProd = Math::Cross(vEdge1, vEdge2);
			if (Math::Length(crossProd) > 0.0f)
				vFaceNormals[i] = Math::Normalize(crossProd);
			else
				vFaceNormals[i] = Vector3::ZERO;
		}

		struct VertexFace
		{
			int iCount;
			Array<int> List;
			VertexFace()
				: iCount(0) {}
		};
		Array<VertexFace> VertexFaceList;
		VertexFaceList.Resize(mVertices.Size());
		for (auto i = 0; i < mFaces.Size(); i++)
		{
			for (auto j = 0; j < 3; j++)
			{
				VertexFaceList[mFaces[i].aiIndices[j]].iCount++;
				VertexFaceList[mFaces[i].aiIndices[j]].List.Add(i);
			}
		}

		// Compute per vertex normals with smoothing group
		for (auto i = 0; i < mFaces.Size(); i++)
		{
			const MeshFace& face = mFaces[i];
			for (auto j = 0; j < 3; j++)
			{
				int iFaceCount = 0;
				Vector3 vNormal;
				for (auto k = 0; k < VertexFaceList[mFaces[i].aiIndices[j]].iCount; k++)
				{
					int iFaceIdx = VertexFaceList[mFaces[i].aiIndices[j]].List[k];
					if (face.iSmoothingGroup & mFaces[iFaceIdx].iSmoothingGroup)
					{
						vNormal += vFaceNormals[iFaceIdx];
						iFaceCount++;
					}
				}

				if (iFaceCount > 0)
					vNormal /= float(iFaceCount);
				else
					vNormal = vFaceNormals[i];

				if (Math::Length(vNormal) > 0.0f)
					vNormal = Math::Normalize(vNormal);
				else
					vNormal = Vector3::ZERO;

				MeshVertex& vert = mVertices[face.aiIndices[j]];
				if (vert.normal == Vector3::ZERO)
					vert.normal = vNormal;
				else if (vert.normal != vNormal)
				{
					MeshVertex newVertex = vert;
					newVertex.normal = vNormal;

					auto idx = AddVertex(mFaces[i].aiIndices[j], &newVertex);
					//mFaces[i].aiIndices[j] = idx;
					mIndices[3 * i + j] = idx;
				}
			}
		}

		mVertexCount = mVertices.Size();
		mNormaled = true;
	}

	void ObjMesh::LoadPlane(const Vector3& pos, const Vector3& scl, const Vector3& rot, const float length)
	{
		Matrix mWorld, mWorldInv;
		Matrix::CalcTransform(pos, scl, rot, &mWorld, &mWorldInv);

		float length_2 = length * 0.5f;

		Vector3 pt = Matrix::TransformPoint(Vector3(-length_2, 0.0f, length_2), mWorld);
		mBounds = Math::Union(mBounds, pt);
		mVertices.Add(MeshVertex(pt,
			Math::Normalize(Matrix::TransformNormal(Vector3(Vector3::UNIT_Y), mWorldInv)),
			0.0f, 0.0f));

		pt = Matrix::TransformPoint(Vector3(-length_2, 0.0f, -length_2), mWorld);
		mBounds = Math::Union(mBounds, pt);
		mVertices.Add(MeshVertex(pt,
			Math::Normalize(Matrix::TransformNormal(Vector3(Vector3::UNIT_Y), mWorldInv)),
			0.0f, 1.0f));

		pt = Matrix::TransformPoint(Vector3(length_2, 0.0f, -length_2), mWorld);
		mBounds = Math::Union(mBounds, pt);
		mVertices.Add(MeshVertex(pt,
			Math::Normalize(Matrix::TransformNormal(Vector3(Vector3::UNIT_Y), mWorldInv)),
			1.0f, 1.0f));

		pt = Matrix::TransformPoint(Vector3(length_2, 0.0f, length_2), mWorld);
		mBounds = Math::Union(mBounds, pt);
		mVertices.Add(MeshVertex(pt,
			Math::Normalize(Matrix::TransformNormal(Vector3(Vector3::UNIT_Y), mWorldInv)),
			1.0f, 0.0f));

		mIndices.Add(0);
		mIndices.Add(2);
		mIndices.Add(1);
		mIndices.Add(2);
		mIndices.Add(0);
		mIndices.Add(3);

		mTriangleCount = mIndices.Size() / 3;
		mVertexCount = mVertices.Size();
		mMaterialIdx.Init(0, mTriangleCount);
		mNormaled = mTextured = true;

		mMaterials.Add(ObjMaterial());

		mNumSubsets = 1;
		mSubsetMtlIdx.Add(0);
		mSubsetStartIdx.Add(0);
		mSubsetStartIdx.Add(mIndices.Size());
	}

	void ObjMesh::LoadSphere(const Vector3& pos, const Vector3& scl, const Vector3& rot, const float fRadius, const int slices, const int stacks)
	{
		Matrix mWorld, mWorldInv;
		Matrix::CalcTransform(pos, scl, rot, &mWorld, &mWorldInv);

		const float fThetaItvl = float(Math::EDX_PI) / float(stacks);
		const float fPhiItvl = float(Math::EDX_TWO_PI) / float(slices);

		float fTheta = 0.0f;
		for (int i = 0; i <= stacks; i++)
		{
			float fPhi = 0.0f;
			for (int j = 0; j <= slices; j++)
			{
				Vector3 vDir = Math::SphericalDirection(Math::Sin(fTheta), Math::Cos(fTheta), fPhi);

				Vector3 pt = Matrix::TransformPoint(fRadius * vDir, mWorld);
				mBounds = Math::Union(mBounds, pt);
				mVertices.Add(MeshVertex(
					pt,
					Matrix::TransformNormal(vDir, mWorldInv),
					fPhi / float(Math::EDX_TWO_PI), fTheta / float(Math::EDX_PI)));

				fPhi += fPhiItvl;
			}

			fTheta += fThetaItvl;
		}

		for (int i = 0; i < stacks; i++)
		{
			for (int j = 0; j < slices; j++)
			{
				mIndices.Add(i * (slices + 1) + j);
				mIndices.Add(i * (slices + 1) + j + 1);
				mIndices.Add((i + 1) * (slices + 1) + j);

				mIndices.Add(i * (slices + 1) + j + 1);
				mIndices.Add((i + 1) * (slices + 1) + j + 1);
				mIndices.Add((i + 1) * (slices + 1) + j);
			}
		}

		mTriangleCount = mIndices.Size() / 3;
		mVertexCount = mVertices.Size();
		mMaterialIdx.Init(0, mTriangleCount);
		mNormaled = mTextured = true;

		mMaterials.Add(ObjMaterial());

		mNumSubsets = 1;
		mSubsetMtlIdx.Add(0);
		mSubsetStartIdx.Add(0);
		mSubsetStartIdx.Add(mIndices.Size());
	}

	void ObjMesh::Release()
	{
		mVertices.Clear();
		mIndices.Clear();
		mFaces.Clear();
		mCache.Clear();
	}
}