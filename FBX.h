#pragma once

#if 0
//#define FBXSDK_SHARED
#include <fbxsdk.h>

class Fbx
{
public:
	Fbx() {
		Manager = FbxManager::Create();
		const auto IOSettings = FbxIOSettings::Create(Manager, IOSROOT);
		if (nullptr != IOSettings) {
			Manager->SetIOSettings(IOSettings);
		}
	}
	virtual ~Fbx() {
		if (nullptr != Manager) {
			Manager->Destroy();
		}
	}

	virtual void Load([[maybe_unused]] std::string_view Path) {
		if (nullptr != Manager) {
			const auto Importer = FbxImporter::Create(Manager, "");
			if (nullptr != Importer) {
				if (Importer->Initialize(data(Path), -1, Manager->GetIOSettings())) {
					Scene = FbxScene::Create(Manager, "");
					if (Importer->Import(Scene)) {
						//!< ƒm[ƒh‚ð‚½‚Ç‚é
#pragma region TRAVERSE_NODE
						const auto RootNode = Scene->GetRootNode();
						if (nullptr != RootNode) {
							for (int i = 0; i < RootNode->GetChildCount(); i++) {
								const auto Node = RootNode->GetChild(i);
								if (nullptr != Node) {
									const auto Name = Node->GetName();
									const auto T = Node->LclTranslation.Get();
									const auto R = Node->LclRotation.Get();
									const auto S = Node->LclScaling.Get();
									for (int j = 0; j < Node->GetNodeAttributeCount(); ++j) {
										const auto Attr = Node->GetNodeAttributeByIndex(j);
										if (nullptr != Attr) {
											const auto AttrName = Attr->GetName();
											switch (Attr->GetAttributeType())
											{
											default:
												break;
											case FbxNodeAttribute::eUnknown:
												break;
											case FbxNodeAttribute::eNull:
												break;
											case FbxNodeAttribute::eMarker:
												break;
											case FbxNodeAttribute::eSkeleton:
												break;
											case FbxNodeAttribute::eMesh:
												break;
											case FbxNodeAttribute::eNurbs:
												break;
											case FbxNodeAttribute::ePatch:
												break;
											case FbxNodeAttribute::eCamera:
												break;
											case FbxNodeAttribute::eCameraStereo:
												break;
											case FbxNodeAttribute::eCameraSwitcher:
												break;
											case FbxNodeAttribute::eLight:
												break;
											case FbxNodeAttribute::eOpticalReference:
												break;
											case FbxNodeAttribute::eOpticalMarker:
												break;
											case FbxNodeAttribute::eNurbsCurve:
												break;
											case FbxNodeAttribute::eTrimNurbsSurface:
												break;
											case FbxNodeAttribute::eBoundary:
												break;
											case FbxNodeAttribute::eNurbsSurface:
												break;
											case FbxNodeAttribute::eShape:
												break;
											case FbxNodeAttribute::eLODGroup:
												break;
											case FbxNodeAttribute::eSubDiv:
												break;
											case FbxNodeAttribute::eCachedEffect:
												break;
											case FbxNodeAttribute::eLine:
												break;
											}
										}
									}
									for (int j = 0; j < Node->GetChildCount(); ++j) {
										const auto ChildNode = Node->GetChild(j);
									}
								}
							}
						}
#pragma endregion

						//!< ’¼ÚŽæ“¾
						for (int i = 0; i < Scene->GetSrcObjectCount<FbxMesh>(); ++i) {
							const auto Mesh = Scene->GetSrcObject<FbxMesh>(i);
							if (nullptr != Mesh) {
								FbxArray<FbxVector4> Normals;
								if (Mesh->GetPolygonVertexNormals(Normals)) {

								}
								FbxStringList UVSetName;
								Mesh->GetUVSetNames(UVSetName);
								FbxArray<FbxVector2> UVs;
								if (Mesh->GetPolygonVertexUVs(UVSetName.GetStringAt(0), UVs)) {

								}
							}
						}
						for (int i = 0; i < Scene->GetMaterialCount(); ++i) {
							const auto Material = Scene->GetMaterial(i);
							if (nullptr != Material) {
								const auto Prop = Material->FindProperty(FbxSurfaceMaterial::sAmbient);
								if (Prop.IsValid()) {
									const auto& Color = Prop.Get<FbxDouble3>();
								}
							}
						}
					}
				}
				Importer->Destroy();
			}
		}
	}

protected:
	FbxManager* Manager = nullptr;
	FbxScene* Scene = nullptr;
};

#endif