/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#ifndef CLUSTER_DIM_X
#define CLUSTER_DIM_X 24 
#endif

#ifndef CLUSTER_DIM_Y
#define CLUSTER_DIM_Y 12 
#endif

#ifndef CLUSTER_DIM_Z
#define CLUSTER_DIM_Z 128 
#endif

#ifndef NEAR_CLUST
#define NEAR_CLUST 50.0f
#endif

#ifndef LIGHT_PER_CLUSTER
#define LIGHT_PER_CLUSTER 30
#endif
 
#define LINKED_LIST_INDEX_PER_BYTE 4	 
#define CLUSTER_MAX_DEPTH 255.0f
#define CLUSTER_LIGHT_TYPE_PER_BIT 8
#define CLUSTER_LIGHT_ID_PER_BIT 24
#define CLUSTER_LIGHT_NODE_PER_BIT 32
#define CLUSTER_BIT (CLUSTER_LIGHT_TYPE_PER_BIT + CLUSTER_LIGHT_ID_PER_BIT + CLUSTER_LIGHT_NODE_PER_BIT)

#define CLUSTER_LIGHT_ID_RANGE 0xFFFFFF					 
#define CLUSTER_LIGHT_INVALID_ID 0x3FFFFFFF				 

#define POINT_LIGHT_TYPE_VALUE 0
#define SPOT_LIGHT_TYPE_VALUE 1
#define RECT_LIGHT_TYPE_VALUE 2

struct LinkedLightID
{
	uint lightID;
	uint link;
};
