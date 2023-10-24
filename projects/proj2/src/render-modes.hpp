/*! \file render-modes.hpp
 *
 * CS23700 Autumn 2023 Sample Code for Project 2
 *
 * \author John Reppy
 */

/*
 * COPYRIGHT (c) 2023 John Reppy (http://cs.uchicago.edu/~jhr)
 * All rights reserved.
 */

#ifndef _RENDER_MODES_HPP_
#define _RENDER_MODES_HPP_

enum class RenderMode : int {
    eWireframe = 0,
    eFlatShading,
    eTextureShading,
    eNormalMapShading,
    eNumModes
};

/* cast to integer type for array indexing */
constexpr int kWireframe = static_cast<int>(RenderMode::eWireframe);
constexpr int kFlatShading = static_cast<int>(RenderMode::eFlatShading);
constexpr int kTextureShading = static_cast<int>(RenderMode::eTextureShading);
constexpr int kNormalMapShading = static_cast<int>(RenderMode::eNormalMapShading);
constexpr int kNumRenderModes = static_cast<int>(RenderMode::eNumModes);

#endif // !_RENDER_MODES_HPP_
