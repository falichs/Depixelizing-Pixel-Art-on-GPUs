#version 330 core

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

#define EDGE_HORVERT 16
#define EDGE_DIAGONAL_ULLR 32
#define EDGE_DIAGONAL_LLUR 64
#define EDGE_N 1
#define EDGE_E 2
#define EDGE_S 4
#define EDGE_W 8

//UL offsets
#define OFF_UL_UR_X -0.25
#define OFF_UL_UR_Y  0.75
#define OFF_UL_C_X  -0.5
#define OFF_UL_C_Y   0.5
#define OFF_UL_LR_X -0.25
#define OFF_UL_LR_Y  0.25
#define OFF_UL_LL_X -0.75
#define OFF_UL_LL_Y  0.25
//UR offsets
#define OFF_UR_UL_X  0.25
#define OFF_UR_UL_Y  0.75
#define OFF_UR_C_X   0.5
#define OFF_UR_C_Y   0.5
#define OFF_UR_LR_X  0.75
#define OFF_UR_LR_Y  0.25
#define OFF_UR_LL_X  0.25
#define OFF_UR_LL_Y  0.25
//LR offsets
#define OFF_LR_UL_X  0.25
#define OFF_LR_UL_Y -0.25
#define OFF_LR_UR_X  0.75
#define OFF_LR_UR_Y -0.25
#define OFF_LR_C_X   0.5
#define OFF_LR_C_Y  -0.5
#define OFF_LR_LL_X  0.25 
#define OFF_LR_LL_Y -0.75
//LL offsets
#define OFF_LL_UL_X -0.75
#define OFF_LL_UL_Y -0.25
#define OFF_LL_UR_X -0.25
#define OFF_LL_UR_Y -0.25
#define OFF_LL_C_X  -0.5
#define OFF_LL_C_Y  -0.5
#define OFF_LL_LR_X -0.25
#define OFF_LL_LR_Y -0.75

layout (points) in;
//in vec4[] color;
layout (triangle_strip, max_vertices = 18) out;
out vec4 perVertexColor;

uniform sampler2D similarityGraph;
uniform sampler2D pixelArt;

uniform float uf_zoomFactor;
uniform vec2 uv2_zoomWindowCenter;

vec4 mapVertex(vec2 vIn) {
	return vec4((vIn*2.0/(textureSize(pixelArt,0)-1.0)-1.0)  * uf_zoomFactor + uv2_zoomWindowCenter,0.0,1.0);
}


void main() {
	ivec2 simGraphPixelCoords = ivec2(gl_in[0].gl_Position.xy*2.0f+1.0);
	int eCenter = int(texelFetch(similarityGraph,simGraphPixelCoords,0).r*255);
	perVertexColor = texelFetch(pixelArt,ivec2(gl_in[0].gl_Position.xy),0);
	vec4 firstVertex = vec4(0.0, 0.0, 0.0, 0.0);
	//fetch the diagonals around the block
	
	int eUL = int(texelFetch(similarityGraph,ivec2(simGraphPixelCoords.x-1,simGraphPixelCoords.y+1),0).r*255);
	int eUR = int(texelFetch(similarityGraph,ivec2(simGraphPixelCoords.x+1,simGraphPixelCoords.y+1),0).r*255);
	int eLR = int(texelFetch(similarityGraph,ivec2(simGraphPixelCoords.x+1,simGraphPixelCoords.y-1),0).r*255);
	int eLL = int(texelFetch(similarityGraph,ivec2(simGraphPixelCoords.x-1,simGraphPixelCoords.y-1),0).r*255);

	//UL sector
	if(eUL == EDGE_DIAGONAL_ULLR) {// ULLR
		firstVertex = mapVertex(gl_in[0].gl_Position.xy+vec2(OFF_UL_LL_X,OFF_UL_LL_Y));
		gl_Position = firstVertex;
		EmitVertex();
		gl_Position = mapVertex(gl_in[0].gl_Position.xy);
		EmitVertex();
		gl_Position = mapVertex(gl_in[0].gl_Position.xy+vec2(OFF_UL_UR_X,OFF_UL_UR_Y));
		EmitVertex();
		gl_Position = mapVertex(gl_in[0].gl_Position.xy);
		EmitVertex();
	} else if (eUL == EDGE_DIAGONAL_LLUR) {//LLUR
		firstVertex = mapVertex(gl_in[0].gl_Position.xy+vec2(OFF_UL_LR_X,OFF_UL_LR_Y));
		gl_Position = firstVertex;
		EmitVertex();
		gl_Position = mapVertex(gl_in[0].gl_Position.xy);
		EmitVertex();
	} else {// no diag
		firstVertex = mapVertex(gl_in[0].gl_Position.xy+vec2(OFF_UL_C_X,OFF_UL_C_Y));
		gl_Position = firstVertex;
		EmitVertex();
		gl_Position = mapVertex(gl_in[0].gl_Position.xy);
		EmitVertex();
	}
	//UR sector
	if(eUR == EDGE_DIAGONAL_ULLR) {// ULLR
		gl_Position = mapVertex(gl_in[0].gl_Position.xy+vec2(OFF_UR_LL_X,OFF_UR_LL_Y));
		EmitVertex();
		gl_Position = mapVertex(gl_in[0].gl_Position.xy);
		EmitVertex();
	} else if (eUR == EDGE_DIAGONAL_LLUR) {//LLUR
		gl_Position = mapVertex(gl_in[0].gl_Position.xy+vec2(OFF_UR_UL_X,OFF_UR_UL_Y));
		EmitVertex();
		gl_Position = mapVertex(gl_in[0].gl_Position.xy);
		EmitVertex();
		gl_Position = mapVertex(gl_in[0].gl_Position.xy+vec2(OFF_UR_LR_X,OFF_UR_LR_Y));
		EmitVertex();
		gl_Position = mapVertex(gl_in[0].gl_Position.xy);
		EmitVertex();
	} else {// no diag
		gl_Position = mapVertex(gl_in[0].gl_Position.xy+vec2(OFF_UR_C_X,OFF_UR_C_Y));
		EmitVertex();
		gl_Position = mapVertex(gl_in[0].gl_Position.xy);
		EmitVertex();
	}
	//LR sector
	if(eLR == EDGE_DIAGONAL_ULLR) {// ULLR
		gl_Position = mapVertex(gl_in[0].gl_Position.xy+vec2(OFF_LR_UR_X,OFF_LR_UR_Y));
		EmitVertex();
		gl_Position = mapVertex(gl_in[0].gl_Position.xy);
		EmitVertex();
		gl_Position = mapVertex(gl_in[0].gl_Position.xy+vec2(OFF_LR_LL_X,OFF_LR_LL_Y));
		EmitVertex();
		gl_Position = mapVertex(gl_in[0].gl_Position.xy);
		EmitVertex();
	} else if (eLR == EDGE_DIAGONAL_LLUR) {//LLUR
		gl_Position = mapVertex(gl_in[0].gl_Position.xy+vec2(OFF_LR_UL_X,OFF_LR_UL_Y));
		EmitVertex();
		gl_Position = mapVertex(gl_in[0].gl_Position.xy);
		EmitVertex();
	} else {// no diag
		gl_Position = mapVertex(gl_in[0].gl_Position.xy+vec2(OFF_LR_C_X,OFF_LR_C_Y));
		EmitVertex();
		gl_Position = mapVertex(gl_in[0].gl_Position.xy);
		EmitVertex();
	}
	//LL sector
	if(eLL == EDGE_DIAGONAL_ULLR) {// ULLR
		gl_Position = mapVertex(gl_in[0].gl_Position.xy+vec2(OFF_LL_UR_X,OFF_LL_UR_Y));
		EmitVertex();
		gl_Position = mapVertex(gl_in[0].gl_Position.xy);
		EmitVertex();
	} else if (eLL == EDGE_DIAGONAL_LLUR) {//LLUR
		gl_Position = mapVertex(gl_in[0].gl_Position.xy+vec2(OFF_LL_LR_X,OFF_LL_LR_Y));
		EmitVertex();
		gl_Position = mapVertex(gl_in[0].gl_Position.xy);
		EmitVertex();
		gl_Position = mapVertex(gl_in[0].gl_Position.xy+vec2(OFF_LL_UL_X,OFF_LL_UL_Y));
		EmitVertex();
		gl_Position = mapVertex(gl_in[0].gl_Position.xy);
		EmitVertex();
	} else {// no diag
		gl_Position = mapVertex(gl_in[0].gl_Position.xy+vec2(OFF_LL_C_X,OFF_LL_C_Y));
		EmitVertex();
		gl_Position = mapVertex(gl_in[0].gl_Position.xy);
		EmitVertex();
	}

	gl_Position = firstVertex;
	EmitVertex();
	EndPrimitive();
}
