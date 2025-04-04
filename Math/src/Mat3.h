#pragma once

#include "Vec3.h"
#include "NMat.h"

namespace mthz {

	class Mat3 {
	public:
		double v[3][3] = { {0, 0, 0}, {0, 0, 0}, {0, 0, 0} };

		Mat3() {}
		Mat3(const Mat3& m) {
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					v[i][j] = m.v[i][j];
				}
			}
		}

		NMat<3, 3> toNMat() {
			NMat<3, 3> out;
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					out.v[i][j] = v[i][j];
				}
			}
			return out;
		}

		static Mat3 zero() {
			return Mat3();
		}

		static Mat3 iden() {
			Mat3 out;
			out.v[0][0] = 1;
			out.v[1][1] = 1;
			out.v[2][2] = 1;
			return out;
		}

		static Mat3 cross_mat(Vec3 v) {
			Mat3 out;
			out.v[0][0] =    0; out.v[0][1] = -v.z; out.v[0][2] =  v.y;
			out.v[1][0] =  v.z; out.v[1][1] =    0; out.v[1][2] = -v.x;
			out.v[2][0] = -v.y; out.v[2][1] =  v.x; out.v[2][2] =    0;
			return out;
		}

		double det() const {
			double a = v[0][0], b = v[0][1], c = v[0][2], d = v[1][0], e = v[1][1], f = v[1][2], g = v[2][0], h = v[2][1], i = v[2][2];
			return a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g);
		}

		//https://ardoris.wordpress.com/2008/07/18/general-formula-for-the-inverse-of-a-3x3-matrix/
		Mat3 inverse() const {
			Mat3 out;
			double a = v[0][0], b = v[0][1], c = v[0][2], d = v[1][0], e = v[1][1], f = v[1][2], g = v[2][0], h = v[2][1], i = v[2][2];
			double det = a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g);

			out.v[0][0] = (e * i - f * h); out.v[0][1] = (c * h - b * i); out.v[0][2] = (b * f - c * e);
			out.v[1][0] = (f * g - d * i); out.v[1][1] = (a * i - c * g); out.v[1][2] = (c * d - a * f);
			out.v[2][0] = (d * h - e * g); out.v[2][1] = (b * g - a * h); out.v[2][2] = (a * e - b * d);

			out /= det;
			return out;
		}

		Vec3 operator*(Vec3 w) const {
			return Vec3(v[0][0] * w.x + v[0][1] * w.y + v[0][2] * w.z,
				v[1][0] * w.x + v[1][1] * w.y + v[1][2] * w.z,
				v[2][0] * w.x + v[2][1] * w.y + v[2][2] * w.z);
		}

		void operator+=(const Mat3& m) {
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					v[i][j] += m.v[i][j];
				}
			}
		}

		Mat3 operator-() const {
			Mat3 out;
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					out.v[i][j] = -v[i][j];
				}
			}
			return out;
		}

		Mat3 operator*(const Mat3& m) const {
			Mat3 out;
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					for (int k = 0; k < 3; k++) {
						out.v[i][j] += v[i][k] * m.v[k][j];
					}
				}
			}
			return out;
		}

		void operator*=(const Mat3& m) {
			double out[3][3] = { {0, 0, 0}, {0, 0, 0}, {0, 0, 0} };
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					for (int k = 0; k < 3; k++) {
						out[i][j] += v[i][k] * m.v[k][j];
					}
				}
			}

			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					v[i][j] = out[i][j];
				}
			}
		}

		void operator*=(const double d) {
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					v[i][j] *= d;
				}
			}
		}

		void operator/=(const double d) {
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					v[i][j] /= d;
				}
			}
		}

	};

	Mat3 operator*(const double d, const Mat3& m);
	Mat3 operator*(const Mat3& m, const double d);
	Mat3 operator/(const Mat3& m, const double d);
	Mat3 operator+(const Mat3& m1, const Mat3& m2);
	Mat3 operator-(const Mat3& m1, const Mat3& m2);

}