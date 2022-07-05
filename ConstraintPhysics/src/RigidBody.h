#pragma once

#include "../../Math/src/Vec3.h"
#include "../../Math/src/Quaternion.h"
#include "../../Math/src/Mat3.h"
#include "ConvexPoly.h"
#include "AABB.h"
#include <Vector>

namespace phyz {

	class RigidBody {
	public:
		typedef int PKey;

		struct GaussArc {
			unsigned int v1_indx;
			unsigned int v2_indx;
		};

		struct GaussMap {
			std::vector<mthz::Vec3> face_verts;
			std::vector<GaussArc> arcs;
		};

		GaussMap computeGaussMap(const ConvexPoly& c);

		RigidBody(const std::vector<ConvexPoly>& geometry, double density, int id);
		PKey track_point(mthz::Vec3 p); //track the movement of p which is on the body b. P given in world coordinates
		mthz::Vec3 getTrackedP(PKey pk);
		mthz::Vec3 getVelOfPoint(mthz::Vec3 p) const;
		void applyImpulse(mthz::Vec3 impulse, mthz::Vec3 position);
		void applyGyroAccel(float fElapsedTime, int n_itr = 1);
		void updateGeometry();

		static AABB genAABB(const std::vector<ConvexPoly>& geometry);

		int id;
		mthz::Quaternion orientation;
		mthz::Vec3 com;
		mthz::Vec3 vel;
		mthz::Vec3 ang_vel;

		mthz::Mat3 invTensor;
		mthz::Mat3 tensor;
		double mass;

		AABB aabb;
		double radius;
		bool fixed;

		friend class PhysicsEngine;
	private:

		std::vector<ConvexPoly> geometry;
		std::vector<ConvexPoly> reference_geometry;
		std::vector<GaussMap> gauss_maps;
		std::vector<GaussMap> reference_gauss_maps;
		
		std::vector<mthz::Vec3> track_p;
	};

}