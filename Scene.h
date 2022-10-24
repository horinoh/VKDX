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
		virtual ~Scene() {
			for (auto i : RigidBodies) {
				if (nullptr != i) {
					delete i;
				}
			}
		}

		virtual void Init() 
		{
			//!< ���I
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

			//!< �ÓI
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
						Rb->Init(new ShapeSphere(Radius));
					}
				}
			}
		}
		virtual void Update(const float DeltaSec)
		{
			//!< �d��
			for (auto i : RigidBodies) {
				i->ApplyGravity(DeltaSec);
			}

#pragma region �u���[�h�t�F�[�Y
			std::vector<std::pair<int, int>> PotentialPairs;
			{
				std::vector<BoundEdge> BoundEdges;
				{
					//!< ���Ɏˉe���� AABB �͈̔͂��v�Z
					const auto Axis = Vec3(1.0f, 1.0f, 1.0f).Normalize();

					const auto nRb = size(RigidBodies);
					BoundEdges.reserve(nRb * 2);
					for (auto i = 0; i < nRb; ++i) {
						const auto Rb = RigidBodies[i];

						auto aabb = Rb->Shape->GetAABB(Rb->Position, Rb->Rotation);
						//!< ���x�� AABB ���g������
						aabb.Expand(aabb.Min + Rb->LinearVelocity * DeltaSec);
						aabb.Expand(aabb.Max + Rb->LinearVelocity * DeltaSec);
						aabb.Expand(aabb.Min - Vec3::Epsilon());
						aabb.Expand(aabb.Max + Vec3::Epsilon());

						BoundEdges.emplace_back(BoundEdge({ i, Axis.Dot(aabb.Min), true }));
						BoundEdges.emplace_back(BoundEdge({ i, Axis.Dot(aabb.Max), false }));
					}
					std::ranges::sort(BoundEdges, std::ranges::less{}, &BoundEdge::Value);
				}

				//!< �ˉe AABB ����A���ݓI�Փ˃y�A���X�g���\�z
				const auto nBe = size(BoundEdges);
				for (auto i = 0; i < nBe; ++i) {
					const auto& A = BoundEdges[i];
					//!< Min �Ȃ� Max ��������܂Ő��ݓI�Փˑ����T��
					if (A.isMin) {
						for (auto j = i + 1; j < nBe; ++j) {
							const auto& B = BoundEdges[j];
							//!< Max ��������ΏI��
							if (A.Index == B.Index) {
								break;
							}
							//!< ���ݓI�Փˑ���
							if (B.isMin) {
								PotentialPairs.emplace_back(std::pair<int, int>({ A.Index, B.Index }));
							}
						}
					}
				}
#ifdef _DEBUG
				//if (!empty(PotentialPairs)) { std::cout << "Potential collision = " << size(PotentialPairs) << std::endl; }
#endif
			}
#pragma endregion

#pragma region �i���[�t�F�[�Y	
			std::vector<Contact> Contacts;
			{
				//!< �Փˎ��W
				const auto nRb = size(RigidBodies);
				Contacts.reserve(nRb * nRb);
#if false
				for (auto i : PotentialPairs) {
					const auto RbA = RigidBodies[i.first];
					const auto RbB = RigidBodies[i.second];

					if (0.0f != RbA->InvMass || 0.0f != RbB->InvMass) {
						Contact Ct;
						if (Intersect(RbA, RbB, DeltaSec, Ct)) {
							Contacts.emplace_back(Ct);
						}
					}
				}
#else
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
#endif
				//!< TOI �Ń\�[�g
				std::ranges::sort(Contacts, std::ranges::less{}, &Contact::TimeOfImpact);
#ifdef _DEBUG
				//if (!empty(Contacts)) { std::cout << "Contact detected = " << size(Contacts) << std::endl; }
#endif
			}
#pragma endregion

			//!< TOI���Ɏ��Ԃ��X���C�X���āA�V�~�����[�V������i�߂� (�͐ς̓K�p)
			auto AccumTime = 0.0f;
			for (auto i : Contacts) {
				const auto dt = i.TimeOfImpact - AccumTime;

				for (auto j : RigidBodies) {
					j->Update(dt);
				}

				Resolve(i);
				AccumTime += dt;
			}
			//!< �����Փ˂͖����̂ŁA�c��̃V�~�����[�V������i�߂� (�͐ς̓K�p)
			const auto RemainTime = DeltaSec - AccumTime;
			if (0.0f < RemainTime) {
				for (auto i : RigidBodies) {
					i->Update(RemainTime);
				}
			}
		}

		//!< Brute force
		//virtual void Update(const float DeltaSec)
		//{
		//	//!< �d��
		//	for (auto i : RigidBodies) {
		//		i->ApplyGravity(DeltaSec);
		//	}
		//	const auto nRb = size(RigidBodies);
		//	for (auto i = 0; i < nRb; ++i) {
		//		for (auto j = i + 1; j < nRb; ++j) {
		//			const auto RbA = RigidBodies[i];
		//			const auto RbB = RigidBodies[j];
		//			if (0.0f != RbA->InvMass || 0.0f != RbB->InvMass) {
		//				Contact Ct;
		//				if (Intersect(RbA, RbB, DeltaSec, Ct)) {
		//					Resolve(Ct);
		//				}
		//			}
		//		}
		//	}
		//	for (auto i : RigidBodies) {
		//		i->Update(DeltaSec);
		//	}
		//}

		std::vector<RigidBody *> RigidBodies;
	};
}