#include "ObjMesh.h"
#include "../Math/Matrix.h"
#include "../Memory/Memory.h"

namespace EDX
{
	bool ObjMesh::LoadFromObj(const Vector3& pos,
		const Vector3& scl,
		const Vector3& rot,
		const char* strPath,
		const bool makeLeftHanded)
	{
		vector<Vector3> positionBuf;
		vector<Vector3> normalBuf;
		vector<float> texCoordBuf;
		int iSmoothingGroup = 0;
		int iCurrentMtl = 0;
		char strMaterialFilename[MAX_PATH] = { 0 };

		Matrix mWorld, mWorldInv;

		Vector3 leftHandedScl = makeLeftHanded ? Vector3(-1.0f, 1.0f, 1.0f) : Vector3::UNIT_SCALE;
		Matrix::CalcTransform(pos, scl * leftHandedScl, rot, &mWorld, &mWorldInv);

		char strCommand[MAX_PATH] = { 0 };
		FILE* pInFile = 0;
		fopen_s(&pInFile, strPath, "rt");
		assert(pInFile);

		while (!feof(pInFile))
		{
			fscanf_s(pInFile, "%s", strCommand, MAX_PATH);

			if (0 == strcmp(strCommand, "#"))
			{
				// Comment
			}
			else if (0 == strcmp(strCommand, "v"))
			{
				// Vertex Position
				float x, y, z;
				fscanf_s(pInFile, "%f %f %f", &x, &y, &z);
				positionBuf.push_back(Matrix::TransformPoint(Vector3(x, y, z), mWorld));
			}
			else if (0 == strcmp(strCommand, "vt"))
			{
				// Vertex TexCoord
				float u, v;
				fscanf_s(pInFile, "%f %f", &u, &v);
				texCoordBuf.push_back(u);
				texCoordBuf.push_back(v);
				mTextured = true;
			}
			else if (0 == strcmp(strCommand, "vn"))
			{
				// Vertex Normal
				float x, y, z;
				fscanf_s(pInFile, "%f %f %f", &x, &y, &z);
				normalBuf.push_back(Matrix::TransformNormal(Vector3(x, y, z), mWorldInv));
				mNormaled = true;
			}
			else if (0 == strcmp(strCommand, "f"))
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
								posIdx = positionBuf.size() + posIdx + 1;
							Vertex.position = positionBuf[posIdx - 1];
						}
						else if (slashCount == 1)
						{
							sscanf_s(strCommand + startIdx, "%d/%d", &posIdx, &texIdx);
							if (posIdx < 0)
								posIdx = positionBuf.size() + posIdx + 1;
							if (texIdx < 0)
								texIdx = texCoordBuf.size() / 2 + texIdx + 1;
							Vertex.position = positionBuf[posIdx - 1];
							Vertex.fU = texCoordBuf[2 * texIdx - 2];
							Vertex.fV = texCoordBuf[2 * texIdx - 1];
						}
						else if (slashCount == 2)
						{
							sscanf_s(strCommand + startIdx, "%d/%d/%d", &posIdx, &texIdx, &normalIdx);
							if (posIdx < 0)
								posIdx = positionBuf.size() + posIdx + 1;
							if (texIdx < 0)
								texIdx = texCoordBuf.size() / 2 + texIdx + 1;
							if (normalIdx < 0)
								normalIdx = normalBuf.size() + normalIdx + 1;
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
							posIdx = positionBuf.size() + posIdx + 1;
						if (normalIdx < 0)
							normalIdx = normalBuf.size() + normalIdx + 1;
						Vertex.position = positionBuf[posIdx - 1];
						Vertex.normal = normalBuf[normalIdx - 1];
					}

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

				mIndices.push_back(Face.aiIndices[0]);
				mIndices.push_back(Face.aiIndices[1]);
				mIndices.push_back(Face.aiIndices[2]);

				// Add face
				Face.iSmoothingGroup = iSmoothingGroup;
				mFaces.push_back(Face);
				mMaterialIdx.push_back(iCurrentMtl);

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

						mIndices.push_back(quadFace.aiIndices[0]);
						mIndices.push_back(quadFace.aiIndices[1]);
						mIndices.push_back(quadFace.aiIndices[2]);
					}

					quadFace.iSmoothingGroup = iSmoothingGroup;
					mFaces.push_back(quadFace);
					mMaterialIdx.push_back(iCurrentMtl);
				}
			}
			else if (0 == strcmp(strCommand, "s")) // Handle smoothing group for normal computation
			{
				fscanf_s(pInFile, "%s", strCommand, MAX_PATH);

				if (strCommand[0] >= '0' && strCommand[0] <= '9')
					sscanf_s(strCommand, "%d", &iSmoothingGroup);
				else
					iSmoothingGroup = 0;
			}
			else if (0 == strcmp(strCommand, "mtllib"))
			{
				// Material library
				fscanf_s(pInFile, "%s", strMaterialFilename, MAX_PATH);
			}
			else if (0 == strcmp(strCommand, "usemtl"))
			{
				// Material
				char strName[MAX_PATH] = { 0 };
				fscanf_s(pInFile, "%s", strName, MAX_PATH);

				ObjMaterial currMtl = ObjMaterial(strName);
				auto itMtl = find(mMaterials.begin(), mMaterials.end(), currMtl);
				if (itMtl != mMaterials.end())
				{
					iCurrentMtl = itMtl - mMaterials.begin();
				}
				else
				{
					iCurrentMtl = mMaterials.size();
					mMaterials.push_back(currMtl);
				}

				mSubsetStartIdx.push_back(mIndices.size());
				mSubsetMtlIdx.push_back(iCurrentMtl);
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
			mSubsetStartIdx.push_back(0);
			mNumSubsets = 1;
			mSubsetMtlIdx.push_back(0);
		}

		mSubsetStartIdx.push_back(mIndices.size());

		mVertexCount = mVertices.size();
		mTriangleCount = mIndices.size() / 3;

		// Recompute per-vertex normals
		if (iSmoothingGroup != 0 || !mNormaled)
			ComputeVertexNormals();

		// Delete cache
		for (uint i = 0; i < mCache.size(); i++)
		{
			CacheEntry* pEntry = mCache[i];
			while (pEntry != NULL)
			{
				CacheEntry* pNext = pEntry->pNext;
				SafeDelete(pEntry);
				pEntry = pNext;
			}
		}
		mCache.clear();

		if (strMaterialFilename[0])
		{
			int idx = strrchr(strPath, '/') - strPath + 1;
			char strMtlPath[MAX_PATH] = { 0 };
			strncpy_s(strMtlPath, MAX_PATH, strPath, idx);
			strcat_s(strMtlPath, MAX_PATH, strMaterialFilename);

			LoadMaterialsFromMtl(strMtlPath);
		}

		if (mMaterials.empty())
			mMaterials.push_back(ObjMaterial(""));

		return true;
	}

	// Detect whether there is a duplicate vertex, returns the existing
	// index if yes, otherwise create add new vertex to the vertex buffer
	uint ObjMesh::AddVertex(uint iHash, const MeshVertex* pVertex)
	{
		bool bFound = false;
		uint iIndex = 0;

		if (mCache.size() > iHash)
		{
			CacheEntry* pEntry = mCache[iHash];
			while (pEntry != NULL)
			{
				MeshVertex* pCacheVertex = mVertices.data() + pEntry->iIndex;
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
			iIndex = mVertices.size();
			mVertices.push_back(*pVertex);

			CacheEntry* pEntryNew = new CacheEntry();
			if (pEntryNew == NULL)
				return uint(-1);

			pEntryNew->iIndex = iIndex;
			pEntryNew->pNext = NULL;

			while (mCache.size() <= iHash)
				mCache.push_back(NULL);

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

		auto itCurrMaterial = mMaterials.end();
		while (!feof(pInFile))
		{
			fscanf_s(pInFile, "%s", strCommand, MAX_PATH);

			if (0 == strcmp(strCommand, "#"))
			{
				// Comment
			}
			else if (0 == strcmp(strCommand, "newmtl"))
			{
				// Switching active materials
				char strName[MAX_PATH] = { 0 };
				fscanf_s(pInFile, "%s", strName, MAX_PATH);

				ObjMaterial tmpMtl = ObjMaterial(strName);

				itCurrMaterial = find(mMaterials.begin(), mMaterials.end(), tmpMtl);
			}

			if (itCurrMaterial == mMaterials.end())
				continue;

			else if (0 == strcmp(strCommand, "Kd"))
			{
				// Diffuse color
				float r, g, b;
				fscanf_s(pInFile, "%f %f %f", &r, &g, &b);
				itCurrMaterial->color = Color(r, g, b);
			}
			else if (0 == strcmp(strCommand, "d") || 0 == strcmp(strCommand, "Tr"))
			{
				// Alpha
				fscanf_s(pInFile, "%f", &itCurrMaterial->color.a);
			}
			else if (0 == strcmp(strCommand, "map_Kd"))
			{
				if (!itCurrMaterial->strTexturePath[0])
				{
					// Texture
					char strTexName[MAX_PATH] = { 0 };
					fgets(strTexName, MAX_PATH, pInFile);

					if (strTexName[strlen(strTexName) - 1] == '\n')
						strTexName[strlen(strTexName) - 1] = '\0';

					int idx = strrchr(strPath, '/') - strPath + 1;
					char strMtlPath[MAX_PATH] = { 0 };
					strncpy_s(itCurrMaterial->strTexturePath, MAX_PATH, strPath, idx);
					strcat_s(itCurrMaterial->strTexturePath, MAX_PATH, strTexName + 1);
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
		vector<Vector3> vFaceNormals;
		vFaceNormals.resize(mFaces.size());
		for (auto i = 0; i < mFaces.size(); i++)
		{
			const Vector3& pt1 = GetVertexAt(mFaces[i].aiIndices[0]).position;
			const Vector3& pt2 = GetVertexAt(mFaces[i].aiIndices[1]).position;
			const Vector3& pt3 = GetVertexAt(mFaces[i].aiIndices[2]).position;

			Vector3 vEdge1 = pt2 - pt1;
			Vector3 vEdge2 = pt3 - pt1;

			vFaceNormals[i] = Math::Normalize(Math::Cross(vEdge1, vEdge2));
		}

		struct VertexFace
		{
			int iCount;
			vector<int> List;
			VertexFace()
				: iCount(0) {}
		};
		vector<VertexFace> VertexFaceList;
		VertexFaceList.resize(mVertices.size());
		for (auto i = 0; i < mFaces.size(); i++)
		{
			for (auto j = 0; j < 3; j++)
			{
				VertexFaceList[mFaces[i].aiIndices[j]].iCount++;
				VertexFaceList[mFaces[i].aiIndices[j]].List.push_back(i);
			}
		}

		// Compute per vertex normals with smoothing group
		for (auto i = 0; i < mFaces.size(); i++)
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
				vNormal = Math::Normalize(vNormal);

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

		mVertexCount = mVertices.size();
		mNormaled = true;
	}

	void ObjMesh::LoadPlane(const Vector3& pos, const Vector3& scl, const Vector3& rot, const float length)
	{
		Matrix mWorld, mWorldInv;
		Matrix::CalcTransform(pos, scl, rot, &mWorld, &mWorldInv);

		float length_2 = length * 0.5f;

		mVertices.push_back(MeshVertex(Matrix::TransformPoint(Vector3(-length_2, 0.0f, length_2), mWorld),
			Math::Normalize(Matrix::TransformNormal(Vector3(Vector3::UNIT_Y), mWorldInv)),
			0.0f, 0.0f));
		mVertices.push_back(MeshVertex(Matrix::TransformPoint(Vector3(-length_2, 0.0f, -length_2), mWorld),
			Math::Normalize(Matrix::TransformNormal(Vector3(Vector3::UNIT_Y), mWorldInv)),
			0.0f, 1.0f));
		mVertices.push_back(MeshVertex(Matrix::TransformPoint(Vector3(length_2, 0.0f, -length_2), mWorld),
			Math::Normalize(Matrix::TransformNormal(Vector3(Vector3::UNIT_Y), mWorldInv)),
			1.0f, 1.0f));
		mVertices.push_back(MeshVertex(Matrix::TransformPoint(Vector3(length_2, 0.0f, length_2), mWorld),
			Math::Normalize(Matrix::TransformNormal(Vector3(Vector3::UNIT_Y), mWorldInv)),
			1.0f, 0.0f));

		mIndices.push_back(0);
		mIndices.push_back(2);
		mIndices.push_back(1);
		mIndices.push_back(2);
		mIndices.push_back(0);
		mIndices.push_back(3);

		mTriangleCount = mIndices.size() / 3;
		mVertexCount = mVertices.size();
		mMaterialIdx.assign(mTriangleCount, 0);
		mNormaled = mTextured = true;
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
				mVertices.push_back(MeshVertex(
					Matrix::TransformPoint(fRadius * vDir, mWorld),
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
				mIndices.push_back(i * (slices + 1) + j);
				mIndices.push_back(i * (slices + 1) + j + 1);
				mIndices.push_back((i + 1) * (slices + 1) + j);

				mIndices.push_back(i * (slices + 1) + j + 1);
				mIndices.push_back((i + 1) * (slices + 1) + j + 1);
				mIndices.push_back((i + 1) * (slices + 1) + j);
			}
		}

		mTriangleCount = mIndices.size() / 3;
		mVertexCount = mVertices.size();
		mMaterialIdx.assign(mTriangleCount, 0);
		mNormaled = mTextured = true;
	}

	void ObjMesh::Release()
	{
		mVertices.clear();
		mIndices.clear();
		mFaces.clear();
		mCache.clear();
	}
}