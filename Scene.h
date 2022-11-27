#pragma once

#include <vector>
#include <algorithm>

#include "Math.h"
using namespace Math;

#include "Shape.h"
#include "RigidBody.h"

#include "Colli.h"
using namespace Colli;

namespace Phys
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
				//!< 軸に射影した AABB の範囲を計算
				const auto Axis = Vec3(1.0f, 1.0f, 1.0f).Normalize();

				const auto nRb = size(RigidBodies);
				BoundEdges.reserve(nRb * 2);
				for (auto i = 0; i < nRb; ++i) {
					const auto Rb = RigidBodies[i];

					auto Aabb = Rb->Shape->GetAABB(Rb->Position, Rb->Rotation);
					//!< 速度分 AABB を拡張する
					Aabb.Expand(Aabb.Min + Rb->LinearVelocity * DeltaSec);
					Aabb.Expand(Aabb.Max + Rb->LinearVelocity * DeltaSec);
					//aabb.Expand(aabb.Min - Vec3::Epsilon());
					//aabb.Expand(aabb.Max + Vec3::Epsilon());
					//Aabb.Expand(Aabb.Min - Vec3::One() * 0.01f);
					//Aabb.Expand(Aabb.Max + Vec3::One() * 0.01f);

					BoundEdges.emplace_back(BoundEdge({ i, Axis.Dot(Aabb.Min), true }));
					BoundEdges.emplace_back(BoundEdge({ i, Axis.Dot(Aabb.Max), false }));
				}
				std::ranges::sort(BoundEdges, std::ranges::less{}, &BoundEdge::Value);
			}

			//!< 射影 AABB から、潜在的衝突ペアリストを構築
			const auto NumBounds = size(BoundEdges);
			for (auto i = 0; i < NumBounds; ++i) {
				const auto& A = BoundEdges[i];
				//!< Min なら Max が見つかるまで探す
				if (A.isMin) {
					for (auto j = i + 1; j < NumBounds; ++j) {
						const auto& B = BoundEdges[j];
						//!< 同じインデックス (==Max) が見つかれば終了
						if (A.Index == B.Index) {
							break;
						}
						//!< 潜在的衝突相手
						if (B.isMin) {
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
			for (auto i : CollidablePairs) {
				const auto RbA = RigidBodies[i.first];
				const auto RbB = RigidBodies[i.second];

				if (0.0f != RbA->InvMass || 0.0f != RbB->InvMass) {
					Contact Ct;
					if (Intersect(RbA, RbB, DeltaSec, Ct)) {
						Contacts.emplace_back(Ct);
					}
				}
			}
			//!< TOI でソート
			std::ranges::sort(Contacts, std::ranges::less{}, &Contact::TimeOfImpact);
		}
		virtual void BruteForce(std::vector<Contact>& Contacts, const float DeltaSec)
		{
			const auto nRb = size(RigidBodies);
			Contacts.reserve(nRb * nRb);
			Contacts.clear();
			for (auto i = 0; i < nRb; ++i) {
				for (auto j = i + 1; j < nRb; ++j) {
					const auto RbA = RigidBodies[i];
					const auto RbB = RigidBodies[j];

					if (0.0f != RbA->InvMass || 0.0f != RbB->InvMass) {
						Contact Ct;
						if (Intersect(RbA, RbB, DeltaSec, Ct)) {
							Contacts.emplace_back(Ct);
						}
					}
				}
			}
			//!< TOI でソート
			std::ranges::sort(Contacts, std::ranges::less{}, &Contact::TimeOfImpact);
		}
		virtual void Update(const float DeltaSec)
		{
			for (auto i : RigidBodies) {
				i->ApplyGravity(DeltaSec);
			}

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
				const auto Delta = i.TimeOfImpact - AccumTime;

				for (auto j : RigidBodies) {
					j->Update(Delta);
				}

				//!< 衝突時の力積の適用
				Resolve(i);

				AccumTime += Delta;
			}

			//!< もう衝突は無いので、残りのシミュレーションを進める
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