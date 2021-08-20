#pragma once

#include <vector>
#include <array>
#include <iostream>
#include <cassert>

//!< DLL ライブラリを使用する場合は FBXSDK_SHARED を定義する必要がある
#define FBXSDK_SHARED
#include <fbxsdk.h>

#include "Hierarchy.h"

static std::ostream& operator<<(std::ostream& lhs, const FbxVector2& rhs) { lhs << rhs[0] << ", " << rhs[1] << std::endl; return lhs; }
static std::ostream& operator<<(std::ostream& lhs, const FbxVector4& rhs) { lhs << rhs[0] << ", " << rhs[1] << ", " << rhs[2] << ", " << rhs[3] << std::endl; return lhs; }
//static std::ostream& operator<<(std::ostream& lhs, const FbxMatrix& rhs) { lhs << rhs.GetRow(0); lhs << rhs.GetRow(1); lhs << rhs.GetRow(2); lhs << rhs.GetRow(3); return lhs; }
static std::ostream& operator<<(std::ostream& lhs, const FbxColor& rhs) { lhs << rhs[0] << ", " << rhs[1] << ", " << rhs[2] << ", " << rhs[3] << std::endl; return lhs; }

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
		//Load(GetEnv("FBX_SDK_PATH") + "\\samples\\SwitchBinding\\Bind_Before_Switch.fbx"); //!< メッシュ、ジョイント、アニメーション、(四角形ポリゴン)
		//Load(GetEnv("FBX_SDK_PATH") + "\\samples\\Transformations\\JointHierarchy.fbx"); //!< ジョイントのみ
		//Load(GetEnv("FBX_SDK_PATH") + "\\samples\\UVSample\\sadface.fbx"); //!< メッシュ、テクスチャ
		//Load(GetEnv("FBX_SDK_PATH") + "\\samples\\ViewScene\\humanoid.fbx"); //!< ジョイント、メッシュ、アニメーション、カメラ、ライト
	}
	virtual ~Fbx() {
		if (nullptr != Scene) { Scene->Destroy(); }
		if (nullptr != Manager) { Manager->Destroy(); }
	}

	virtual void Process(FbxMesh* Mesh) {
		if (nullptr != Mesh) {
			Mesh->GenerateNormals();

#pragma region POLYGON
			for (auto i = 0; i < Mesh->GetPolygonCount(); ++i) {
				if (i > 8) { Tabs(); std::cout << "..." << std::endl; return; }

#pragma region MATERIAL
				for (auto j = 0; j < Mesh->GetElementMaterialCount(); ++j) {
					const auto ElementMaterial = Mesh->GetElementMaterial(j);
					const auto a = ElementMaterial->GetMappingMode();
					switch (ElementMaterial->GetMappingMode()) {
					default: break;
					//case FbxGeometryElementMaterial::eNone: break;
					//case FbxGeometryElementMaterial::eByControlPoint: break;
					//case FbxGeometryElementMaterial::eByPolygonVertex: break;
					case FbxGeometryElementMaterial::eByPolygon:
						Tabs(); std::cout << "[ Material ] (eByPolygon)" << std::endl;
						Tabs(); std::cout << "\t" << Mesh->GetNode()->GetMaterial(ElementMaterial->GetIndexArray().GetAt(i))->GetName() << std::endl;
						break;
					//case FbxGeometryElementMaterial::eByEdge: break;
					case FbxGeometryElementMaterial::eAllSame:
						Tabs(); std::cout << "[ Material ] (eAllSame)" << std::endl;
						//!< 全部一緒なので GetAt(0) で良い (GetAt(i)でも問題はないみたい)
						Tabs(); std::cout << "\t" << Mesh->GetNode()->GetMaterial(ElementMaterial->GetIndexArray().GetAt(0/*i*/))->GetName() << std::endl;
						break;
					}
				}
#pragma endregion

#pragma region POLYGON_GROUP
				if (Mesh->GetElementPolygonGroupCount()) { Tabs(); std::cout << "[ PolygonGroup ]" << std::endl; }
				for (auto j = 0; j < Mesh->GetElementPolygonGroupCount(); ++j) {
					if (j > 8) { Tabs(); std::cout << "..." << std::endl; return; }
					const auto PolygonGroup = Mesh->GetElementPolygonGroup(j);
					switch (PolygonGroup->GetMappingMode()) {
					default: break;
						//case FbxGeometryElementPolygonGroup::eNone: break;
						//case FbxGeometryElementPolygonGroup::eByControlPoint: break;
						//case FbxGeometryElementPolygonGroup::eByPolygonVertex: break;
					case FbxGeometryElementPolygonGroup::eByPolygon:
						if (FbxGeometryElement::eIndex == PolygonGroup->GetReferenceMode()) {
							Tabs(); std::cout << "\t" << PolygonGroup->GetIndexArray().GetAt(i);
						}
						break;
						//case FbxGeometryElementPolygonGroup::eByEdge: break;
						//case FbxGeometryElementPolygonGroup::eAllSame: break;
					}
				}
#pragma endregion

#pragma region POLYGON_SZIE
				for (auto j = 0; j < Mesh->GetPolygonSize(i); ++j) {
					Tabs(); std::cout << "[" << j << "]" << std::endl;
					const auto PolygonVertex = Mesh->GetPolygonVertex(i, j);
					if (0 <= PolygonVertex) {
#pragma region VERTEX
						Tabs(); std::cout << "\t" << "[ Vertex ]" << std::endl;
						Tabs(); std::cout << "\t" << "\t" << Mesh->GetControlPoints()[PolygonVertex];
#pragma endregion

#pragma region VERTEX_COLOR
						if (Mesh->GetElementVertexColorCount()) { Tabs(); std::cout << "\t" << "[ VertexColor ]" << std::endl; }
						for (auto k = 0; k < Mesh->GetElementVertexColorCount(); ++k) {
							if (k > 8) { Tabs(); std::cout << "\t" << "..." << std::endl; return; }
							const auto VertexColor = Mesh->GetElementVertexColor(k);
							switch (VertexColor->GetMappingMode()) {
							default: break;
								//case FbxGeometryElementVertexColor::eNone: break;
							case FbxGeometryElementVertexColor::eByControlPoint:
								Tabs(); std::cout << "\t" << "\t" << VertexColor->GetDirectArray().GetAt(j);
								break;
							case FbxGeometryElementVertexColor::eByPolygonVertex:
								Tabs(); std::cout << "\t" << "\t" << VertexColor->GetDirectArray().GetAt(VertexColor->GetIndexArray().GetAt(j));
								break;
								//case FbxGeometryElementVertexColor::eByPolygon: break;
								//case FbxGeometryElementVertexColor::eByEdge: break;
								//case FbxGeometryElementVertexColor::eAllSame: break;
							}
						}
#pragma endregion

#pragma region UV
						if (Mesh->GetElementUVCount()) { Tabs(); std::cout << "\t" << "[ UV ]" << std::endl; }
						for (auto k = 0; k < Mesh->GetElementUVCount(); ++k) {
							if (k > 8) { Tabs(); std::cout << "\t" << "..." << std::endl; return; }
							const auto UV = Mesh->GetElementUV(k);
							switch (UV->GetMappingMode()) {
							default: break;
								//case FbxGeometryElementUV::eNone: break;
							case FbxGeometryElementUV::eByControlPoint:
								switch (UV->GetReferenceMode()) {
								case FbxGeometryElementUV::eDirect:
									Tabs(); std::cout << "\t" << "\t" << UV->GetDirectArray().GetAt(PolygonVertex);
									break;
								case FbxGeometryElementUV::eIndexToDirect:
									Tabs(); std::cout << "\t" << "\t" << UV->GetDirectArray().GetAt(UV->GetIndexArray().GetAt(PolygonVertex));
									break;
									//case FbxGeometryElementUV::eIndex: break;
								default: break;
								}
								break;
							case FbxGeometryElementUV::eByPolygonVertex:
								switch (UV->GetReferenceMode())
								{
								case FbxGeometryElementUV::eDirect:
								case FbxGeometryElementUV::eIndexToDirect:
									Tabs(); std::cout << "\t" << "\t" << UV->GetDirectArray().GetAt(Mesh->GetTextureUVIndex(i, j));
									break;
								default: break;
									//case FbxGeometryElementUV::eIndex: break;
								}
								break;
								//case FbxGeometryElementUV::eByPolygon: break;
								//case FbxGeometryElementUV::eByEdge: break;
								//case FbxGeometryElementUV::eAllSame: break;
							}
						}
#pragma endregion

#pragma region NORMAL
						if (Mesh->GetElementNormalCount()) { Tabs(); std::cout << "\t" << "[ Normal ]" << std::endl; }
						for (auto k = 0; k < Mesh->GetElementNormalCount(); ++k) {
							if (k > 8) { Tabs(); std::cout << "\t" << "..." << std::endl; return; }
							const auto Normal = Mesh->GetElementNormal(k);
							switch (Normal->GetMappingMode()) {
							default: break;
								//case FbxGeometryElementNormal::eNone: break;
							case FbxGeometryElementNormal::eByControlPoint:
								if (FbxGeometryElement::eDirect == Normal->GetReferenceMode()) {
									Tabs(); std::cout << "\t" << "\t" << Normal->GetDirectArray().GetAt(i);
								}
								break;
							case FbxGeometryElementNormal::eByPolygonVertex:
								switch (Normal->GetReferenceMode()) {
								default: break;
								case FbxGeometryElementNormal::eDirect:
									Tabs(); std::cout << "\t" << "\t" << Normal->GetDirectArray().GetAt(j);
									break;
									//case FbxGeometryElementNormal::eIndex: break;
								case FbxGeometryElementNormal::eIndexToDirect:
									Tabs(); std::cout << "\t" << "\t" << Normal->GetDirectArray().GetAt(Normal->GetIndexArray().GetAt(j));
									break;
								}
								break;
								//case FbxGeometryElementNormal::eByPolygon: break;
								//case FbxGeometryElementNormal::eByEdge: break;
								//case FbxGeometryElementNormal::eAllSame: break;
							}
						}
#pragma endregion

#pragma region TANGENT
						if (Mesh->GetElementTangentCount()) { Tabs(); std::cout << "\t" << "[ Tangent ]" << std::endl; }
						for (auto k = 0; k < Mesh->GetElementTangentCount(); ++k) {
							if (k > 8) { Tabs(); std::cout << "\t" << "..." << std::endl; return; }
							const auto Tangent = Mesh->GetElementTangent(k);
							if (FbxGeometryElement::eByPolygonVertex == Tangent->GetMappingMode()) {
								switch (Tangent->GetReferenceMode()) {
								default: break;
								case FbxGeometryElementTangent::eDirect:
									Tabs(); std::cout << "\t" << "\t" << Tangent->GetDirectArray().GetAt(j);
									break;
									//case FbxGeometryElementTangent::eIndex: break;
								case FbxGeometryElementTangent::eIndexToDirect:
									Tabs(); std::cout << "\t" << "\t" << Tangent->GetDirectArray().GetAt(Tangent->GetIndexArray().GetAt(j));
									break;
								}
							}
						}
#pragma endregion

#pragma region BINORMAL
						if (Mesh->GetElementBinormalCount()) { Tabs(); std::cout << "\t" << "[ Binormal ]" << std::endl; }
						for (auto k = 0; k < Mesh->GetElementBinormalCount(); ++k) {
							if (k > 8) { Tabs(); std::cout << "\t" << "..." << std::endl; return; }
							const auto Binormal = Mesh->GetElementBinormal(k);
							if (FbxGeometryElement::eByPolygonVertex == Binormal->GetMappingMode()) {
								switch (Binormal->GetReferenceMode())
								{
								default: break;
								case FbxGeometryElementBinormal::eDirect:
									Tabs(); std::cout << "\t" << "\t" << Binormal->GetDirectArray().GetAt(j);
									break;
									//case FbxGeometryElementBinormal::eIndex: break;
								case FbxGeometryElementBinormal::eIndexToDirect:
									Tabs(); std::cout << "\t" << "\t" << Binormal->GetDirectArray().GetAt(Binormal->GetIndexArray().GetAt(j));
									break;
								}
							}
						}
#pragma endregion
					}
				}
#pragma endregion //!< POLYGON_SIZE
			}
#pragma endregion //!< POLYGON

#pragma region CONTROL_POINT
			for (auto i = 0; i < Mesh->GetControlPointsCount(); ++i) {
				if (i > 8) { Tabs(); std::cout << "..." << std::endl; return; }
				Tabs(); std::cout << "[ ControlPoint ]" << std::endl;
				Tabs(); std::cout << "\t" << Mesh->GetControlPoints()[i];

				if (Mesh->GetElementBinormalCount()) { Tabs(); std::cout << "[ Normal ]" << std::endl; }
				for (auto j = 0; j < Mesh->GetElementNormalCount(); ++j) {
					const auto Normal = Mesh->GetElementNormal(j);
					if (FbxGeometryElement::eByControlPoint == Normal->GetMappingMode()) {
						if (FbxGeometryElement::eDirect == Normal->GetReferenceMode()) {
							Tabs(); std::cout << "\t" << Normal->GetDirectArray().GetAt(i);
						}
					}
				}
			}
#pragma endregion //!< CONTROL_POINT

#pragma region VISIBILITY
			if (Mesh->GetElementVisibilityCount()) { Tabs(); std::cout << "[ Visibility ]" << std::endl; }
			for (auto i = 0; i < Mesh->GetElementVisibilityCount(); ++i) {
				const auto Visibility = Mesh->GetElementVisibility(i);
				if (FbxGeometryElement::eByEdge == Visibility->GetMappingMode()) {
					for (auto j = 0; j < Mesh->GetMeshEdgeCount(); ++j) {
						Tabs(); std::cout << "\t" << Visibility->GetDirectArray().GetAt(j);
					}
				}
			}
#pragma endregion

#pragma region DEFORMER
#pragma region SKIN
			for (auto i = 0; i < Mesh->GetDeformerCount(FbxDeformer::eSkin); ++i) {
				const auto Skin = FbxCast<FbxSkin>(Mesh->GetDeformer(i, FbxDeformer::eSkin));
				for (auto j = 0; j < Skin->GetClusterCount(); ++j) {
					const auto Cluster = Skin->GetCluster(j);
					switch (Cluster->GetLinkMode()) {
					default: break;
					case FbxCluster::eNormalize: break;
					case FbxCluster::eAdditive: break;
					case FbxCluster::eTotalOne: break;
					}
					Cluster->GetLink();
					for (auto k = 0; k < Cluster->GetControlPointIndicesCount(); ++k) {
						Cluster->GetControlPointIndices()[k];
						Cluster->GetControlPointWeights()[k];
					}
					FbxAMatrix Matrix;
					Cluster->GetTransformMatrix(Matrix);
					Cluster->GetTransformLinkMatrix(Matrix);
					if (nullptr != Cluster->GetAssociateModel()) {
						Cluster->GetTransformAssociateModelMatrix(Matrix);
					}
				}
			}
#pragma endregion
#pragma region BLEND_SHAPE
			for (auto i = 0; i < Mesh->GetDeformerCount(FbxDeformer::eBlendShape); ++i) {
				const auto BlendShape = FbxCast<FbxBlendShape>(Mesh->GetDeformer(i, FbxDeformer::eBlendShape));
				for (auto j = 0; j < BlendShape->GetBlendShapeChannelCount(); ++j) {
					const auto BlendShapeChannel = BlendShape->GetBlendShapeChannel(j);
					BlendShapeChannel->DeformPercent.Get();
					for (auto k = 0; k < BlendShapeChannel->GetTargetShapeCount(); ++k) {
						const auto Shape = BlendShapeChannel->GetTargetShape(k);
						if (FbxNodeAttribute::eMesh == Mesh->GetAttributeType()) {

						}
						else {
							FbxLayerElementArrayTemplate<FbxVector4>* Normals = nullptr;
							if (Shape->GetNormals(&Normals)) {}
							for (auto l = 0; l < Shape->GetControlPointsCount(); ++l) {
								Shape->GetControlPoints()[l];
								if (Normals) {
									Normals->GetAt(l);
								}
							}
						}
					}
				}
			}
#pragma endregion
#pragma region VERTEX_CACHE
			for (auto i = 0; i < Mesh->GetDeformerCount(FbxDeformer::eVertexCache); ++i) {
				const auto VertexCacheDeformer = FbxCast<FbxVertexCacheDeformer>(Mesh->GetDeformer(i, FbxDeformer::eVertexCache));
				const auto Cache = VertexCacheDeformer->GetCache();
				if (nullptr != Cache && Cache->OpenFileForRead()) {
					const auto Index = Cache->GetChannelIndex(VertexCacheDeformer->Channel.Get());
					if (Index >= 0) {
						FbxString Name;
						if (Cache->GetChannelName(Index, Name)) {}

						FbxString Interpolation;
						if (Cache->GetChannelInterpretation(Index, Interpolation)) {}

						FbxCache::EMCSamplingType SamplingType;
						if (Cache->GetChannelSamplingType(Index, SamplingType)) {
							switch (SamplingType) {
							case fbxsdk::FbxCache::eSamplingRegular: break;
							case fbxsdk::FbxCache::eSamplingIrregular: break;
							default: break;
							}
						}

						FbxTime Start, Stop;
						if (Cache->GetAnimationRange(Index, Start, Stop)) {}

						FbxTime Rate;
						if (Cache->GetChannelSamplingRate(Index, Rate)) {}

						unsigned int SampleCount;
						if (Cache->GetChannelSampleCount(Index, SampleCount)) {}

						FbxCache::EMCDataType Type;
						if (Cache->GetChannelDataType(Index, Type)) {
							switch (Type) {
							case fbxsdk::FbxCache::eUnknownData: break;
							case fbxsdk::FbxCache::eDouble: break;
							case fbxsdk::FbxCache::eDoubleArray: break;
							case fbxsdk::FbxCache::eDoubleVectorArray: break;
							case fbxsdk::FbxCache::eInt32Array: break;
							case fbxsdk::FbxCache::eFloatArray: break;
							case fbxsdk::FbxCache::eFloatVectorArray:
								for (auto t = Start; t <= Stop; t += Rate) {
									unsigned int ChannelPointCount;
									if (Cache->GetChannelPointCount(Index, t, ChannelPointCount)) {
										//Cache->Read(Index, t, Buffer, ChannelPointCount);
									}
								}
								break;
							default: break;
							}
						}
					}
					Cache->CloseFile();
				}
			}
#pragma endregion
#pragma endregion
		}
	}
	virtual void Process(FbxCamera* Camera) {
		if (nullptr != Camera) {
			switch (Camera->ProjectionType.Get()) {
				using enum FbxCamera::EProjectionType;
			case ePerspective:
				Tabs(); std::cout << "ePerspective" << std::endl;
				break;
			case eOrthogonal:
				Tabs(); std::cout << "eOrthogonal" << std::endl;
				break;
			}

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
			//Light->Color.Get();
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
	virtual void Process(FbxSkeleton* Skeleton) {
		if (nullptr != Skeleton) {
			switch (Skeleton->GetSkeletonType()) {
			default: break;
			case FbxSkeleton::eRoot:
				Skeleton->Size.Get();
				break;
			case FbxSkeleton::eLimb:
				Skeleton->LimbLength.Get();
				break;
			case FbxSkeleton::eLimbNode:
				Skeleton->Size.Get();
				break;
			case FbxSkeleton::eEffector: break;
			}
			Skeleton->GetLimbNodeColor();
			for (auto i = 0; i < Skeleton->GetSrcObjectCount<FbxObjectMetaData>(); ++i) {
				[[maybe_unused]] const auto MetaData = Skeleton->GetSrcObject<FbxObjectMetaData>(i);
			}
		}
	}
	virtual void Process(FbxLODGroup* LODGroup) {
		if (nullptr != LODGroup) {
			if (LODGroup->MinMaxDistance.Get()) {
				LODGroup->MinDistance.Get();
				LODGroup->MaxDistance.Get();
			}
			if (LODGroup->WorldSpace.Get()) {}
			for (auto i = 0; i < LODGroup->GetNumThresholds(); ++i) {
				FbxDistance Threshold;
				if (LODGroup->GetThreshold(i, Threshold) || LODGroup->ThresholdsUsedAsPercentage.Get()) {
					//!< ThresholdsUsedAsPercentage の場合には GetThreshold() は false を返すが Threshold には(パーセンテージの)値が返っている
					Threshold.value();
				}
			}
			for (auto i = 0; i < LODGroup->GetNumDisplayLevels(); ++i) {
				FbxLODGroup::EDisplayLevel DisplayLevel;
				if (LODGroup->GetDisplayLevel(i, DisplayLevel)) {
					switch (DisplayLevel) {
					case fbxsdk::FbxLODGroup::eUseLOD: break;
					case fbxsdk::FbxLODGroup::eShow: break;
					case fbxsdk::FbxLODGroup::eHide: break;
					default: break;
					}
				}
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
				PushTab();
				Process(FbxCast<FbxSkeleton>(Attr));
				PopTab();
				break;
			case eMesh:
				PushTab();
				Process(FbxCast<FbxMesh>(Attr));
				PopTab();
				break;
			case eNurbs:
				break;
			case ePatch:
				break;
			case eCamera:
				PushTab();
				Process(FbxCast<FbxCamera>(Attr));
				PopTab();
				break;
			case eCameraStereo:
				break;
			case eCameraSwitcher:
				break;
			case eLight:
				PushTab();
				Process(FbxCast<FbxLight>(Attr));
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
				PushTab();
				Process(FbxCast<FbxLODGroup>(Attr));
				PopTab();
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
			[[maybe_unused]] const auto GTransAnim = Node->EvaluateGlobalTransform(Time);

			const auto Evaluator = Scene->GetAnimationEvaluator();
			if (nullptr != Evaluator) {
				const FbxTime t(32);
				const auto GlobalMatrix = Evaluator->GetNodeGlobalTransform(Node, t);
				const auto LocalMatrix = Evaluator->GetNodeLocalTransform(Node, t);

				const auto LocalTranslation = Evaluator->GetNodeLocalTranslation(Node, t);
				const auto LocalRotation = Evaluator->GetNodeLocalRotation(Node, t);
				const auto LocalScaling = Evaluator->GetNodeLocalScaling(Node, t);
			}

			PushTab();
			for (auto i = 0; i < Node->GetNodeAttributeCount(); ++i) {
				Process(Node->GetNodeAttributeByIndex(i));
			}
			PopTab();

			PushTab();
			for (auto i = 0; i < Node->GetChildCount(); i++) {
				Process(Node->GetChild(i));
			}
			PopTab();
		}
	}
	virtual void Process(FbxSurfaceMaterial* Material) {
		if (nullptr != Material) {
			Tabs(); std::cout << "[ Material ] " << Material->GetName() << std::endl;
			Tabs(); std::cout << "\t" << Material->ShadingModel.Get() << std::endl;

			constexpr std::array Implementations = { FBXSDK_IMPLEMENTATION_CGFX, FBXSDK_IMPLEMENTATION_HLSL, FBXSDK_IMPLEMENTATION_SFX, FBXSDK_IMPLEMENTATION_OGS, FBXSDK_IMPLEMENTATION_SSSL };
			const FbxImplementation* Implementation = nullptr;
			for (auto i : Implementations) {
				Implementation = GetImplementation(Material, i);
				if (nullptr != Implementation) { break; }
			}
#pragma region IMPLEMENTATION
			if (nullptr != Implementation) {
				Tabs(); std::cout << "\t" << Implementation->Language.Get() << std::endl;
				Tabs(); std::cout << "\t" << Implementation->LanguageVersion.Get() << std::endl;
				Tabs(); std::cout << "\t" << Implementation->RenderName << std::endl;
				Tabs(); std::cout << "\t" << Implementation->RenderAPI.Get() << std::endl;
				Tabs(); std::cout << "\t" << Implementation->RenderAPIVersion.Get() << std::endl;

				const auto RootTable = Implementation->GetRootTable();
				Tabs(); std::cout << "\t" << RootTable->DescAbsoluteURL.Get() << std::endl;
				Tabs(); std::cout << "\t" << RootTable->DescTAG.Get() << std::endl;
				for (auto j = 0; j < RootTable->GetEntryCount(); ++j) {
					const auto Entry = RootTable->GetEntry(j);
					const auto EntryType = Entry.GetEntryType(true);

					FbxProperty Prop;
					if (0 == std::strcmp(FbxPropertyEntryView::sEntryType, EntryType)) {
						Prop = Material->FindPropertyHierarchical(Entry.GetSource());
						if (!Prop.IsValid()) {
							Prop = Material->RootProperty.FindHierarchical(Entry.GetSource());
						}
					}
					else if (0 == std::strcmp(FbxConstantEntryView::sEntryType, EntryType)) {
						Prop = Implementation->GetConstants().FindHierarchical(Entry.GetSource());
					}
					if (Prop.IsValid()) {
						if (Prop.GetSrcObjectCount<FbxTexture>() > 0) {
							for (int k = 0; k < Prop.GetSrcObjectCount<FbxFileTexture>(); ++k) {
								Tabs(); std::cout << "\t" << "[ FileTexture ] " << Prop.GetSrcObject<FbxFileTexture>(k)->GetName() << std::endl;
							}
							for (int k = 0; k < Prop.GetSrcObjectCount<FbxLayeredTexture>(); ++k) {
								Tabs(); std::cout << "\t" << "[ LayeredTexture ] " << Prop.GetSrcObject<FbxLayeredTexture>(k)->GetName() << std::endl;
							}
							for (int k = 0; k < Prop.GetSrcObjectCount<FbxProceduralTexture>(); ++k) {
								Tabs(); std::cout << "\t" << "[ ProceduralTexture ] " << Prop.GetSrcObject<FbxProceduralTexture>(k)->GetName() << std::endl;
							}
						}
						else {
							const auto PropType = Prop.GetPropertyDataType();
							if (FbxBoolDT == PropType) {
								Prop.Get<FbxBool>();
							}
							else if (FbxIntDT == PropType || FbxEnumDT == PropType) {
								Prop.Get<FbxInt>();
							}
							else if (FbxFloatDT == PropType) {
								Prop.Get<FbxFloat>();
							}
							else if (FbxDoubleDT == PropType) {
								Prop.Get<FbxDouble>();
							}
							else if (FbxStringDT == PropType || FbxUrlDT == PropType || FbxXRefUrlDT == PropType) {
								Prop.Get<FbxString>();
							}
							else if (FbxDouble2DT == PropType) {
								Prop.Get<FbxDouble2>();
							}
							else if (FbxDouble3DT == PropType || FbxColor3DT == PropType) {
								Prop.Get<FbxDouble3>();
							}
							else if (FbxDouble4DT == PropType || FbxColor4DT == PropType) {
								Prop.Get<FbxDouble4>();
							}
							else if (FbxDouble4x4DT == PropType) {
								Prop.Get<FbxDouble4x4>();
							}
						}
					}
				}
			}
#pragma endregion
#pragma region PHONG_LAMBERT
			else {
				const auto Lambert = FbxCast<FbxSurfaceLambert>(Material);
				if (nullptr != Lambert) {
					const auto Emissive = Lambert->Emissive.Get();
					const auto EmissiveFactor = Lambert->EmissiveFactor.Get();
					Tabs(); std::cout << "\t\t" << "Emissive = " << Emissive[0] << ", " << Emissive[1] << ", " << Emissive[2] << ", EmissiveFactor = " << EmissiveFactor << std::endl;

					const auto Ambient = Lambert->Ambient.Get();
					const auto AmbientFactor = Lambert->AmbientFactor.Get();
					Tabs(); std::cout << "\t\t" << "Ambient = " << Ambient[0] << ", " << Ambient[1] << ", " << Ambient[2] << ", AmbientFactor = " << AmbientFactor << std::endl;

					const auto Diffuse = Lambert->Diffuse.Get();
					const auto DiffuseFactor = Lambert->DiffuseFactor.Get();
					Tabs(); std::cout << "\t\t" << "Diffuse = " << Diffuse[0] << ", " << Diffuse[1] << ", " << Diffuse[2] << ", DiffuseFactor = " << DiffuseFactor << std::endl;
					//!< テクスチャの有無を調べる例
					for (auto i = 0; i < Lambert->Diffuse.GetSrcObjectCount<FbxFileTexture>(); ++i) {
						const auto FileTexture = Lambert->Diffuse.GetSrcObject<FbxFileTexture>(i);
						if (nullptr != FileTexture) {
							Tabs(); std::cout << "\t\t\t" << FileTexture->GetRelativeFileName() << std::endl;
						}
					}

					const auto NormalMap = Lambert->NormalMap.Get();
					Tabs(); std::cout << "\t\t" << "NormalMap = " << NormalMap[0] << ", " << NormalMap[1] << ", " << NormalMap[2] << std::endl;
					for (auto i = 0; i < Lambert->NormalMap.GetSrcObjectCount<FbxFileTexture>(); ++i) {
						const auto FileTexture = Lambert->NormalMap.GetSrcObject<FbxFileTexture>(i);
						if (nullptr != FileTexture) {
							Tabs(); std::cout << "\t\t\t" << FileTexture->GetRelativeFileName() << std::endl;
						}
					}

					const auto Bump = Lambert->Bump.Get();
					const auto BumpFactor = Lambert->BumpFactor.Get();
					Tabs(); std::cout << "\t\t" << "Bump = " << Bump[0] << ", " << Bump[1] << ", " << Bump[2] << ", BumpFactor = " << BumpFactor << std::endl;

					const auto TransparentColor = Lambert->TransparentColor.Get();
					const auto TransparencyFactor = Lambert->TransparencyFactor.Get();
					Tabs(); std::cout << "\t\t" << "TransparentColor = " << TransparentColor[0] << ", " << TransparentColor[1] << ", " << TransparentColor[2] << ", TransparentFactor = " << TransparencyFactor << std::endl;

					const auto DisplacementColor = Lambert->DisplacementColor.Get();
					const auto DisplacementFactor = Lambert->DisplacementFactor.Get();
					Tabs(); std::cout << "\t\t" << "DisplacementColor = " << DisplacementColor[0] << ", " << DisplacementColor[1] << ", " << DisplacementColor[2] << ", DisplacementFactor = " << DisplacementFactor << std::endl;

					const auto VectorDisplacementColor = Lambert->VectorDisplacementColor.Get();
					const auto VectorDisplacementFactor = Lambert->VectorDisplacementFactor.Get();
					Tabs(); std::cout << "\t\t" << "VectorDisplacementColor = " << VectorDisplacementColor[0] << ", " << VectorDisplacementColor[1] << ", " << VectorDisplacementColor[2] << ", VectorDisplacementFactor = " << VectorDisplacementFactor << std::endl;

					const auto Phong = FbxCast<FbxSurfacePhong>(Lambert);
					if (nullptr != Phong) {
						const auto Specular = Phong->Specular.Get();
						const auto SpecularFactor = Phong->SpecularFactor.Get();
						Tabs(); std::cout << "\t\t" << "Specular = " << Specular[0] << ", " << Specular[1] << ", " << Specular[2] << ", SpecularFactor = " << SpecularFactor << std::endl;

						const auto Shiness = Phong->Shininess.Get();
						Tabs(); std::cout << "\t\t" << "Shiness = " << Shiness << std::endl;

						const auto Reflection = Phong->Reflection.Get();
						const auto ReflectionFactor = Phong->ReflectionFactor.Get();
						Tabs(); std::cout << "\t\t" << "Reflection = " << Reflection[0] << ", " << Reflection[1] << ", " << Reflection[2] << ", ReflectionFactor = " << ReflectionFactor << std::endl;
					}
				}
			}
#pragma endregion

#pragma region TEXTURE
			for (auto i = 0; i < FbxLayerElement::sTypeTextureCount; ++i) {
				const auto Prop = Material->FindProperty(FbxLayerElement::sTextureChannelNames[i]);
				if (Prop.IsValid()) {
					for (auto j = 0; j < Prop.GetSrcObjectCount<FbxTexture>(); ++j) {
						const auto LayeredTexture = Prop.GetSrcObject<FbxLayeredTexture>(j);
						if (nullptr != LayeredTexture) {
							PushTab();
							Process(LayeredTexture);
							PopTab();
						}
						else {
							PushTab();
							Process(Prop.GetSrcObject<FbxTexture>(j));
							PopTab();
						}
					}
				}
			}
#pragma endregion
		}
	}
	virtual void Process(FbxLayeredTexture* LayeredTexture) {
		if (nullptr != LayeredTexture) {
			Tabs(); std::cout << "[ LayeredTexture ] " << LayeredTexture->GetName() << std::endl;
			for (auto i = 0; i < LayeredTexture->GetSrcObjectCount<FbxTexture>(); ++i) {

				FbxLayeredTexture::EBlendMode BlendMode;
				if (LayeredTexture->GetTextureBlendMode(i, BlendMode)) {
					switch (BlendMode) {
					case fbxsdk::FbxLayeredTexture::eTranslucent: break;
					case fbxsdk::FbxLayeredTexture::eAdditive: break;
					case fbxsdk::FbxLayeredTexture::eModulate: break;
					case fbxsdk::FbxLayeredTexture::eModulate2: break;
					case fbxsdk::FbxLayeredTexture::eOver: break;
					case fbxsdk::FbxLayeredTexture::eNormal: break;
					case fbxsdk::FbxLayeredTexture::eDissolve: break;
					case fbxsdk::FbxLayeredTexture::eDarken: break;
					case fbxsdk::FbxLayeredTexture::eColorBurn: break;
					case fbxsdk::FbxLayeredTexture::eLinearBurn: break;
					case fbxsdk::FbxLayeredTexture::eDarkerColor: break;
					case fbxsdk::FbxLayeredTexture::eLighten: break;
					case fbxsdk::FbxLayeredTexture::eScreen: break;
					case fbxsdk::FbxLayeredTexture::eColorDodge: break;
					case fbxsdk::FbxLayeredTexture::eLinearDodge: break;
					case fbxsdk::FbxLayeredTexture::eLighterColor: break;
					case fbxsdk::FbxLayeredTexture::eSoftLight: break;
					case fbxsdk::FbxLayeredTexture::eHardLight: break;
					case fbxsdk::FbxLayeredTexture::eVividLight: break;
					case fbxsdk::FbxLayeredTexture::eLinearLight: break;
					case fbxsdk::FbxLayeredTexture::ePinLight: break;
					case fbxsdk::FbxLayeredTexture::eHardMix: break;
					case fbxsdk::FbxLayeredTexture::eDifference: break;
					case fbxsdk::FbxLayeredTexture::eExclusion: break;
					case fbxsdk::FbxLayeredTexture::eSubtract: break;
					case fbxsdk::FbxLayeredTexture::eDivide: break;
					case fbxsdk::FbxLayeredTexture::eHue: break;
					case fbxsdk::FbxLayeredTexture::eSaturation: break;
					case fbxsdk::FbxLayeredTexture::eColor: break;
					case fbxsdk::FbxLayeredTexture::eLuminosity: break;
					case fbxsdk::FbxLayeredTexture::eOverlay: break;
					case fbxsdk::FbxLayeredTexture::eBlendModeCount: break;
					default: break;
					}
				}

				PushTab();
				Process(LayeredTexture->GetSrcObject<FbxTexture>(i));
				PopTab();
			}
		}
	}
	virtual void Process(FbxTexture* Texture) {
		if (nullptr != Texture) {
			Tabs(); std::cout << "[ Texture ] " << Texture->GetName() << std::endl;

			const auto FileTexture = FbxCast<FbxFileTexture>(Texture);
			if (nullptr != FileTexture) {
				switch (FileTexture->GetMaterialUse()) {
				default: break;
				case FbxFileTexture::eModelMaterial: break;
				case FbxFileTexture::eDefaultMaterial: break;
				}
				Tabs(); std::cout << "\t" << "[ FileTexture ]" << std::endl;
				Tabs(); std::cout << "\t\t" << FileTexture->GetRelativeFileName() << " (" << FileTexture->GetFileName() << ") " << std::endl;
			}

			const auto ProceduralTexture = FbxCast<FbxProceduralTexture>(Texture);
			if (nullptr != ProceduralTexture) {
				Tabs(); std::cout << "\t" << "[ ProceduralTexture ]" << std::endl;
			}

			Texture->GetScaleU(); Texture->GetScaleV();
			Texture->GetTranslationU();Texture->GetTranslationV();
			Texture->GetSwapUV();
			Texture->GetRotationU(); Texture->GetRotationV(); Texture->GetRotationW();
			switch (Texture->GetAlphaSource()) {
			default: break;
			case FbxTexture::eNone: break;
			case FbxTexture::eRGBIntensity: break;
			case FbxTexture::eBlack: break;
			};
			Texture->GetCroppingLeft();Texture->GetCroppingTop();Texture->GetCroppingRight();Texture->GetCroppingBottom();
			switch (Texture->GetMappingType()) {
			default: break;
			case FbxTexture::eNull: break;
			case FbxTexture::ePlanar:
				switch (Texture->GetPlanarMappingNormal()) {
				default: break;
				case FbxTexture::ePlanarNormalX: break;
				case FbxTexture::ePlanarNormalY: break;
				case FbxTexture::ePlanarNormalZ: break;
				};
				break;
			case FbxTexture::eSpherical: break;
			case FbxTexture::eCylindrical: break;
			case FbxTexture::eBox: break;
			case FbxTexture::eFace: break;
			case FbxTexture::eUV: break;
			case FbxTexture::eEnvironment: break;
			}
			Texture->GetDefaultAlpha();
			switch (Texture->GetTextureUse()) {
			default: break;
			case FbxTexture::eStandard: break;
			case FbxTexture::eShadowMap: break;
			case FbxTexture::eLightMap: break;
			case FbxTexture::eSphericalReflectionMap: break;
			case FbxTexture::eSphereReflectionMap: break;
			case FbxTexture::eBumpNormalMap: break;
			}
		}
	}

	virtual void Convert(FbxScene* Scn) {
		if (nullptr != Scn) {
			//!< 【OpenGL】Right Hand
			FbxAxisSystem::OpenGL.ConvertScene(Scn);
			//!< 【DirectX】Left Hand
			//FbxAxisSystem::DirectX.ConvertScene(Scn);

			//!< 【単位】m (メートル)
			FbxSystemUnit::m.ConvertScene(Scn);

			if (FbxGeometryConverter(Manager).Triangulate(Scn, true)) {
			}
		}
	}

	virtual void Load(std::string_view Path) {
		if (nullptr != Manager) {
			const auto Importer = FbxImporter::Create(Manager, "");
			if (nullptr != Importer) {
				if (Importer->Initialize(data(Path), -1, Manager->GetIOSettings())) {
					Scene = FbxScene::Create(Manager, "");
					if (Importer->Import(Scene)) {
						
						Convert(Scene);

						int Major, Minor, Revision;
						Importer->GetFileVersion(Major, Minor, Revision);
						std::cout << Major << "." << Minor << "." << Revision << std::endl;
						
						//!< テクスチャを列挙しておく
						for (auto i = 0; i < Scene->GetTextureCount(); ++i) {
							const auto LayeredTexture = FbxCast<FbxLayeredTexture>(Scene->GetTexture(i));
							if (nullptr != LayeredTexture) {
								Process(LayeredTexture);
							}
							else {
								Process(Scene->GetTexture(i));
							}
						}
						//!< マテリアルを列挙しておく
						for (auto i = 0; i < Scene->GetMaterialCount(); ++i) {
							Process(Scene->GetMaterial(i));
						}
						//!< アニメーション
						Tabs(); std::cout << "[ AnimStack ]" << std::endl;
						for (auto i = 0; i < Scene->GetSrcObjectCount<FbxAnimStack>(); ++i) {
							const auto AnimStack = Scene->GetSrcObject<FbxAnimStack>(i);
							Tabs(); std::cout << "\t" << AnimStack->GetName() << std::endl;

							Tabs(); std::cout << "\t" << "[ AnimLayer ]" << std::endl;
							for (auto j = 0; j < AnimStack->GetMemberCount<FbxAnimLayer>(); ++j) {
								const auto AnimLayer = AnimStack->GetMember<FbxAnimLayer>(j);
								Tabs(); std::cout << "\t\t" << AnimLayer->GetName() << std::endl;

								const FbxTime t;
								const auto Weight = AnimLayer->Weight.EvaluateValue(t);
								//AnimLayer->Mute;
								//AnimLayer->Solo;
								//AnimLayer->Lock;
								//AnimLayer->Color;
								//AnimLayer->BlendMode;
								//AnimLayer->RotationAccumulationMode;
								//AnimLayer->ScaleAccumulationMode;
								//AnimLayer->GetBlendModeBypass();
							}
						}

						//!< ノードをたどる
						Process(Scene->GetRootNode());
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
