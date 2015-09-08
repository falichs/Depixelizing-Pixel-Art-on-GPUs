// Copyright (c) 2015 Felix Kreuzer
//
// This source is subject to the MIT License.
// Please see the LICENSE.txt file for more information.
// All other rights reserved.
//
// THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.

#ifndef SHADER_H
#define SHADER_H

/** 
 * @file Shader.h 
 * Contains helper functions for loading various shaders from file.
 */

#define PA_SHADER_VS_QUAD 					"../shaders/quad_shader.vert" 
#define PA_SHADER_FS_PIXELS					"../shaders/drawPixels.frag"

#define PA_SHADER_FS_DISSIMILAR 			"../shaders/SimilarityGraph/dissimilar.frag"
#define PA_SHADER_FS_UPDATE_VALENCE 		"../shaders/SimilarityGraph/valenceUpdate.frag"
#define PA_SHADER_FS_ELIMINATE_CROSSINGS	"../shaders/SimilarityGraph/eliminateCrossings.frag"
#define PA_SHADER_VS_SIMGRAPH_DRAW			"../shaders/SimilarityGraph/drawGraph.vert"
#define PA_SHADER_GS_SIMGRAPH_DRAW			"../shaders/SimilarityGraph/drawGraph.geom"
#define PA_SHADER_FS_SIMGRAPH_DRAW			"../shaders/SimilarityGraph/drawGraph.frag"
#define PA_SHADER_VS_SIMGRAPH_DRAW_NODES	"../shaders/SimilarityGraph/drawGraphNodes.vert"

#define PA_SHADER_VS_CELLGRAPH				"../shaders/FullCellGraphConstruction/FullCellGraphConstruction.vert"
#define PA_SHADER_GS_CELLGRAPH				"../shaders/FullCellGraphConstruction/FullCellGraphConstruction.geom"
#define PA_SHADER_VS_CELLGRAPH_DRAW			"../shaders/FullCellGraphDrawing/FullCellGraphDrawing.vert" 
#define PA_SHADER_GS_CELLGRAPH_DRAW			"../shaders/FullCellGraphDrawing/FullCellGraphDrawing.geom"
#define PA_SHADER_FS_CELLGRAPH_DRAW			"../shaders/FullCellGraphDrawing/FullCellGraphDrawing.frag"

#define PA_SHADER_VS_OPTIMIZE_ENERGY		"../shaders/Optimization/OptimizeEnergy.vert"
#define PA_SHADER_VS_UPDATE_POSITIONS		"../shaders/UpdateCorrectedPositions/UpdateCorrectedPositions.vert"

#define PA_SHADER_VS_RASTERIZER				"../shaders/GaussRasterizer/GaussRasterizer.vert"
#define PA_SHADER_FS_RASTERIZER				"../shaders/GaussRasterizer/GaussRasterizer.frag"

#define PA_SHADER_VS_VORONOI_DRAW			"../shaders/VoronoiGraph/dbgDrawVoronoiCellGraph3x3.vert"
#define PA_SHADER_GS_VORONOI_DRAW			"../shaders/VoronoiGraph/dbgDrawVoronoiCellGraph3x3.geom"
#define PA_SHADER_FS_VORONOI_DRAW			"../shaders/VoronoiGraph/dbgDrawVoronoiCellGraph3x3.frag"
#define PA_SHADER_VS_VORONOI_OVERLAY		"../shaders/VoronoiGraph/dbgDrawVoronoiCellOverlay.vert"
#define PA_SHADER_GS_VORONOI_OVERLAY		"../shaders/VoronoiGraph/dbgDrawVoronoiCellOverlay.geom"
#define PA_SHADER_FS_VORONOI_OVERLAY		"../shaders/VoronoiGraph/dbgDrawVoronoiCellOverlay.frag"

 /**
 * Load a GLSL shader porgram from file.
 * @param vertex_file_path Path to Vertex Shader file
 * @param geometry_file_path Path to Geometry Shader file
 * @param fragment_file_path Path to Fragment Shader file
 */
GLuint LoadShaders(const char * vertex_file_path,const char * geometry_file_path,const char * fragment_file_path);

 /**
 * Load a GLSL shader porgram from file.
 * @param vertex_file_path Path to Vertex Shader file
 * @param fragment_file_path Path to Fragment Shader file
 */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path);

 /**
 * Specialized loader used to load the cell graph shader porgram from file.
 * @param vertex_file_path Path to Vertex Shader file
 * @param geometry_file_path Path to Geometry Shader file
 */
GLuint LoadCellGraphShader(const char * vertex_file_path,const char * geometry_file_path);

 /**
 * DEPRECATED
 */
GLuint LoadSegmentPreparationShader(const char * vertex_file_path,const char * geometry_file_path);

 /**
 * Specialized loader used to load the spline control-point optimization shader porgram from file.
 * @param vertex_file_path Path to Vertex Shader file
 */
GLuint LoadOptimizationShader(const char * vertex_file_path);

#endif
