#pragma once

#include <vector>
#include <algorithm>

#include "Math.h"
using namespace Math;

#include "Shape.h"
#include "RigidBody.h"

#include "Colli.h"
using namespace Collision;

namespace Physics
{
	class Scene 
	{
	public:
		using CollidablePair = std::pair<int, int>;

		virtual ~Scene() {
			for (auto i : RigidBodies) {
				if (nullptr != i) {
					delete i;
				}
			}
		}

		virtual void Init() 
		{
			//!< 動的オブジェクト配置
			{
				constexpr auto Radius = 0.5f;
				constexpr auto Y = 10.0f;

				const auto n = 6;
				const auto n2 = n >> 1;
				for (auto x = 0; x < n; ++x) {
					for (auto z = 0; z < n; ++z) {
						auto Rb = RigidBodies.emplace_back(new RigidBody());
						Rb->Position = Vec3(static_cast<float>(x - n2) * Radius * 2.0f * 1.5f, Y, static_cast<float>(z - n2) * Radius * 2.0f * 1.5f);
						Rb->Init(new ShapeSphere(Radius));
					}
				}
			}

			//!< 静的オブジェクト配置
			{
				constexpr auto Radius = 80.0f;
				constexpr auto Y = -Radius;

				const auto n = 3;
				const auto n2 = n >> 1;
				for (auto x = 0; x < n; ++x) {
					for (auto z = 0; z < n; ++z) {
						auto Rb = RigidBodies.emplace_back(new RigidBody());
						Rb->Position = Vec3(static_cast<float>(x - n2) * Radius * 0.25f, Y, static_cast<float>(z - n2) * Radius * 0.25f);
						Rb->InvMass = 0;
						Rb->Elasticity = 0.99f;
						Rb->Init(new ShapeSphere(Radius));
					}
				}
			}

#if false
			{
				const std::vector Hoge = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
				std::cout << IndexOf(Hoge, Hoge[4]) << std::endl;
				std::cout << std::distance(std::begin(Hoge), std::ranges::find(Hoge, 4)) << std::endl;
			}
#endif

#if false
			{
				const auto Lambda = Vec4(1, 0, 1, 0);
				std::vector SimplexPoints = {
					SupportPoints(Vec3::AxisX(), Vec3::Zero()),
					SupportPoints(Vec3::AxisY(), Vec3::Zero()),
					SupportPoints(Vec3::AxisZ(), Vec3::Zero()),
					SupportPoints(Vec3::One(), Vec3::Zero()),
				};

				const auto [B, E] = std::ranges::remove_if(SimplexPoints, [&](const auto& rhs) { return 0.0f == Lambda[static_cast<int>(IndexOf(SimplexPoints, rhs))]; });
				SimplexPoints.erase(B, E);

				for (auto i : SimplexPoints) {
					std::cout << i.GetA() << std::endl;
				}
			}
#endif

#if false
			const std::array OrgPts = { Vec3::Zero(), Vec3::AxisX(), Vec3::AxisY(), Vec3::AxisZ() };
			{
				const auto Tmp = Vec3::One();
				const std::array Pts = { Vec3::Zero() + Tmp, Vec3::AxisX() + Tmp, Vec3::AxisY() + Tmp, Vec3::AxisZ() + Tmp };
				{
					const auto Lambda1 = SignedVolume(Pts[0], Pts[1]);
					std::cout << Lambda1 << "== 1, 0" << std::endl << std::endl;
					const auto Lambda2 = SignedVolume(Pts[0], Pts[1], Pts[2]);
					std::cout << Lambda2 << "== 1, 0, 0" << std::endl << std::endl;
				}
				const auto Lambda = SignedVolume(Pts[0], Pts[1], Pts[2], Pts[3]);

				std::cout << Lambda << "== 1, 0, 0, 0" << std::endl << std::endl;
			}
			{
				const auto Tmp = -Vec3::One() * 0.25f;
				const std::array Pts = { Vec3::Zero() + Tmp, Vec3::AxisX() + Tmp, Vec3::AxisY() + Tmp, Vec3::AxisZ() + Tmp };
				{
					const auto Lambda1 = SignedVolume(Pts[0], Pts[1]);
					std::cout << Lambda1 << "== 0.75, 0.25" << std::endl << std::endl;
					const auto Lambda2 = SignedVolume(Pts[0], Pts[1], Pts[2]);
					std::cout << Lambda2 << "== 0.5, 0.25, 0.25" << std::endl << std::endl;
				}
				const auto Lambda = SignedVolume(Pts[0], Pts[1], Pts[2], Pts[3]);

				std::cout << Lambda << "== 0.25, 0.25, 0.25, 0.25" << std::endl << std::endl;
			}
			{
				const auto Tmp = -Vec3::One();
				const std::array Pts = { Vec3::Zero() + Tmp, Vec3::AxisX() + Tmp, Vec3::AxisY() + Tmp, Vec3::AxisZ() + Tmp };
				{
					const auto Lambda1 = SignedVolume(Pts[0], Pts[1]);
					std::cout << Lambda1 << "== 0, 1" << std::endl << std::endl;
					const auto Lambda2 = SignedVolume(Pts[0], Pts[1], Pts[2]);
					std::cout << Lambda2 << "== 0, 0.5, 0.5" << std::endl << std::endl;
				}
				const auto Lambda = SignedVolume(Pts[0], Pts[1], Pts[2], Pts[3]);

				std::cout << Lambda << "== 0, 0.333, 0.333, 0.333" << std::endl << std::endl;
			}
			{
				const auto Tmp = Vec3(1.0f, 1.0f, -0.5f);
				const std::array Pts = { Vec3::Zero() + Tmp, Vec3::AxisX() + Tmp, Vec3::AxisY() + Tmp, Vec3::AxisZ() + Tmp };
				{
					const auto Lambda1 = SignedVolume(Pts[0], Pts[1]);
					std::cout << Lambda1 << "== 1, 0" << std::endl << std::endl;
					const auto Lambda2 = SignedVolume(Pts[0], Pts[1], Pts[2]);
					std::cout << Lambda2 << "== 1, 0, 0" << std::endl << std::endl;
				}
				const auto Lambda = SignedVolume(Pts[0], Pts[1], Pts[2], Pts[3]);

				std::cout << Lambda << "== 0.5, 0, 0, 0.5" << std::endl << std::endl;
			}
			{
				const std::array Pts = { Vec3(51.1996613f, 26.1989613f, 1.91339576f), Vec3(-51.0567360f, -26.0565681f, -0.436143428f), Vec3(50.8978920f, -24.1035538f, -1.04042661f), Vec3(-49.1021080f, 25.8964462f, -1.04042661f) };
				{
					const auto Lambda1 = SignedVolume(Pts[0], Pts[1]);
					std::cout << Lambda1 << "== 0.499, 0.501" << std::endl << std::endl;
					const auto Lambda2 = SignedVolume(Pts[0], Pts[1], Pts[2]);
					std::cout << Lambda2 << "== 0.498, 0.501, 0.002" << std::endl << std::endl;
				}
				const auto Lambda = SignedVolume(Pts[0], Pts[1], Pts[2], Pts[3]);

				std::cout << Lambda << "== 0.29, 0.302, 0.206, 0.202" << std::endl << std::endl;
			}
#endif
		}
		virtual void BroadPhase(std::vector<CollidablePair>& CollidablePairs, const float DeltaSec)
		{
			std::vector<BoundEdge> BoundEdges;
			{
				//!< (ここでは) この軸に射影した AABB の範囲を計算
				const auto Axis = Vec3(1.0f, 1.0f, 1.0f).Normalize();

				const auto RbCount = size(RigidBodies);
				BoundEdges.reserve(RbCount * 2);
				for (auto i = 0; i < RbCount; ++i) {
					const auto Rb = RigidBodies[i];

					auto Aabb = Rb->Shape->GetAABB(Rb->Position, Rb->Rotation);
					//!< 速度分 AABB を拡張する
					Aabb.Expand(Aabb.Min + Rb->LinearVelocity * DeltaSec);
					Aabb.Expand(Aabb.Max + Rb->LinearVelocity * DeltaSec);

					//!< さらに少し拡張 (取りこぼし防止？)
					const auto Epsilon = 0.01f;
					Aabb.Expand(Aabb.Min - Vec3::One() * Epsilon);
					Aabb.Expand(Aabb.Max + Vec3::One() * Epsilon);

					BoundEdges.emplace_back(BoundEdge({ i, Axis.Dot(Aabb.Min), true })); //!< 下限
					BoundEdges.emplace_back(BoundEdge({ i, Axis.Dot(Aabb.Max), false }));//!< 上限
				}

				//!< 軸に射影した値でソート
				std::ranges::sort(BoundEdges, std::ranges::less{}, &BoundEdge::Value);
			}

			//!< 射影 AABB から、潜在的衝突ペアリストを構築
			const auto BoundsCount = size(BoundEdges);
			for (auto i = 0; i < BoundsCount; ++i) {
				const auto& A = BoundEdges[i];
				//!< Min なら対となる Max が見つかるまで探す
				if (A.IsMin) {
					for (auto j = i + 1; j < BoundsCount; ++j) {
						const auto& B = BoundEdges[j];
						//!< 対となる Max (同じインデックス) が見つかれば終了
						if (A.Index == B.Index) {
							break;
						}
						//!< 他オブジェクトが見つかった場合は、潜在的衝突相手として収集
						if (B.IsMin) {
							CollidablePairs.emplace_back(CollidablePair({ A.Index, B.Index }));
						}
					}
				}
			}
		}
		virtual void NarrowPhase(std::vector<Contact>& Contacts, const std::vector<CollidablePair>& CollidablePairs, const float DeltaSec)
		{
			Contacts.reserve(std::size(CollidablePairs));
			Contacts.clear();

			//!< 潜在的衝突相手と、実際に衝突しているかを調べる
			for (auto i : CollidablePairs) {
				const auto RbA = RigidBodies[i.first];
				const auto RbB = RigidBodies[i.second];
				if (0.0f != RbA->InvMass || 0.0f != RbB->InvMass) {
					Contact Ct;
					if (Collision::Intersection::RigidBodies(RbA, RbB, DeltaSec, Ct)) {
						//!< 衝突を収集
						Contacts.emplace_back(Ct);
					}
				}
			}
			//!< TOI でソート
			std::ranges::sort(Contacts, std::ranges::less{}, &Contact::TimeOfImpact);
		}
		virtual void BruteForce(std::vector<Contact>& Contacts, const float DeltaSec)
		{
			const auto RbCount = size(RigidBodies);
			Contacts.reserve(RbCount * RbCount);
			Contacts.clear();
			for (auto i = 0; i < RbCount; ++i) {
				for (auto j = i + 1; j < RbCount; ++j) {
					const auto RbA = RigidBodies[i];
					const auto RbB = RigidBodies[j];
					if (0.0f != RbA->InvMass || 0.0f != RbB->InvMass) {
						Contact Ct;
						if (Collision::Intersection::RigidBodies(RbA, RbB, DeltaSec, Ct)) {
							Contacts.emplace_back(Ct);
						}
					}
				}
			}
			std::ranges::sort(Contacts, std::ranges::less{}, &Contact::TimeOfImpact);
		}
		virtual void Update(const float DeltaSec)
		{
			//!< 重力
			for (auto i : RigidBodies) {
				i->ApplyGravity(DeltaSec);
			}

			//!< 衝突 Contacts を収集
			std::vector<Contact> Contacts;
#if true
			{
				std::vector<CollidablePair> CollidablePairs;
				BroadPhase(CollidablePairs, DeltaSec);
				NarrowPhase(Contacts, CollidablePairs, DeltaSec);
			}
#else
			BruteForce(Contacts, DeltaSec);
#endif

			//!< TOI 毎に時間をスライスして、シミュレーションを進める
			auto AccumTime = 0.0f;
			for (auto i : Contacts) {
				//!< 次の衝突までの時間
				const auto Delta = i.TimeOfImpact - AccumTime;

				//!< 次の衝突までシミュレーションを進める
				for (auto j : RigidBodies) {
					j->Update(Delta);
				}

				//!< 衝突の解決
				Resolve(i);

				AccumTime += Delta;
			}

			//!< 残りのシミュレーションを進める
			const auto Delta = DeltaSec - AccumTime;
			if (0.0f < Delta) {
				for (auto i : RigidBodies) {
					i->Update(Delta);
				}
			}
		}

		std::vector<RigidBody *> RigidBodies;
	};
}