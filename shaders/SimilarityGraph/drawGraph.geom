#version 330

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

#define NORTH		128
#define NORTHEAST	64
#define EAST		32
#define SOUTHEAST	16
#define SOUTH		8
#define SOUTHWEST	4
#define WEST		2
#define NORTHWEST	1

layout (points) in;
layout (line_strip) out;
layout (max_vertices = 20) out;

uniform sampler2D similarityGraph;
uniform float uf_zoomFactor;
uniform vec2 uv2_zoomWindowCenter;
//float uf_zoomFactor = 1.0;
//vec2 uv2_zoomWindowCenter = vec2(0.5,0.5);

vec4 calcNormalizedPosition(int xShift, int yShift) {
	vec2 pixelArtDimensions = (textureSize(similarityGraph, 0) - 1) / 2.0f;
	float dx = 1.0f / pixelArtDimensions.x;
	float dy = 1.0f / pixelArtDimensions.y;
	//float nXPos = (gl_in[0].gl_Position.x + xShift) * 2*dx - (1.0f - dx);
	//float nYPos = (gl_in[0].gl_Position.y + yShift) * 2*dy - (1.0f - dy);
	float nXPos = (gl_in[0].gl_Position.x + xShift) * 2*dx - (1.0f - dx);
	float nYPos = (gl_in[0].gl_Position.y + yShift) * 2*dy - (1.0f - dy);
	return vec4(nXPos * uf_zoomFactor + uv2_zoomWindowCenter.x,nYPos * uf_zoomFactor + uv2_zoomWindowCenter.y,gl_in[0].gl_Position.z,gl_in[0].gl_Position.w);
	
}

void main(void) {
	
	//get edge info for current point
	int xPos = int(gl_in[0].gl_Position.x);
	int yPos = int(gl_in[0].gl_Position.y);
	ivec2 similarityGraphCoordinates = 2*ivec2(xPos,yPos)+1;
	int nhood = int(texelFetch(similarityGraph, similarityGraphCoordinates, 0).y*255);
	if( (nhood & NORTHEAST) == NORTHEAST ) {
		gl_Position = calcNormalizedPosition(0,0);
		EmitVertex();
		gl_Position = calcNormalizedPosition(1,1);
		EmitVertex();
		EndPrimitive();
	}
	if( (nhood & EAST) == EAST ) {
		gl_Position = calcNormalizedPosition(0,0);
		EmitVertex();
		gl_Position = calcNormalizedPosition(1,0);
		EmitVertex();
		EndPrimitive();
	}
	if( (nhood & SOUTHEAST) == SOUTHEAST ) {
		gl_Position = calcNormalizedPosition(0,0);
		EmitVertex();
		gl_Position = calcNormalizedPosition(1,-1);
		EmitVertex();
		EndPrimitive();
	}
	if( (nhood & SOUTH) == SOUTH ) {
		gl_Position = calcNormalizedPosition(0,0);
		EmitVertex();
		gl_Position = calcNormalizedPosition(0,-1);
		EmitVertex();
		EndPrimitive();
	}
}