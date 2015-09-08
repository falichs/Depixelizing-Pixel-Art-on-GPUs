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

// Input vertex data
layout(location = 0) in vec2 pos;
layout(location = 1) in ivec4 neighbors;
layout(location = 2) in int flags;
//layout(location = 3) in int valence;

out VertexData
{
  vec2 pos;
  ivec4 neighbors;
  int flags;
  //int valence;
} outData;

void main(){

	gl_Position.xy = pos;
	outData.pos = pos;
	outData.neighbors = neighbors;
	outData.flags = flags;
	//outData.valence = valence;
}

