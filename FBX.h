#pragma once

#include <fbxsdk.h>
#include <iostream>
#include <cassert>

#include "Hierarchy.h"

class Fbx : public Hierarchy
{
public:
	Fbx() {
		Manager = FbxManager::Create();
		const auto IOSettings = FbxIOSettings::Create(Manager, IOSROOT);
		if (nullptr != IOSettings) {
			Manager->SetIOSettings(IOSettings);
		}
		//!< インポート設定を変更する例
		//Manager->GetIOSettings()->SetBoolProp(IMP_FBX_MATERIAL, true);
		//Manager->GetIOSettings()->SetBoolProp(IMP_FBX_TEXTURE, true);
		//Manager->GetIOSettings()->SetBoolProp(IMP_FBX_LINK, false);
		//Manager->GetIOSettings()->SetBoolProp(IMP_FBX_SHAPE, false);
		//Manager->GetIOSettings()->SetBoolProp(IMP_FBX_GOBO, false);
		//Manager->GetIOSettings()->SetBoolProp(IMP_FBX_ANIMATION, true);
		//Manager->GetIOSettings()->SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
		
		//!< テスト
		//Load(GetEnv("FBX_SDK_PATH") + "\\samples\\ConvertScene\\box.fbx"); //!< メッシュのみ 
		//Load(GetEnv("FBX_SDK_PATH") + "\\samples\\Normals\\Normals.fbx"); //!< (四角形ポリゴン)
		//Load(GetEnv("FBX_SDK_PATH") + "\\samples\\StereoCamera\\StereoCamera.fbx"); //!< カメラのみ
		//Load(GetEnv("FBX_SDK_PATH") + "\\samples\\SwitchBinding\\Bind_Before_Switch.fbx"); //!< メッシュ、ジョイント、(四角形ポリゴン)
		//Load(GetEnv("FBX_SDK_PATH") + "\\samples\\Transformations\\JointHierarchy.fbx"); //!< ジョイントのみ
		//Load(GetEnv("FBX_SDK_PATH") + "\\samples\\UVSample\\sadface.fbx"); //!< メッシュ、テクスチャ
		Load(GetEnv("FBX_SDK_PATH") + "\\samples\\ViewScene\\humanoid.fbx"); //!< ジョイント、メッシュ、アニメーション、カメラ、ライト
	}
	virtual ~Fbx() {
		if (nullptr != Scene) { Scene->Destroy(); }
		if (nullptr != Manager) { Manager->Destroy(); }
	}

	virtual void Process(FbxMesh* Mesh) {
		if (nullptr != Mesh) {
			FbxStringList UVSetName;
			Mesh->GetUVSetNames(UVSetName);			

			for (int i = 0; i < Mesh->GetPolygonCount(); ++i) {
				if (i > 50) { Tabs(); std::cout << "..." << std::endl; return; }

				Tabs(); std::cout << "[" << i << "]{" << std::endl;
				const auto PS = Mesh->GetPolygonSize(i);
				for (int j = 0; j < Mesh->GetPolygonSize(i); ++j) {
					Tabs(); std::cout << "\t";
					//!< 頂点
					const auto Vertex = Mesh->GetControlPointAt(Mesh->GetPolygonVertex(i, j));
					std::cout << "( " << Vertex[0] << ", " << Vertex[1] << ", " << Vertex[2] << " ), ";

					//!< 法線
					FbxVector4 Normal;
					if (Mesh->GetPolygonVertexNormal(i, j, Normal)) {
						std::cout << "( " << Normal[0] << ", " << Normal[1] << ", " << Normal[2] << " ), ";
					}

					//!< UV
					for (int k = 0; k < UVSetName.GetCount(); ++k) {
						FbxVector2 UV;
						bool Unmapped;
						if (Mesh->GetPolygonVertexUV(i, j, UVSetName.GetStringAt(k), UV, Unmapped)) {
							std::cout << "( " << UV[0] << ", " << UV[1] << " ), ";
						}
					}
					std::cout << std::endl;
				}
				Tabs(); std::cout << "}" << std::endl;
			}
		}
	}
	virtual void Process(FbxCamera* Camera) {
		if (nullptr != Camera) {
			FbxTime Time;
			const auto Pos = Camera->EvaluatePosition(Time);
			const auto LookAt = Camera->EvaluateLookAtPosition(Time);
			const auto Up = Camera->EvaluateUpDirection(Pos, LookAt, Time);
			Tabs(); std::cout << "Position = ( " << Pos[0] << ", " << Pos[1] << ", " << Pos[2] << " )" << std::endl;
			Tabs(); std::cout << "LookAt = ( " << LookAt[0] << ", " << LookAt[1] << ", " << LookAt[2] << " )" << std::endl;
			Tabs(); std::cout << "Up = ( " << Up[0] << ", " << Up[1] << ", " << Up[2] << " )" << std::endl;
		}
	}
	virtual void Process(FbxLight* Light) {
		if (nullptr != Light) {
			switch (Light->LightType.Get()) {
				using enum FbxLight::EType;
			default: break;
			case ePoint:
				Tabs(); std::cout << "ePoint" << std::endl;
				break;
			case eDirectional:
				Tabs(); std::cout << "eDirectional" << std::endl;
				break;
			case eSpot:
				Tabs(); std::cout << "eSpot" << std::endl;
				break;
			case eArea:
				Tabs(); std::cout << "eArea" << std::endl;
				break;
			case eVolume:
				Tabs(); std::cout << "eVolume" << std::endl;
				break;
			}
		}
	}
	virtual void Process(FbxNodeAttribute* Attr) {
		if (nullptr != Attr) {
			Tabs(); std::cout << "[ Attribute ] " << Attr->GetTypeName() << std::endl;
			switch (Attr->GetAttributeType())
			{
				using enum FbxNodeAttribute::EType;
			default:
				break;
			case eUnknown:
				break;
			case eNull:
				break;
			case eMarker:
				break;
			case eSkeleton:
				break;
			case eMesh:
				PushTab();
				Process(static_cast<FbxMesh*>(Attr));
				PopTab();
				break;
			case eNurbs:
				break;
			case ePatch:
				break;
			case eCamera:
				PushTab();
				Process(static_cast<FbxCamera*>(Attr));
				PopTab();
				break;
			case eCameraStereo:
				break;
			case eCameraSwitcher:
				break;
			case eLight:
				PushTab();
				Process(static_cast<FbxLight*>(Attr));
				PopTab();
				break;
			case eOpticalReference:
				break;
			case eOpticalMarker:
				break;
			case eNurbsCurve:
				break;
			case eTrimNurbsSurface:
				break;
			case eBoundary:
				break;
			case eNurbsSurface:
				break;
			case eShape:
				break;
			case eLODGroup:
				break;
			case eSubDiv:
				break;
			case eCachedEffect:
				break;
			case eLine:
				break;
			}
		}
	}
	virtual void Process(FbxNode* Node) {
		if (nullptr != Node) {
			Tabs(); std::cout << "[ Node ] " << Node->GetName() << std::endl;
			[[maybe_unused]] const auto T = Node->LclTranslation.Get();
			[[maybe_unused]] const auto R = Node->LclRotation.Get();
			[[maybe_unused]] const auto S = Node->LclScaling.Get();

			[[maybe_unused]] const auto GTrans = Node->EvaluateGlobalTransform();
			[[maybe_unused]] const auto LTrans = Node->EvaluateLocalTransform();

			FbxTime Time;
			Time.SetSecondDouble(2.0);
			[[maybe_unused]] const auto GTransAnim = Node->EvaluateGlobalTransform(Time);

			PushTab();
			for (int i = 0; i < Node->GetNodeAttributeCount(); ++i) {
				Process(Node->GetNodeAttributeByIndex(i));
			}
			PopTab();

			PushTab();
			for (int i = 0; i < Node->GetChildCount(); i++) {
				Process(Node->GetChild(i));
			}
			PopTab();
		}
	}

	virtual void Load(std::string_view Path) {
		if (nullptr != Manager) {
			const auto Importer = FbxImporter::Create(Manager, "");
			if (nullptr != Importer) {
				if (Importer->Initialize(data(Path), -1, Manager->GetIOSettings())) {
					Scene = FbxScene::Create(Manager, "");
					if (Importer->Import(Scene)) {
						int Major, Minor, Revision;
						Importer->GetFileVersion(Major, Minor, Revision);
						std::cout << Major << "." << Minor << "." << Revision << std::endl;

						//!< ノードをたどる
						Process(Scene->GetRootNode());

						//!< 直接取得
						//for (int i = 0; i < Scene->GetSrcObjectCount<FbxMesh>(); ++i) {
						//	const auto Mesh = Scene->GetSrcObject<FbxMesh>(i);
						//	if (nullptr != Mesh) {
						//		FbxArray<FbxVector4> Normals;
						//		if (Mesh->GetPolygonVertexNormals(Normals)) {
						//		}
						//		FbxStringList UVSetName;
						//		Mesh->GetUVSetNames(UVSetName);
						//		FbxArray<FbxVector2> UVs;
						//		if (Mesh->GetPolygonVertexUVs(UVSetName.GetStringAt(0), UVs)) {
						//		}
						//	}
						//}
						//for (int i = 0; i < Scene->GetMaterialCount(); ++i) {
						//	const auto Material = Scene->GetMaterial(i);
						//	if (nullptr != Material) {
						//		const auto Prop = Material->FindProperty(FbxSurfaceMaterial::sAmbient);
						//		if (Prop.IsValid()) {
						//			[[maybe_unused]] const auto& Color = Prop.Get<FbxDouble3>();
						//		}
						//	}
						//}
					}
				} else {
					std::cerr << Importer->GetStatus().GetErrorString() << std::endl;
					switch (Importer->GetStatus().GetCode()) {
					case FbxStatus::eSuccess:break;
					case FbxStatus::eFailure:break;
					case FbxStatus::eInsufficientMemory:break;
					case FbxStatus::eInvalidParameter:break;
					case FbxStatus::eIndexOutOfRange:break;
					case FbxStatus::ePasswordError:break;
					case FbxStatus::eInvalidFileVersion:break;
					case FbxStatus::eInvalidFile:break;
					case FbxStatus::eSceneCheckFail:break;
					}
					__debugbreak();
				}
				Importer->Destroy();
			}
		}
	}

protected:
	FbxManager* Manager = nullptr;
	FbxScene* Scene = nullptr;
};
