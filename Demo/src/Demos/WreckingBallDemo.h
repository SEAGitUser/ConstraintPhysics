#pragma once
#include "DemoScene.h"
#include "../Mesh.h"
#include "../../../ConstraintPhysics/src/PhysicsEngine.h"

class WreckingBallDemo : public DemoScene {
public:
	WreckingBallDemo(DemoManager* manager, DemoProperties properties) : DemoScene(manager, properties) {}

	~WreckingBallDemo() override {

	}

	std::vector<ControlDescription> controls() override {
		return {
			ControlDescription{"W, A, S, D", "Move the camera around when in free-look"},
			ControlDescription{"UP, DOWN, LEFT, RIGHT", "Rotate the camera"},
			ControlDescription{"I. K", "Raise, Lower crane arm"},
			ControlDescription{"J, L", "Rotate crane counter-clockwise, clockwise"},
			ControlDescription{"R", "Reset tower"},
			ControlDescription{"ESC", "Return to main menu"},
		};
	}

	void run() override {

		rndr::init(properties.window_width, properties.window_height, "Wrecking Ball Demo");
		if (properties.n_threads != 0) {
			phyz::PhysicsEngine::enableMultithreading(properties.n_threads);
		}

		phyz::PhysicsEngine p;
		p.setSleepingEnabled(false);
		p.setPGSIterations(45, 35);

		bool lock_cam = true;

		std::vector<PhysBod> bodies;
		std::vector<phyz::ConstraintID> constraints;

		//************************
		//*******BASE PLATE*******
		//************************
		double s = 100;
		phyz::Geometry geom2 = phyz::Geometry::box(mthz::Vec3(-s / 2, -2, -s / 2), s, 2, s);
		Mesh m2 = fromGeometry(geom2);
		phyz::RigidBody* r2 = p.createRigidBody(geom2, true);
		phyz::RigidBody::PKey draw_p = r2->trackPoint(mthz::Vec3(0, -2, 0));
		bodies.push_back({ m2, r2 });

		//*********************
		//****WRECKING BALL****
		//*********************
		mthz::Vec3 crane_tower_pos(0, 0, 4);
		double crane_tower_height = 4;
		double crane_tower_width = 2;
		double leg_width = 0.33;

		phyz::Geometry leg1 = phyz::Geometry::box(crane_tower_pos + mthz::Vec3(-crane_tower_width / 2.0, 0, -crane_tower_width / 2.0), leg_width, crane_tower_height, leg_width);
		phyz::Geometry leg2 = phyz::Geometry::box(crane_tower_pos + mthz::Vec3(crane_tower_width / 2.0 - leg_width, 0, -crane_tower_width / 2.0), leg_width, crane_tower_height, leg_width);
		phyz::Geometry leg3 = phyz::Geometry::box(crane_tower_pos + mthz::Vec3(crane_tower_width / 2.0 - leg_width, 0, crane_tower_width / 2.0 - leg_width), leg_width, crane_tower_height, leg_width);
		phyz::Geometry leg4 = phyz::Geometry::box(crane_tower_pos + mthz::Vec3(-crane_tower_width / 2.0, 0, crane_tower_width / 2.0 - leg_width), leg_width, crane_tower_height, leg_width);
		phyz::Geometry tower_base = phyz::Geometry::box(crane_tower_pos + mthz::Vec3(-crane_tower_width / 2.0, crane_tower_height, -crane_tower_width / 2.0), crane_tower_width, leg_width, crane_tower_width);
		double cabin_height = crane_tower_width * 0.67;
		phyz::Geometry cabin = phyz::Geometry::box(crane_tower_pos + mthz::Vec3(-crane_tower_width, crane_tower_height + leg_width, -crane_tower_width / 2.0), crane_tower_width * 1.5, cabin_height, crane_tower_width, phyz::Material::modified_density(2));

		double crane_length = crane_tower_height;
		double crane_height = crane_length / 8.0;
		double crane_width = crane_height / 3.0;
		mthz::Vec3 crane_pos = crane_tower_pos + mthz::Vec3(crane_tower_width/2.0, crane_tower_height + leg_width + cabin_height/3.0, 0);
		phyz::Geometry crane = phyz::Geometry::box(crane_pos + mthz::Vec3(-crane_height / 2.0, -crane_height / 2.0, -crane_width/2.0), crane_length + crane_height / 2.0, crane_height, crane_width, phyz::Material::modified_density(2));
		
		bodies.push_back({ fromGeometry(leg1), p.createRigidBody(leg1, true) });
		bodies.push_back({ fromGeometry(leg2), p.createRigidBody(leg2, true) });
		bodies.push_back({ fromGeometry(leg3), p.createRigidBody(leg3, true) });
		bodies.push_back({ fromGeometry(leg4), p.createRigidBody(leg4, true) });

		phyz::RigidBody* base_r = p.createRigidBody(tower_base, true);
		phyz::RigidBody* cabin_r = p.createRigidBody(cabin);
		phyz::RigidBody* crane_r = p.createRigidBody(crane);
		bodies.push_back({ fromGeometry(tower_base), base_r });
		bodies.push_back({ fromGeometry(cabin), cabin_r });
		bodies.push_back({ fromGeometry(crane), crane_r });
		
		double chain_width = crane_width;
		double chain_height = chain_width * 4;
		phyz::Geometry chain = phyz::Geometry::box(mthz::Vec3(-chain_width / 2.0, 0, -chain_width / 2.0), chain_width, -chain_height, chain_width, phyz::Material::modified_density(2));
		int n_chain = 6;
		mthz::Vec3 chain_start_pos = crane_pos + mthz::Vec3(crane_length, -crane_height / 2.0, 0);
		phyz::RigidBody* previous_chain = nullptr;

		for (int i = 0; i < n_chain; i++) {
			mthz::Vec3 this_chain_pos = chain_start_pos - mthz::Vec3(0, i * chain_height, 0);
			phyz::Geometry this_chain = chain.getTranslated(this_chain_pos);
			phyz::RigidBody* this_chain_r = p.createRigidBody(this_chain);

			if (i == 0) {
				p.addBallSocketConstraint(crane_r, this_chain_r, this_chain_pos);
			}
			else {
				p.addBallSocketConstraint(previous_chain, this_chain_r, this_chain_pos);
			}

			bodies.push_back({ fromGeometry(this_chain), this_chain_r });
			previous_chain = this_chain_r;
		}

		mthz::Vec3 final_chain_pos = chain_start_pos + mthz::Vec3(0, -n_chain * chain_height, 0);
		double ball_size = 0.75;
		phyz::Geometry ball = phyz::Geometry::box(final_chain_pos + mthz::Vec3(-ball_size/2.0, -ball_size, -ball_size/2.0), ball_size, ball_size, ball_size, phyz::Material::modified_density(2));
		phyz::RigidBody* ball_r = p.createRigidBody(ball);
		p.addBallSocketConstraint(ball_r, previous_chain, final_chain_pos);
		bodies.push_back({ fromGeometry(ball), ball_r });


		double rotate_torque = 0.2;
		phyz::ConstraintID rotate_motor = p.addHingeConstraint(base_r, cabin_r, crane_tower_pos + mthz::Vec3(0, crane_tower_height + crane_tower_width, 0), mthz::Vec3(0, 1, 0));
		double lift_torque = 60;
		phyz::ConstraintID lift_motor = p.addHingeConstraint(cabin_r, crane_r, crane_pos, mthz::Vec3(0, 0, 1));

		p.setMotor(rotate_motor, 0, rotate_torque);
		p.setMotor(lift_motor, 0, lift_torque);

		//*************
		//****TOWER****
		//*************
		mthz::Vec3 tower_pos(8, 0, 0);
		double tower_width = 4;
		double tower_story_height = 1.25;
		double floor_height = 0.25;
		double pillar_width = 0.3;
		int n_stories = 6;
		phyz::Geometry pillar = phyz::Geometry::box(mthz::Vec3(), pillar_width, tower_story_height, pillar_width);
		phyz::Geometry floor_plate = phyz::Geometry::box(mthz::Vec3(), tower_width / 2.0, floor_height, tower_width / 2.0);

		std::vector<phyz::RigidBody*> tower_bodies;

		for (int i = 0; i < n_stories; i++) {
			mthz::Vec3 story_pos = tower_pos + mthz::Vec3(0, i * (floor_height + tower_story_height), 0);
			phyz::Geometry pillar1 = pillar.getTranslated(story_pos + mthz::Vec3(-tower_width / 2.0, 0, - tower_width / 2.0));
			phyz::Geometry pillar2 = pillar.getTranslated(story_pos + mthz::Vec3(-pillar_width / 2.0, 0, -tower_width / 2.0));
			phyz::Geometry pillar3 = pillar.getTranslated(story_pos + mthz::Vec3(tower_width / 2.0 - pillar_width, 0, -tower_width / 2.0));
			phyz::Geometry pillar4 = pillar.getTranslated(story_pos + mthz::Vec3(-tower_width / 2.0, 0, -pillar_width / 2.0));
			phyz::Geometry pillar5 = pillar.getTranslated(story_pos + mthz::Vec3(-pillar_width / 2.0, 0, -pillar_width / 2.0));
			phyz::Geometry pillar6 = pillar.getTranslated(story_pos + mthz::Vec3(tower_width / 2.0 - pillar_width, 0, -pillar_width / 2.0));
			phyz::Geometry pillar7 = pillar.getTranslated(story_pos + mthz::Vec3(-tower_width / 2.0, 0, tower_width / 2.0 - pillar_width));
			phyz::Geometry pillar8 = pillar.getTranslated(story_pos + mthz::Vec3(-pillar_width / 2.0, 0, tower_width / 2.0 - pillar_width));
			phyz::Geometry pillar9 = pillar.getTranslated(story_pos + mthz::Vec3(tower_width / 2.0 - pillar_width, 0, tower_width / 2.0 - pillar_width));

			phyz::Geometry floor_plate1 = floor_plate.getTranslated(story_pos + mthz::Vec3(-tower_width / 2.0, tower_story_height, -tower_width / 2.0));
			phyz::Geometry floor_plate2 = floor_plate.getTranslated(story_pos + mthz::Vec3(0, tower_story_height, -tower_width / 2.0));
			phyz::Geometry floor_plate3 = floor_plate.getTranslated(story_pos + mthz::Vec3(-tower_width / 2.0, tower_story_height, 0));
			phyz::Geometry floor_plate4 = floor_plate.getTranslated(story_pos + mthz::Vec3(0, tower_story_height, 0));

			phyz::RigidBody* pillar1_r = p.createRigidBody(pillar1);
			phyz::RigidBody* pillar2_r = p.createRigidBody(pillar2);
			phyz::RigidBody* pillar3_r = p.createRigidBody(pillar3);
			phyz::RigidBody* pillar4_r = p.createRigidBody(pillar4);
			phyz::RigidBody* pillar5_r = p.createRigidBody(pillar5);
			phyz::RigidBody* pillar6_r = p.createRigidBody(pillar6);
			phyz::RigidBody* pillar7_r = p.createRigidBody(pillar7);
			phyz::RigidBody* pillar8_r = p.createRigidBody(pillar8);
			phyz::RigidBody* pillar9_r = p.createRigidBody(pillar9);

			phyz::RigidBody* floor_plate1_r = p.createRigidBody(floor_plate1);
			phyz::RigidBody* floor_plate2_r = p.createRigidBody(floor_plate2);
			phyz::RigidBody* floor_plate3_r = p.createRigidBody(floor_plate3);
			phyz::RigidBody* floor_plate4_r = p.createRigidBody(floor_plate4);

			std::vector<phyz::RigidBody*> new_bodies = {
				pillar1_r, pillar2_r, pillar3_r, pillar4_r, pillar5_r, pillar6_r, pillar7_r, pillar8_r, pillar9_r,
				floor_plate1_r, floor_plate2_r, floor_plate3_r, floor_plate4_r
			};

			tower_bodies.insert(tower_bodies.end(), new_bodies.begin(), new_bodies.end());

			bodies.push_back({ fromGeometry(pillar1), pillar1_r });
			bodies.push_back({ fromGeometry(pillar2), pillar2_r });
			bodies.push_back({ fromGeometry(pillar3), pillar3_r });
			bodies.push_back({ fromGeometry(pillar4), pillar4_r });
			bodies.push_back({ fromGeometry(pillar5), pillar5_r });
			bodies.push_back({ fromGeometry(pillar6), pillar6_r });
			bodies.push_back({ fromGeometry(pillar7), pillar7_r });
			bodies.push_back({ fromGeometry(pillar8), pillar8_r });
			bodies.push_back({ fromGeometry(pillar9), pillar9_r });

			bodies.push_back({ fromGeometry(floor_plate1), floor_plate1_r });
			bodies.push_back({ fromGeometry(floor_plate2), floor_plate2_r });
			bodies.push_back({ fromGeometry(floor_plate3), floor_plate3_r });
			bodies.push_back({ fromGeometry(floor_plate4), floor_plate4_r });
		}
		

		rndr::Shader shader("resources/shaders/Basic.shader");
		shader.bind();

		float t = 0;
		float fElapsedTime;

		mthz::Vec3 pos(0, 2 * crane_tower_height, 10);
		mthz::Quaternion orient;
		double mv_speed = 2;
		double rot_speed = 1;

		double phyz_time = 0;
		double timestep = 1 / 90.0;
		p.setStep_time(timestep);
		p.setGravity(mthz::Vec3(0, -4.9, 0));

		while (rndr::render_loop(&fElapsedTime)) {

			if (rndr::getKeyDown(GLFW_KEY_W)) {
				pos += orient.applyRotation(mthz::Vec3(0, 0, -1) * fElapsedTime * mv_speed);
			}
			else if (rndr::getKeyDown(GLFW_KEY_S)) {
				pos += orient.applyRotation(mthz::Vec3(0, 0, 1) * fElapsedTime * mv_speed);
			}
			if (rndr::getKeyDown(GLFW_KEY_A)) {
				pos += orient.applyRotation(mthz::Vec3(-1, 0, 0) * fElapsedTime * mv_speed);
			}
			else if (rndr::getKeyDown(GLFW_KEY_D)) {
				pos += orient.applyRotation(mthz::Vec3(1, 0, 0) * fElapsedTime * mv_speed);
			}

			if (rndr::getKeyDown(GLFW_KEY_UP)) {
				orient = orient * mthz::Quaternion(fElapsedTime * rot_speed, mthz::Vec3(1, 0, 0));
			}
			else if (rndr::getKeyDown(GLFW_KEY_DOWN)) {
				orient = orient * mthz::Quaternion(-fElapsedTime * rot_speed, mthz::Vec3(1, 0, 0));
			}
			if (rndr::getKeyDown(GLFW_KEY_LEFT)) {
				orient = mthz::Quaternion(fElapsedTime * rot_speed, mthz::Vec3(0, 1, 0)) * orient;
			}
			else if (rndr::getKeyDown(GLFW_KEY_RIGHT)) {
				orient = mthz::Quaternion(-fElapsedTime * rot_speed, mthz::Vec3(0, 1, 0)) * orient;
			}

			t += fElapsedTime;

			if (rndr::getKeyDown(GLFW_KEY_I)) {
				p.setMotor(lift_motor, -1, lift_torque);
			}
			else if (rndr::getKeyDown(GLFW_KEY_K)) {
				p.setMotor(lift_motor, 1, lift_torque);
			}
			else {
				p.setMotor(lift_motor, 0, lift_torque);
			}
			if (rndr::getKeyDown(GLFW_KEY_J)) {
				p.setMotor(rotate_motor, -3, rotate_torque);
			}
			else if (rndr::getKeyDown(GLFW_KEY_L)) {
				p.setMotor(rotate_motor, 3, rotate_torque);
			}
			else {
				p.setMotor(rotate_motor, 0, rotate_torque);
			}
			if (rndr::getKeyPressed(GLFW_KEY_R)) {
				for (phyz::RigidBody* r : tower_bodies) {
					r->setOrientation(mthz::Quaternion());
					r->setToPosition(mthz::Vec3());
					r->setAngVel(mthz::Vec3());
					r->setVel(mthz::Vec3());
				}
			}
			if (rndr::getKeyPressed(GLFW_KEY_ESCAPE)) {
				for (PhysBod b : bodies) {
					delete b.mesh.ib;
					delete b.mesh.va;
				}
				manager->deselectCurrentScene();
				return;
			}


			phyz_time += fElapsedTime;
			phyz_time = std::min<double>(phyz_time, 1.0 / 30.0);
			while (phyz_time > timestep) {
				phyz_time -= timestep;
				p.timeStep();
			}


			rndr::clear(rndr::color(0.0f, 0.0f, 0.0f));

			for (const PhysBod& b : bodies) {
				mthz::Vec3 cam_pos = pos;
				mthz::Quaternion cam_orient = orient;
				shader.setUniformMat4f("u_MVP", rndr::Mat4::proj(0.1, 50.0, 2.0, 2.0, 120.0) * rndr::Mat4::cam_view(cam_pos, cam_orient) * rndr::Mat4::model(b.r->getPos(), b.r->getOrientation()));
				shader.setUniform1i("u_Asleep", b.r->getAsleep());
				rndr::draw(*b.mesh.va, *b.mesh.ib, shader);

			}
		}
	}
};