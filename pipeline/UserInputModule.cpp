#include <osg/io_utils>
#include <osg/BlendFunc>
#include <osg/ComputeBoundsVisitor>
#include <osgDB/ReadFile>
#include <iostream>
#include "UserInputModule.h"
#include "Utilities.h"

namespace osgVerse
{
    UserInputModule::UserInputModule(const std::string& name, Pipeline* pipeline)
    :   _pipeline(pipeline)
    {
        setName(name);
        if (pipeline) pipeline->addModule(name, this);
    }

    UserInputModule::~UserInputModule()
    {}

    Pipeline::Stage* UserInputModule::createStages(osg::Shader* vs, osg::Shader* fs,
                                                   Pipeline::Stage* bypass, unsigned int cullMask,
                                                   const std::string& cName, osg::Texture* colorBuffer,
                                                   const std::string& dName, osg::Texture* depthBuffer)
    {
        if (colorBuffer != NULL || depthBuffer != NULL)
        {
            Pipeline::BufferDescriptions buffers;
            {
                Pipeline::BufferDescription desc0(cName, osgVerse::Pipeline::RGB_INT8);
#ifdef VERSE_WASM
                Pipeline::BufferDescription desc1(dName, osgVerse::Pipeline::DEPTH32);
#else
                Pipeline::BufferDescription desc1(dName, osgVerse::Pipeline::DEPTH24_STENCIL8);
#endif
                if (colorBuffer != NULL) desc0.bufferToShare = colorBuffer;
                if (depthBuffer != NULL) desc1.bufferToShare = depthBuffer;
                buffers.push_back(desc0); buffers.push_back(desc1);
            }

            // Draw on existing buffers, no clear masks... This requires single-threaded only!!
            int flags = Pipeline::NO_DEFAULT_TEXTURES;
            Pipeline::Stage* stage = _pipeline->addInputStage(getName(), cullMask, flags, vs, fs, buffers);

            CustomData* customData = new CustomData(true);
            if (bypass != NULL) customData->bypassCamera = bypass->camera.get();
            stage->camera->setUserData(customData); stage->camera->setClearMask(0);
            stage->parentModule = this; return stage;
        }
        else
        {
            Pipeline::Stage* stage = _pipeline->addInputStage(
                getName(), cullMask, 0, vs, fs, 2, cName.c_str(), osgVerse::Pipeline::RGB_INT8,
#ifdef VERSE_WASM
                dName.c_str(), osgVerse::Pipeline::DEPTH32);
#else
                dName.c_str(), osgVerse::Pipeline::DEPTH24_STENCIL8);
#endif
            // TODO: combine this with pipeline color/depth?
            stage->parentModule = this; return stage;
        }
    }

    void UserInputModule::operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
        traverse(node, nv);
    }
}
