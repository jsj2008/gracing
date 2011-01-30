#include <assert.h>

#include "btBulletDynamicsCommon.h"

#include "gmlog.h"

#include "Track.hh"
#include "Util.hh"
#include "CameraDataManager.hh"

#ifndef BASE_DIR
#define BASE_DIR "."
#endif
#define MANIFEST_NAME "TRACK"

void suka()
{

	int i;

	///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	btCollisionDispatcher* dispatcher = new	btCollisionDispatcher(collisionConfiguration);

	///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
	btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();

	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;

	btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,overlappingPairCache,solver,collisionConfiguration);

	dynamicsWorld->setGravity(btVector3(0,-10,0));

	///create a few basic rigid bodies
	btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(50.),btScalar(50.),btScalar(50.)));

	//keep track of the shapes, we release memory at exit.
	//make sure to re-use collision shapes among rigid bodies whenever possible!
	btAlignedObjectArray<btCollisionShape*> collisionShapes;

	collisionShapes.push_back(groundShape);

	btTransform groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(btVector3(0,-56,0));

	{
		btScalar mass(0.);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0,0,0);
		if (isDynamic)
			groundShape->calculateLocalInertia(mass,localInertia);

		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,groundShape,localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		//add the body to the dynamics world
		dynamicsWorld->addRigidBody(body);
	}


	{
		//create a dynamic rigidbody

		//btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
		btCollisionShape* colShape = new btSphereShape(btScalar(1.));
		collisionShapes.push_back(colShape);

		/// Create Dynamic Objects
		btTransform startTransform;
		startTransform.setIdentity();

		btScalar	mass(1.f);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0,0,0);
		if (isDynamic)
			colShape->calculateLocalInertia(mass,localInertia);

			startTransform.setOrigin(btVector3(2,10,0));
		
			//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
			btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
			btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,colShape,localInertia);
			btRigidBody* body = new btRigidBody(rbInfo);

			dynamicsWorld->addRigidBody(body);
	}



/// Do some simulation



	for (i=0;i<100;i++)
	{
		dynamicsWorld->stepSimulation(1.f/60.f,10);
		
		//print positions of all objects
		for (int j=dynamicsWorld->getNumCollisionObjects()-1; j>=0 ;j--)
		{
			btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[j];
			btRigidBody* body = btRigidBody::upcast(obj);
			if (body && body->getMotionState())
			{
				btTransform trans;
				body->getMotionState()->getWorldTransform(trans);
				GM_LOG("world pos = %f,%f,%f\n",float(trans.getOrigin().getX()),float(trans.getOrigin().getY()),float(trans.getOrigin().getZ()));
			}
		}
	}


	//cleanup in the reverse order of creation/initialization

	//remove the rigidbodies from the dynamics world and delete them
	for (i=dynamicsWorld->getNumCollisionObjects()-1; i>=0 ;i--)
	{
		btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		dynamicsWorld->removeCollisionObject( obj );
		delete obj;
	}

	//delete collision shapes
	for (int j=0;j<collisionShapes.size();j++)
	{
		btCollisionShape* shape = collisionShapes[j];
		collisionShapes[j] = 0;
		delete shape;
	}

	//delete dynamics world
	delete dynamicsWorld;

	//delete solver
	delete solver;

	//delete broadphase
	delete overlappingPairCache;

	//delete dispatcher
	delete dispatcher;

	delete collisionConfiguration;

	//next line is optional: it will be cleared by the destructor when the array goes out of scope
	collisionShapes.clear();

}

Track::Track(
    irr::IrrlichtDevice * device,
    const char * filename
    )
{
  m_device=device;
  m_filesystem=device->getFileSystem();
  m_filename=strdup(filename);
  m_loaded=false;
}

void Track::load()
{

  suka();

  if(m_loaded) return;

  irr::u32 cnt=m_filesystem->getFileArchiveCount();
  bool res=m_filesystem->addFileArchive(m_filename);

  if(!res) {
    return ;
  }
  assert(res);

  m_archiveIndex=cnt;

  irr::io::IFileArchive* archive=m_filesystem->
    getFileArchive(m_archiveIndex);
  assert(archive);

  GM_LOG("here\n");
  
  const irr::io::IFileList * fileList=archive->getFileList();
  irr::s32 manifestIndex;

  manifestIndex=fileList->findFile(MANIFEST_NAME);
  
  if(manifestIndex<0) {
    GM_LOG("Not a valid track file\n");
    return;
  }

  irr::io::IReadFile *  manifestFile=
    archive->createAndOpenFile(manifestIndex);

  assert(manifestFile);

  irr::io::IXMLReaderUTF8 * xmlReader=
    m_filesystem->createXMLReaderUTF8 (manifestFile);

  enum { MAX_DEPTH=128 };

  int i,level,ot;
  int nodeStack[MAX_DEPTH];

  int nodeStackPtr;
  irr::scene::ISceneManager* smgr = m_device->getSceneManager();

  irr::io::EXML_NODE nodeType;
  bool inElement;
  enum {
    ot_none,
    ot_mesh,
    ot_camera,
    ot_lamp,
    ot_unknown
  };
  irr::io::IReadFile * rfile;
  for(int i=0, inElement=false, nodeStackPtr=0; 
      xmlReader->read(); 
      i++)
  {
    nodeType=xmlReader->getNodeType();
    switch(nodeType) {
      case irr::io::EXN_NONE:
        GM_LOG("%d None '%s'\n",i,
            xmlReader->getNodeName(),
            xmlReader->getNodeType());
      break;
      case irr::io::EXN_ELEMENT:
        if(strcmp("mesh",xmlReader->getNodeName())==0) {
          ot=ot_mesh;
        } else if(strcmp("camera",xmlReader->getNodeName())==0) {
          ot=ot_camera;
        } else if(strcmp("lamp",xmlReader->getNodeName())==0) {
          ot=ot_lamp;
        } else {
          ot=ot_unknown;
        }
        nodeStack[++nodeStackPtr]=ot;
        inElement=true;
      break;
      case irr::io::EXN_ELEMENT_END:
        inElement=false;
        nodeStackPtr --;
      break;
      case irr::io::EXN_TEXT:
        if(inElement) {
          switch(nodeStack[nodeStackPtr]) {
            case ot_lamp:
              GM_LOG("Loading lights: %s\n",xmlReader->getNodeName());
              rfile=archive->
                  createAndOpenFile (xmlReader->getNodeName());
              if(rfile) {
                loadLights(rfile,smgr);
                rfile->drop();
              } else {
                GM_LOG("  --> cannot find file\n");
              }
              break;
            case ot_mesh:
              GM_LOG("Loading mesh: %s\n",xmlReader->getNodeName());
              rfile=archive->
                  createAndOpenFile (xmlReader->getNodeName());
              if(rfile) {
                irr::scene::IAnimatedMesh* mesh = smgr->getMesh(rfile);
                irr::scene::IAnimatedMeshSceneNode* node=0;
                node=smgr->addAnimatedMeshSceneNode( mesh );
                if(!node) {
                  GM_LOG("cannot load mesh\n");
                } else {
                  GM_LOG(" -> mesh at %X\n",node);
                  m_sceneNodes.push_back(node);
                  node->grab();
                }
                rfile->drop();
              } else {
                GM_LOG("  cannot find file\n");
              }
              break;
            case ot_camera:
              GM_LOG("Loading camera data from: '%s'\n",xmlReader->getNodeName());
              irr::io::IReadFile * rfile=archive->
                  createAndOpenFile (xmlReader->getNodeName());
              if(rfile) {
                CameraDataManager * cammgr=new CameraDataManager(rfile);
                irr::scene::ICameraSceneNode * camera=smgr->addCameraSceneNodeFPS();
                irr::core::vector3df p,r;
                cammgr->getPositionAndRotation(p,r);
                camera->setPosition(p);
                camera->setRotation(r);
                rfile->drop();
              } else {
                GM_LOG("Cannot load camera data, using default camera\n");
                smgr->addCameraSceneNodeFPS();
              }

          }
        }
      break;
      case irr::io::EXN_COMMENT:
        GM_LOG("%d Comment '%s'\n",i,
            xmlReader->getNodeName());
      break;
      case irr::io::EXN_CDATA:
        GM_LOG("%d Cdata '%s'\n",i,
            xmlReader->getNodeName());
      break;
      case irr::io::EXN_UNKNOWN:
        GM_LOG("%d Unknown '%s'\n",i,
            xmlReader->getNodeName());
      break;
      default:
      GM_LOG("%d '%s', type: %d\n",i,
          xmlReader->getNodeName(),
          xmlReader->getNodeType());
      break;
    }
  }
  archive->drop();
  m_filesystem->removeFileArchive(m_archiveIndex);
  m_loaded=true;
}

void Track::unload()
{
  if(!m_loaded) return;
	for (irr::u32 i=0; i < m_lights.size(); ++i ) {
    GM_LOG("  dropping light %X\n",m_sceneNodes[i]);
    m_lights[i]->remove();
    m_lights[i]->drop();
  }
  m_lights.erase(0,m_lights.size());

	for (irr::u32 i=0; i < m_sceneNodes.size(); ++i ) {
    GM_LOG("  dropping scene node %X\n",m_sceneNodes[i]);
    m_sceneNodes[i]->remove();
    m_sceneNodes[i]->drop();
  }
  m_sceneNodes.erase(0,m_sceneNodes.size());

  m_loaded=false;
}

Track::~Track()
{
  GM_LOG("Track destruct\n");
  unload();
  GM_LOG("Track destruct done\n");
}


void Track::loadLights( irr::io::IReadFile * file ,
  irr::scene::ISceneManager* smgr )
{
  irr::u16 mark;
  int cnt=0;
  double pos[3],dif[3],spe[3],radius;

  smgr->setAmbientLight(
      irr::video::SColorf(.6,.6,.6));

  mark=Util::readMark(file);

  Util::readTriple(file,pos);
  Util::readTriple(file,dif);
  Util::readTriple(file,spe);
  radius=Util::readDouble(file);

  cnt++;

  GM_LOG(" - %02d light\n   - position %f,%f,%f\n   - diffuse %f,%f,%f\n   - specular: %f,%f,%f\n   - radius: %f\n",
      cnt,
      pos[0], pos[1], pos[2],
      dif[0], dif[1], dif[2],
      spe[0], spe[1], spe[2],radius);

  irr::core::vector3df position = irr::core::vector3df(pos[0], pos[1], pos[2]);
  irr::video::SColorf specularColor = 
    irr::video::SColorf(spe[0],spe[1],spe[2]);
  irr::video::SColorf diffuseColor = 
    irr::video::SColorf(dif[0],dif[1],dif[2]);


  irr::scene::ILightSceneNode* light = 
    smgr->addLightSceneNode( 0, position, diffuseColor, radius);

  //light->setLightType(irr::video::ELT_DIRECTIONAL);
  //light->setRotation( irr::core::vector3df(180, 45, 45) );
  light->getLightData().SpecularColor = specularColor;
  light->grab();

}
