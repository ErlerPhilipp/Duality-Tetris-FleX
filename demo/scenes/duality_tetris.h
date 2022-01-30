class DualityTetris : public Scene
{
public:

	DualityTetris(const char* name) : Scene(name), mNumStaticParticles(0), mFrameCount(0) {}

	virtual void Initialize()
	{
		// bottom plate
		AddBox(Vec3(3.0f, 0.1f, 3.0f), Vec3(-0.1f));

		// trigger box
		//AddBox(Vec3(20.0f, 10.0f, 20.0f), Vec3(0.0f, -20.0, 0.0));
		//g_buffers->shapeFlags[1] |= eNvFlexShapeFlagTrigger;

		int sx = 2;
		int sy = 2;
		int sz = 2;

		Vec3 lower(0.0f, 1.5f + g_params.radius * 0.25f, 0.0f);

		int dimx = 10;
		int dimy = 15;
		int dimz = 10;

		//float radius = g_params.radius;
		float radius = 0.1f;
		g_params.radius = radius;
		int group = 1;

		Mesh* mesh = ImportMesh(GetFilePathByPlatform("../../data/box.ply").c_str());

		mNumStaticParticles = g_buffers->positions.size();

		// create a basic grid
		for (int y = 0; y < dimy; ++y)
			for (int z = 0; z < dimz; ++z)
				for (int x = 0; x < dimx; ++x)
					CreateParticleShape(
						GetFilePathByPlatform("../../data/box.ply").c_str(),
						(Vec3(float(x) / float(dimx), float(y) / float(dimy) + 0.5f + float(y) * radius * 0.25f, float(z) / float(dimz)) - 0.5f) * 3.0f,
						Vec3(5.0f) * radius * 0.5f,
						0.0f,
						radius * 0.5f,
						Vec3(0.0f),
						(float(y) / float(dimy) + 0.1f) / 3.0f,
						true,
						1.0f,
						NvFlexMakePhase(group++, 0),
						true,
						0.0f);
		delete mesh;

		// separte solid particle count
		g_numSolidParticles = g_buffers->positions.size();

		// number of fluid particles to allocate
		g_numExtraParticles = 64 * 1024;
		//g_numExtraParticles = 6 * 1024;  // debug

		g_params.radius = radius;
		g_params.dynamicFriction = 0.5f;
		g_params.viscosity = 0.1f;
		//g_params.numIterations = 3;
		g_params.numIterations = 8;
		g_params.vorticityConfinement = 0.0f;
		g_params.fluidRestDistance = g_params.radius * 0.55f;
		g_params.solidRestDistance = g_params.radius * 0.5f;
		g_warmup = true;

		g_emitters[0].mEnabled = true;
		g_emitters[0].mSpeed = 2.0f * (g_params.fluidRestDistance) / g_dt;

		// draw options
		g_drawPoints = false;
		g_drawEllipsoids = true;
		g_tweakPanel = false;
		g_scenePanel = false;

		g_numFrameWon = 0;
		mFrameCount = 0;
		g_started = false;
	}

	virtual void PostInitialize()
	{
		(Vec4&)g_params.planes[0] = Vec4(0.0f, 1.0f, 0.0f, 5.0f); // bottom
		//(Vec4&)g_params.planes[1] = Vec4(0.0f, 0.0f, 1.0f, 1.0f); // back
		//(Vec4&)g_params.planes[2] = Vec4(1.0f, 0.0f, 0.0f, 3.0f); // left
		//(Vec4&)g_params.planes[3] = Vec4(-1.0f, 0.0f, 0.0f, 3.0f); // right
		//(Vec4&)g_params.planes[4] = Vec4(0.0f, 0.0f, -1.0f, 10.0f); // 
		//(Vec4&)g_params.planes[5] = Vec4(0.0f, -1.0f, 0.0f, 1.0f); // top
		g_params.numPlanes = 1;
	}

	virtual void UpdateTrigger()
	{
		const int maxContactsPerParticle = 6;

		NvFlexVector<Vec4> contactPlanes(g_flexLib, g_buffers->positions.size() * maxContactsPerParticle);
		NvFlexVector<Vec4> contactVelocities(g_flexLib, g_buffers->positions.size() * maxContactsPerParticle);
		NvFlexVector<int> contactIndices(g_flexLib, g_buffers->positions.size());
		NvFlexVector<unsigned int> contactCounts(g_flexLib, g_buffers->positions.size());

		NvFlexGetContacts(g_solver, contactPlanes.buffer, contactVelocities.buffer, contactIndices.buffer, contactCounts.buffer);

		contactPlanes.map();
		contactVelocities.map();
		contactIndices.map();
		contactCounts.map();

		int activeCount = NvFlexGetActiveCount(g_solver);
		
		for (int i = 0; i < activeCount; ++i)
		{
			const int contactIndex = contactIndices[i];
			const unsigned int count = contactCounts[contactIndex];
		
			for (unsigned int c = 0; c < count; ++c)
			{
				Vec4 velocity = contactVelocities[contactIndex * maxContactsPerParticle + c];
		
				const int shapeId = int(velocity.w);
		
				// detect when particle intersects the trigger 
				// volume and teleport it over to the other box
				if (shapeId == 1)
				{
					this->addUnusedParticle(i);
				}
			}
		}
	}

	void addUnusedParticle(unsigned int i)
	{
		auto result = g_unusedParticlesUnique.insert(i);
		if (result.second)  // inserted
			g_unusedParticles.push(i);
	}

	virtual void Update()
	{
		// this->UpdateTrigger();
		int activeCount = NvFlexGetActiveCount(g_solver);

		for (unsigned int i = g_numSolidParticles; i < activeCount; ++i)
		{
			Vec4 part_pos = g_buffers->positions[i];
			if (part_pos.y < -3.0)
				this->addUnusedParticle(i);
		}

		bool anyBoxOnTable = false;
		for (unsigned int i = mNumStaticParticles; i < g_numSolidParticles; ++i)
		{
			Vec4 part_pos = g_buffers->positions[i];
			if (part_pos.y > 0.0)
			{
				anyBoxOnTable = true;
				break;
			}
		}
		if (g_numFrameWon == 0 && !anyBoxOnTable)
			g_numFrameWon = mFrameCount;
		++mFrameCount;

		if (g_emit)
			g_started = true;

		if (!g_started)
			g_frame = 0;
	}

	virtual void Sync()
	{
		//NvFlexSetRigids(g_solver, g_buffers->rigidOffsets.buffer, g_buffers->rigidIndices.buffer, g_buffers->rigidLocalPositions.buffer, g_buffers->rigidLocalNormals.buffer, g_buffers->rigidCoefficients.buffer, g_buffers->rigidPlasticThresholds.buffer, g_buffers->rigidPlasticCreeps.buffer, g_buffers->rigidRotations.buffer, g_buffers->rigidTranslations.buffer, g_buffers->rigidOffsets.size() - 1, g_buffers->rigidIndices.size());
	}

	unsigned int mNumStaticParticles;
	int mFrameCount;
};
