// Fill out your copyright notice in the Description page of Project Settings.


#include "MeshLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Modules/ModuleManager.h"
#include "Misc/FileHelper.h"
#include "IImageWrapper.h"
#include "Runtime/ImageWrapper/Public/IImageWrapperModule.h"
#include "HAL/FileManager.h"
#include "HAL/FileManagerGeneric.h"

FMeshData ProcessMesh(aiMesh* Mesh, const aiScene* Scene)
{
    FMeshData MeshData;

	for (uint32 j = 0; j < Mesh->mNumVertices; ++j)
	{
		FVector Vertex = FVector(Mesh->mVertices[j].x, Mesh->mVertices[j].y, Mesh->mVertices[j].z);
		MeshData.Vertices.Add(Vertex);
		FVector Normal = FVector::ZeroVector;

		if (Mesh->HasNormals())
		{
		    Normal = FVector(Mesh->mNormals[j].x, Mesh->mNormals[j].y, Mesh->mNormals[j].z);
		}
		MeshData.Normals.Add(Normal);
		if (Mesh->mTextureCoords[0])
		{
		    MeshData.UVs.Add(FVector2D(static_cast<float>(Mesh->mTextureCoords[0][j].x), 1.f-static_cast<float>(Mesh->mTextureCoords[0][j].y)));
		}

		if (Mesh->HasTangentsAndBitangents())
		{
			FProcMeshTangent Tangent = FProcMeshTangent(Mesh->mTangents[j].x, Mesh->mTangents[j].y, Mesh->mTangents[j].z);
			MeshData.Tangents.Add(Tangent);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("mNumFaces: %d"), Mesh->mNumFaces);
	for (uint32 i = 0; i < Mesh->mNumFaces; i++)
	{
		aiFace Face = Mesh->mFaces[i];
		for (uint32 f = 0; f < Face.mNumIndices; f++)
		{
		    MeshData.Triangles.Add(Face.mIndices[f]);
		}
	}

	return MeshData;
}

void ProcessNode(aiNode* Node, const aiScene* Scene, int ParentNodeIndex, int* CurrentIndex, FFinalReturnData* FinalReturnData)
{
    FNodeData NodeData;
	NodeData.NodeParentIndex = ParentNodeIndex;
	

	aiMatrix4x4 TempTrans = Node->mTransformation;
	FMatrix tempMatrix;
	tempMatrix.M[0][0] = TempTrans.a1; tempMatrix.M[0][1] = TempTrans.b1; tempMatrix.M[0][2] = TempTrans.c1; tempMatrix.M[0][3] = TempTrans.d1;
	tempMatrix.M[1][0] = TempTrans.a2; tempMatrix.M[1][1] = TempTrans.b2; tempMatrix.M[1][2] = TempTrans.c2; tempMatrix.M[1][3] = TempTrans.d2;
	tempMatrix.M[2][0] = TempTrans.a3; tempMatrix.M[2][1] = TempTrans.b3; tempMatrix.M[2][2] = TempTrans.c3; tempMatrix.M[2][3] = TempTrans.d3;
	tempMatrix.M[3][0] = TempTrans.a4; tempMatrix.M[3][1] = TempTrans.b4; tempMatrix.M[3][2] = TempTrans.c4; tempMatrix.M[3][3] = TempTrans.d4;
	NodeData.RelativeTransformTransform = FTransform(tempMatrix);

    for (uint32 n = 0; n < Node->mNumMeshes; n++)
    {
		uint32 MeshIndex = Node->mMeshes[n];
		UE_LOG(LogTemp, Log, TEXT("Loading Mesh at index: %d"), MeshIndex);
        aiMesh* Mesh = Scene->mMeshes[MeshIndex];
		NodeData.Meshes.Add(ProcessMesh(Mesh, Scene));
    }

	FinalReturnData->Nodes.Add(NodeData);

	UE_LOG(LogTemp, Log, TEXT("mNumMeshes: %d, mNumChildren of Node: %d"), Node->mNumMeshes, Node->mNumChildren);
	int CurrentParentIndex = *CurrentIndex;
	for (uint32 n = 0; n < Node->mNumChildren; n++)
	{
		(*CurrentIndex)++;
	    ProcessNode(Node->mChildren[n], Scene, CurrentParentIndex, CurrentIndex, FinalReturnData);
	}
}

FFinalReturnData UMeshLoader::LoadMeshFromFile(FString FilePath, EPathType type)
{
    FFinalReturnData ReturnData;
	ReturnData.Success = false;

	if (FilePath.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Runtime Mesh Loader: filepath is empty.\n"));
		return ReturnData;
	}

	std::string FinalFilePath;
	switch (type)
	{
	    case EPathType::Absolute:
		    FinalFilePath = TCHAR_TO_UTF8(*FilePath);
		    break;
	    case EPathType::Relative:
		    FinalFilePath = TCHAR_TO_UTF8(*FPaths::Combine(FPaths::ProjectContentDir(), FilePath));
		    break;
	}

	Assimp::Importer mImporter;

	const aiScene* Scene = mImporter.ReadFile(FinalFilePath.c_str(), aiProcess_Triangulate | aiProcess_MakeLeftHanded | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals | aiProcess_OptimizeMeshes);

	if (Scene == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Runtime Mesh Loader: Read mesh file failure.\n"));
		return ReturnData;
	}

	if (Scene->HasMeshes())
	{
		int NodeIndex = 0;
		int* NodeIndexPtr = &NodeIndex;
		ProcessNode(Scene->mRootNode, Scene, -1, NodeIndexPtr, &ReturnData);

		ReturnData.Success = true;
	}

	return ReturnData;
}

bool UMeshLoader::DirectoryExists(FString DirectoryPath)
{
	return FPaths::DirectoryExists(DirectoryPath);
}

bool UMeshLoader::CreateDirectory(FString DirectoryPath)
{
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (!PlatformFile.DirectoryExists(*DirectoryPath))
	{
	    return PlatformFile.CreateDirectoryTree(*DirectoryPath);
	}

	return true;
}

TArray<FString> UMeshLoader::ListFolders(FString DirectoryPath)
{
	TArray<FString> Folders;
	FFileManagerGeneric::Get().FindFilesRecursive(Folders, *DirectoryPath, TEXT("*"), false, true, true);
	return Folders;
}

UTexture2D* UMeshLoader::LoadTexture2DFromFile(const FString& FullFilePath, bool& IsValid, int32& Width, int32& Height)
{
    IsValid = false;
	UTexture2D* LoadedT2D = NULL;
	
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	//Load From File
	TArray<uint8> RawFileData;
	/*If you use lower unreal engine,for example the version is 4.20,you may get a error message in bulid,you should use The following code replace "TArray<uint8> RawFileData;"
	const TArray<uint8>* UncompressedBGRA = NULL;*/
	
	if (!FFileHelper::LoadFileToArray(RawFileData, * FullFilePath)) 
	{
		return NULL;
	}
	
	  
	//Create T2D!
	if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
	{ 
		TArray<uint8> UncompressedBGRA;
		if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBGRA))
		{
			LoadedT2D = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_B8G8R8A8);
			
			//Valid?
			if (!LoadedT2D) 
			{
				return NULL;
			}
			
			//Out!
			Width = ImageWrapper->GetWidth();
			Height = ImageWrapper->GetHeight();
			 
			//Copy!
			void* TextureData = LoadedT2D->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(TextureData, UncompressedBGRA.GetData(), UncompressedBGRA.Num());
			/*if you use the code"const TArray<uint8>* UncompressedBGRA = NULL;"，Accordingly, since UncompressedBGRA becomes a pointer, you need to use a pointer reference method, like this
			FMemory::Memcpy(TextureData, UncompressedBGRA->GetData(), UncompressedBGRA->Num());*/
			LoadedT2D->PlatformData->Mips[0].BulkData.Unlock();

			//Update!
			LoadedT2D->UpdateResource();
		}
	}
	 
	// Success!
	IsValid = true;
	return LoadedT2D;
}
