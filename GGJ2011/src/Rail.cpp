#include "Rail.hpp"

#include "GameApp.hpp"

Rail::Rail() {
	mLastPointSet = 0;

}

void Rail::Initialize(World& world) {
	InitializePhysics();
	world.GetDynamicsWorld()->addConstraint(mConstraint.get());
}

void Rail::InitializePhysics() {
	btTransform tr;
	tr.setIdentity();
	tr.setOrigin(btVector3(GetPointFromFloat(mStartPosition).x, 0, GetPointFromFloat(mStartPosition).y));

	btScalar mass(10.f);
	btVector3 local_inertia(0, 0, 0);
	mCollisionShape = boost::shared_ptr<btCollisionShape>(new btBoxShape(btVector3(1.f,1.f,1.f)));
	mCollisionShape->calculateLocalInertia(mass, local_inertia);

	mMotionState = boost::shared_ptr<btDefaultMotionState>(new btDefaultMotionState(tr));
	btRigidBody::btRigidBodyConstructionInfo rb_info(mass, mMotionState.get(), mCollisionShape.get(), local_inertia);

	mBody = boost::shared_ptr<btRigidBody>(new btRigidBody(rb_info));
	//mBody->setDamping(0.2f, 0.2f);
	mBody->setLinearFactor(btVector3(1,1,0));
	mBody->setAngularFactor(btVector3(0,0,1));
	mBody->setUserPointer(this);
	mBody->setActivationState(DISABLE_DEACTIVATION);


	btTransform frameB;
	frameB.setIdentity();
	frameB.setRotation(btQuaternion(0,0,(mPoint2 - mPoint1).Rotation() * 180 / PI, 90));
	mConstraint = boost::shared_ptr<btGeneric6DofConstraint>(new btGeneric6DofConstraint( *mBody, frameB, true ));

	mConstraint->setAngularLowerLimit(btVector3(0,0,0));
	mConstraint->setAngularUpperLimit(btVector3(0,0,0));
	float l = Coordinates::ScreenPixelToWorldFloat(mPoint2 - mPoint1).Magnitude();
	mConstraint->setLinearLowerLimit(btVector3(-l/2.f,0,0));
	mConstraint->setLinearUpperLimit(btVector3(l/2.f,0,0));

	mConstraint->getTranslationalLimitMotor()->m_enableMotor[0] = true;
	mConstraint->getTranslationalLimitMotor()->m_targetVelocity[0] = 5.0f;
	mConstraint->getTranslationalLimitMotor()->m_maxMotorForce[0] = 0.1f;

	mConstraint->setDbgDrawSize(btScalar(1.f));
}

void Rail::Update(float time_delta) {
	mTiledSprite.SetImage(GameApp::get_mutable_instance().GetResourceManagerPtr()->GetImage("rail"));
	Vector2D diff = mPoint2 - mPoint1;
	mTiledSprite.SetPosition(mPoint1.x, mPoint1.y);
	mTiledSprite.SetScale(diff.Magnitude(), 20);
	mTiledSprite.SetRotation(- diff.Rotation() / PI * 180.f);
}

void Rail::Draw(sf::RenderTarget* target, sf::Shader& shader, bool editor_mode) const {
	if(IsFinished()) {
		// debug: line
		target->Draw(mTiledSprite);
	} else {
		// draw p1
		sf::Shape shape = sf::Shape::Circle(mPoint1.x, mPoint1.y, 3.f, sf::Color(255,255,255));
		target->Draw(shape);
	}
}

void Rail::SetNextPoint(Vector2D point) {
	if(mLastPointSet == 0) {
		mPoint1 = point;
	} else if(mLastPointSet == 1) {
		mPoint2 = point;
	}
	++mLastPointSet;
}

void Rail::Save(boost::property_tree::ptree* pt, int id) {
	std::string sid = boost::lexical_cast<std::string>(id);
	pt->add("rails."+sid+".1.x", mPoint1.x);
	pt->add("rails."+sid+".1.y", mPoint1.y);
	pt->add("rails."+sid+".2.x", mPoint2.x);
	pt->add("rails."+sid+".2.y", mPoint2.y);
}

void Rail::Load(boost::property_tree::ptree* pt, int id) {
	std::string sid = boost::lexical_cast<std::string>(id);
	mPoint1.x = pt->get<float>("rails."+sid+".1.x");
	mPoint1.y = pt->get<float>("rails."+sid+".1.y");
	mPoint2.x = pt->get<float>("rails."+sid+".2.x");
	mPoint2.y = pt->get<float>("rails."+sid+".2.y");
	mLastPointSet = 2;

}

Vector2D Rail::GetCenter() {
	return (mPoint1 + mPoint2) / 2;
}

bool Rail::IsFinished() const {
	return mLastPointSet >= 2;
}

float Rail::ClosestPositionOnLine(Vector2D pixel_pos) {
	// don't ask me, ask him ;)
	// http://www.codeguru.com/forum/showpost.php?p=538288&postcount=2
	double cx = pixel_pos.x;
	double cy = pixel_pos.y;
	double ax = mPoint1.x;
	double ay = mPoint1.y;
	double bx = mPoint2.x;
	double by = mPoint2.y;
	double distanceSegment, distanceLine;


	double r_numerator = (cx-ax)*(bx-ax) + (cy-ay)*(by-ay);
	double r_denomenator = (bx-ax)*(bx-ax) + (by-ay)*(by-ay);
	double r = r_numerator / r_denomenator;
	double px = ax + r*(bx-ax);
	double py = ay + r*(by-ay);
	double s =  ((ay-cy)*(bx-ax)-(ax-cx)*(by-ay) ) / r_denomenator;
	distanceLine = fabs(s)*sqrt(r_denomenator);

	//
	// (xx,yy) is the point on the lineSegment closest to (cx,cy)
	//
	double xx = px;
	double yy = py;

	if(r > 1) r = 1;
	if(r < 0) r = 0;
	return r;

	if ((r >= 0) && (r <= 1)) {
		distanceSegment = distanceLine;
	} else {
		double dist1 = (cx-ax)*(cx-ax) + (cy-ay)*(cy-ay);
		double dist2 = (cx-bx)*(cx-bx) + (cy-by)*(cy-by);
		if (dist1 < dist2) {
			xx = ax;
			yy = ay;
			distanceSegment = sqrt(dist1);
		} else {
			xx = bx;
			yy = by;
			distanceSegment = sqrt(dist2);
		}
	}

	return distanceSegment;
}


Vector2D Rail::GetPointFromFloat(float f) const {
	return mPoint1 + (mPoint2-mPoint1) * f;
}