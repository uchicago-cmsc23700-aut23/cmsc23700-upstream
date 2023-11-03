/*! \file scene.hpp
 *
 * CS23700 Autumn 2023 Sample Code for Project 3
 *
 * \author John Reppy
 *
 * This file defines the Scene class, which represents the contents of a scene
 * loaded from disk.
 */

/*
 * COPYRIGHT (c) 2023 John Reppy (http://cs.uchicago.edu/~jhr)
 * All rights reserved.
 */

#ifndef _SCENE_HPP_
#define _SCENE_HPP_

#include <vector>
#include <string>
#include <map>
#include "cs237.hpp"
#include "obj.hpp"
#include "height-field.hpp"

//! an instance of a model, which has its own position and color.
struct SceneObj {
    int model;          //!< the ID of the model that defines the object's mesh
    glm::mat4 toWorld;  //!< affine transform from object space to world space
    glm::vec3 color;    //!< the color of the object

    /// return the matrix for converting normal vectors from the object's
    /// coordinate system to the world coordinate system
    glm::mat3 normToWorld () const
    {
        return glm::inverseTranspose(glm::mat3(this->toWorld));
    }

};

//! a point light in the scene
struct SpotLight {
    glm::vec3 pos;              //!< world-space coordinates of light
    glm::vec3 dir;              //!< unit-length direction vector
    float cutoff;               //!< the cutoff angle for the light (in degrees)
    float exponent;             //!< the focus exponent
    glm::vec3 intensity;        //!< the light's intensity
    float k0, k1, k2;           //!< attenuation coefficients
};

//! a scene consisting of an initial camera configuration and some objects
class Scene {
  public:

  // constructor/destructor
    Scene ();
    ~Scene ();

  //! load a scene from the specified path
  //! \param path  the path to the scene directory
  //! \return true if there were any errors loading the scene and false otherwise
    bool load (std::string const &path);

  //! the width of the viewport as specified by the scene
    int width () const { return this->_wid; }

  //! the height of the viewport as specified by the scene
    int height () const { return this->_ht; }

  //! the horizontal field of vew
    float horizontalFOV () const { return this->_fov; }

  //! the initial camera position
    glm::vec3 cameraPos () const { return this->_camPos; }

  //! the initial camera look-at point
    glm::vec3 cameraLookAt () const { return this->_camAt; }

  //! the initial camera up vector
    glm::vec3 cameraUp () const { return this->_camUp; }

  //! the number of lights in the scene
    int numLights () const { return this->_lights.size(); }

  //! iterator for looping over the lights in the scene
    std::vector<SpotLight>::const_iterator beginLights ()  { return this->_lights.cbegin(); }

  //! terminator for looping over the lights in the scene
    std::vector<SpotLight>::const_iterator endLights () const { return this->_lights.cend(); }

  //! return the i'th object in the scene
    SpotLight const light (int idx) const { return this->_lights[idx]; }

  //! the ambient light intensity in the scene
    glm::vec3 ambientLight () const { return this->_ambI; }

    /// the shadow factor
    float shadowFactor () const { return this->_shadowFactor; }

  //! return the height-field that represents the ground object,
  //! or nullptr if there is no ground in the scene.
    const HeightField *ground () const { return this->_hf; }

  //! return the number of objects in the scene
    int numObjects () const { return this->_objs.size(); }

  //! iterator for looping over the objects in the scene
    std::vector<SceneObj>::const_iterator beginObjs () const { return this->_objs.begin(); }

  //! terminator for looping over the objects in the scene
    std::vector<SceneObj>::const_iterator endObjs () const { return this->_objs.end(); }

  //! return the i'th object in the scene
    SceneObj const object (int idx) const { return this->_objs[idx]; }

  //! return the number of distinct models in the scene
    int numModels () const { return this->_models.size(); }

  //! iterator for looping over the models in the scene
    std::vector<OBJ::Model const *>::const_iterator beginModels () const
    {
        return this->_models.begin();
    }

  //! terminator for looping over the models in the scene
    std::vector<OBJ::Model const *>::const_iterator endModels () const
    {
        return this->_models.end();
    }

  //! return the i'th model in the scene
    const OBJ::Model *model (int idx) const { return this->_models[idx]; }

  //! lookup a texture image by name
  //! \returns a pointer to the image object or nullptr if the image is not found
    cs237::Image2D *textureByName (std::string name) const;

  private:
    bool _loaded;               //!< has the scene been loaded?

    int _wid;                   //!< initial window width
    int _ht;                    //!< initial window height
    float _fov;                 //!< horizontal field of view in degrees
    glm::vec3 _camPos;          //!< camera position
    glm::vec3 _camAt;           //!< the point at which the camera is pointing at
    glm::vec3 _camUp;           //!< the camera's up vector
    glm::vec3 _ambI;            //!< the ambient light's intensity
    float _shadowFactor;        //!< scaling factor for in-shadow fragments

    HeightField *_hf;           //!< the height field that represents the ground; nullptr if
                                //!  the scene does not have a ground Object

    std::vector<OBJ::Model const *> _models;            //!< the OBJ models in the scene
    std::vector<SceneObj> _objs;                        //!< the objects in the scene
    std::vector<SpotLight> _lights;                     //!< the lights in the scene
    std::map<std::string, cs237::Image2D *> _texs;      //!< the textures keyed by name

    //! helper function for loading textures into the _texs map
    //! \param path  the path to the directory containing the image file
    //! \param name  the name of the file
    //! \param nMap  optional argument specifying if the texture is a normal map (default false).
    void _loadTexture (std::string path, std::string name, bool nMap = false);

};

#endif /* !_SCENE_HPP_ */
