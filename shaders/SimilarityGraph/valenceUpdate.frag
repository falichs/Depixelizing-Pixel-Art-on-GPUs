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

/** 
 * @file valenceUpdate.frag
 * This shader updates the valence fields in the similarity graph.
 * The resulting similarity graph datastructure will follow this schematic:
 * <BR>
 * E E E E E
 * <BR>
 * E V E V E
 * <BR>
 * E E E E E
 * <BR>
 * E V E V E
 * <BR>
 * E E E E E
 * <BR>
 * with Edge indicators E and pixel-nodes V with their valence values encoded.
 * The encoded valence results from an OR combination of the 8 bit values defined in the macros below.
 * e.g. if a pixel-node has a connection to its northern and southern neighbor it's valence flag will be set to 128 || 8 = 136.
 * This way we can later on test connectivity using a boolean AND operator. e.g. 136 & 8 = 8 tells us that the pixel is connected to it's northern neighbor.
 */


/**
* @def NORTH
* this macro represents a connection from the current pixel-node to it's northern neighbor.
*/
#define NORTH		128

/**
* @def NORTHEAST
* this macro represents a connection from the current pixel-node to it's northeastern neighbor.
*/
#define NORTHEAST	64

/**
* @def EAST
* this macro represents a connection from the current pixel-node to it's eastern neighbor.
*/
#define EAST		32

/**
* @def SOUTHEAST
* this macro represents a connection from the current pixel-node to it's southeastern neighbor.
*/
#define SOUTHEAST	16

/**
* @def SOUTH
* this macro represents a connection from the current pixel-node to it's southern neighbor.
*/
#define SOUTH		8

/**
* @def SOUTHWEST
* this macro represents a connection from the current pixel-node to it's southwestern neighbor.
*/
#define SOUTHWEST	4

/**
* @def WEST
* this macro represents a connection from the current pixel-node to it's western neighbor.
*/
#define WEST		2

/**
* @def NORTHWEST
* this macro represents a connection from the current pixel-node to it's northwestern neighbor.
*/
#define NORTHWEST	1

/**
* @var in vec2 UV
* UV coordinate of the current fragment.
*/
in vec2 UV;

/**
* @var layout(pixel_center_integer) in vec4 gl_FragCoord
* absolute viewport coordinate of the current fragment.
*/
layout(pixel_center_integer) in vec4 gl_FragCoord;

/**
* @var uniform sampler2D similarityGraph
* texture containing the similarity graph to be updated.
*/
uniform sampler2D similarityGraph;

/**
* @var layout(location = 0) out vec4 color0
* output pixel-node's valence.
*/
layout(location = 0) out vec4 color0;

/**
* @fn void main()
* computes the pixel-nodes' valence
*/
void main() {

	//evalPos evaluation window position
	// UL UR
	// LL LR
	ivec2 samplerDimensions = textureSize(similarityGraph, 0);
	if( gl_FragCoord.x==0 || gl_FragCoord.x==samplerDimensions.x || gl_FragCoord.y==0 || gl_FragCoord.y==samplerDimensions.y ) {
		//border hit
		color0 = vec4(0.0 , 0.0 , 0.0 , 0.0);
	} else {
		vec2 evalPos = mod(gl_FragCoord.xy,2);
		//check if node-pixel got hit
		if ( evalPos == vec2(1,1) ) {
			
			//calculate node valence
			int valence = 0;
			
			int edges=0;
			
			ivec4 traceNodes = ivec4(0,0,0,0);
			//browse neighborhood
			//NW
			int edgeValue = int(texelFetch(similarityGraph, ivec2(gl_FragCoord.x-1, gl_FragCoord.y+1), 0).x*255);
			if( (edgeValue & 32) == 32) {
				valence++;
				edges = edges | NORTHWEST ;
			}
			//N
			if(texelFetch(similarityGraph, ivec2(gl_FragCoord.x, gl_FragCoord.y+1), 0).x > 0.0) {
				valence++;
				edges = edges | NORTH ;
			}
			//NE
			edgeValue = int(texelFetch(similarityGraph, ivec2(gl_FragCoord.x+1, gl_FragCoord.y+1), 0).x*255);
			if( (edgeValue & 64) == 64) {
				valence++;
				edges = edges | NORTHEAST ;
			}
			//E
			if(texelFetch(similarityGraph, ivec2(gl_FragCoord.x+1, gl_FragCoord.y), 0).x > 0.0) {
				valence++;
				edges = edges | EAST ;
			}
			//SE
			edgeValue = int(texelFetch(similarityGraph, ivec2(gl_FragCoord.x+1, gl_FragCoord.y-1), 0).x*255);
			if( (edgeValue & 32) == 32) {
				valence++;
				edges = edges | SOUTHEAST ;
			}
			//S
			if(texelFetch(similarityGraph, ivec2(gl_FragCoord.x, gl_FragCoord.y-1), 0).x > 0.0) {
				valence++;
				edges = edges | SOUTH ;
			}
			//SW
			edgeValue = int(texelFetch(similarityGraph, ivec2(gl_FragCoord.x-1, gl_FragCoord.y-1), 0).x*255);
			if( (edgeValue & 64) == 64) {
				valence++;
				edges = edges | SOUTHWEST ;
			}
			//W
			if(texelFetch(similarityGraph, ivec2(gl_FragCoord.x-1, gl_FragCoord.y), 0).x > 0.0) {
				valence++;
				edges = edges | WEST ;
			}
			
			color0 = vec4(valence/255.0, edges/255.0, 0.0, 0.0);
			
		} else {
			//copy value
			color0 = texelFetch(similarityGraph, ivec2(gl_FragCoord.xy), 0);
		}
	}

}