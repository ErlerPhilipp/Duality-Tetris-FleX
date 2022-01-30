class BallSandcastle : public Scene
{
public:

	BallSandcastle(const char* name) : Scene(name), mTimeSinceSpawn(0.0), mLastEmit(false) {}

	virtual void Initialize()
	{
		// granular pile
		float radius = 0.075f;

		Vec3 lower(8.0f, 4.0f, 2.0f);

		CreateParticleShape(GetFilePathByPlatform("../../data/sphere.ply").c_str(), lower, 1.0f, 0.0f, radius, 0.0f, 0.f, true, 1.0f, NvFlexMakePhase(1, 0), true, 0.00f);
		g_numSolidParticles = g_buffers->positions.size();

		CreateParticleShape(GetFilePathByPlatform("../../data/sandcastle.obj").c_str(), Vec3(-2.0f, -radius * 0.15f, 0.0f), 4.0f, 0.0f, radius * 1.0001f, 0.0f, 1.0f, false, 0.0f, NvFlexMakePhase(0, eNvFlexPhaseSelfCollide), false, 0.00f);

		g_numSubsteps = 2;

		g_params.radius = radius;
		g_params.staticFriction = 1.0f;
		g_params.dynamicFriction = 0.5f;
		g_params.viscosity = 0.0f;
		g_params.numIterations = 12;
		g_params.particleCollisionMargin = g_params.radius * 0.25f;	// 5% collision margin
		g_params.sleepThreshold = g_params.radius * 0.25f;
		g_params.shockPropagation = 0.5f;
		g_params.restitution = 0.2f;
		g_params.relaxationFactor = 1.f;
		g_params.damping = 0.94f;

		// draw options
		g_drawPoints = true;
		g_warmup = false;

		// hack, change the color of phase 0 particles to 'sand'		
		g_colors[0] = Colour(0.805f, 0.702f, 0.401f);

		// bottom plate
		AddBox(Vec3(3.0f, 0.1f, 3.0f), Vec3(0.0f));

		g_emitters[0].mEnabled = false;

		mLastEmit = g_emit;
	}

	virtual void PostInitialize()
	{
		(Vec4&)g_params.planes[0] = Vec4(0.0f, 1.0f, 0.0f, 5.0f); // bottom
		g_params.numPlanes = 1;
	}

	virtual void Update()
	{
		//this->UpdateTrigger();

		if (g_emit != mLastEmit)
		{
			float spin = DegToRad(0.0f);  // 15.0f default

			const Vec3 forward(-sinf(g_camAngle.x + spin) * cosf(g_camAngle.y), sinf(g_camAngle.y), -cosf(g_camAngle.x + spin) * cosf(g_camAngle.y));
			//const Vec3 right(Normalize(Cross(forward, Vec3(0.0f, 1.0f, 0.0f))));

			//g_emitters[0].mDir = Normalize(forward + Vec3(0.0, 0.4f, 0.0f));
			//g_emitters[0].mRight = right;
			//g_emitters[0].mPos = g_camPos + forward * 1.f + Vec3(0.0f, 0.2f, 0.0f) + right * 0.65f;

			for (int i = 0; i < g_numSolidParticles; ++i)
			{
				g_buffers->positions[i] = Vec4(g_camPos + forward * 1.f, 0.9f);  // last is inv_mass
				// g_buffers->positions[i].w = 0.9f;
				g_buffers->velocities[i] = forward * 10.f;
			}

			mLastEmit = g_emit;
		}
	}

	virtual void Sync()
	{
		//NvFlexSetRigids(g_solver, g_buffers->rigidOffsets.buffer, g_buffers->rigidIndices.buffer, g_buffers->rigidLocalPositions.buffer, g_buffers->rigidLocalNormals.buffer, g_buffers->rigidCoefficients.buffer, g_buffers->rigidPlasticThresholds.buffer, g_buffers->rigidPlasticCreeps.buffer, g_buffers->rigidRotations.buffer, g_buffers->rigidTranslations.buffer, g_buffers->rigidOffsets.size() - 1, g_buffers->rigidIndices.size());
	}

	bool mViscous;
	float mTimeSinceSpawn;
	float mSpacing;
	Mesh* mSpawnObject;
	bool mLastEmit;
};
